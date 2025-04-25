use std::collections::HashMap;
use std::convert::Infallible;
use std::net::{SocketAddr, SocketAddrV4};
use std::sync::{Mutex, MutexGuard};
use std::thread::sleep;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use http_body_util::combinators::BoxBody;
use http_body_util::{BodyExt, Full};
use hyper::body::{Bytes, Frame};
use hyper::header::HeaderValue;
use hyper::server::conn::http1;
use hyper::service::service_fn;
use hyper::{Request, Response};
use hyper_util::rt::TokioIo;
use lazy_static::lazy_static;
use tokio::net::{TcpListener, TcpStream};

use crate::feedback;

const LISTEN_IFACE: &'static str = "0.0.0.0:80";
const REAL_ADDR: (&'static str, u16) = ("127.0.0.1", 81);
const ENTRY_LIFE_DURATION: u64 = 300_000;

#[derive(Debug)]
pub enum IpRefAction {
    Blocked,
    Allowed,
}
struct IpAddressReference {
    action: IpRefAction,
    since: u64,
}

lazy_static! {
    static ref IP_LIST: Mutex<HashMap<String, IpAddressReference>> = Mutex::new(HashMap::new());
}

fn epoch() -> u64 {
    SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_millis() as u64
}

fn lock_wait_global_list<'a>() -> MutexGuard<'a, HashMap<String, IpAddressReference>> {
    loop {
        let guard = IP_LIST.try_lock();
        match guard {
            Err(_) => {
                sleep(Duration::from_millis(100));
            }
            Ok(g) => {
                return g;
            }
        }
    }
}

pub fn add_address(ip: String, action: IpRefAction) {
    eprintln!("Adding address {} to the list as {:?}", &ip, action);
    lock_wait_global_list().insert(ip, IpAddressReference {
        action,
        since: epoch(),
    });
}

fn full<T: Into<Bytes>>(chunk: T) -> BoxBody<Bytes, hyper::Error> {
    Full::new(chunk.into())
        .map_err(|never| match never {})
        .boxed()
}

async fn main_service(
    request: Request<hyper::body::Incoming>,
    peer_address: SocketAddr,
) -> Result<Response<BoxBody<Bytes, hyper::Error>>, Infallible> {
    macro_rules! send_and_log {
        ($log:expr) => {
            println!("[webserver-remote]: {}", $log);
            return Ok(Response::new(full($log)));
        };
    }
    macro_rules! ready_page {
        ($file: expr) => {
            return Ok(Response::builder().header("Content-Type", "text/html").body(full(include_str!($file))).unwrap());
        };
    }
    {
        let ip = peer_address.ip().to_string();
        let mut global_list = lock_wait_global_list();
        match global_list.get_mut(&ip) {
            None => {
                println!("[webserver-remote]: Unknown IP {}", ip);
                feedback::broadcast(ip);
                ready_page!("page_query.html");
            }
            Some(e) => {
                let age = epoch() - e.since;
                if age > ENTRY_LIFE_DURATION {
                    // Drop it.
                    global_list.remove(&ip);
                    println!("[webserver-remote]: Outdated IP {}", ip);
                    feedback::broadcast(ip);
                    ready_page!("page_query.html");
                } else {
                    match e.action {
                        IpRefAction::Blocked => {
                            // Blocked ones do not refresh the time.
                            println!("[webserver-remote]: Blocked IP {} ({} ms left)", ip, ENTRY_LIFE_DURATION - age);
                            ready_page!("page_blocked.html");
                        }
                        IpRefAction::Allowed => {
                            println!("[webserver-remote]: Allowed IP {} access to {} (outdated in {} ms)", ip, request.uri().path(), ENTRY_LIFE_DURATION - age);
                            e.since = epoch();
                        }
                    }
                }
            }
        }
    }

    let mut new_headers = request.headers().clone();
    let new_host_value = HeaderValue::from_str(REAL_ADDR.0).unwrap();
    new_headers.insert("Host", new_host_value);

    let uri = request.uri().clone();
    let stream = match TcpStream::connect(REAL_ADDR).await {
        Err(_) => {
            send_and_log!(format!("Cannot connect to uri {:?}", REAL_ADDR));
        }
        Ok(e) => e,
    };

    let io = TokioIo::new(stream);

    let (mut sender, conn) = match hyper::client::conn::http1::handshake(io).await {
        Err(e) => {
            send_and_log!(format!(
                "Error - handshake to {:?} ({})",
                REAL_ADDR,
                e.to_string()
            ));
        }
        Ok(e) => e,
    };

    tokio::task::spawn(async move {
        if let Err(err) = conn.await {
            println!("Connection failed: {:?}", err);
        }
    });
    let original_method = request.method().clone();

    let frame_stream = request.into_body().map_frame(|frame| {
        let frame = if let Ok(data) = frame.into_data() {
            data
        } else {
            Bytes::new()
        };

        Frame::data(frame)
    });

    let mut real_request_builder = Request::builder().uri(uri).method(original_method);
    for (key, val) in new_headers.iter() {
        real_request_builder
            .headers_mut()
            .unwrap()
            .insert(key, val.clone());
    }
    let real_request = real_request_builder.body(frame_stream.boxed()).unwrap();
    let real_result = match sender.send_request(real_request).await {
        Err(e) => {
            send_and_log!(format!("Unknown error: {}", e.to_string()));
        }
        Ok(e) => e,
    };

    let real_status = real_result.status();
    let mut real_headers = real_result.headers().clone();
    // xochitl sends some headers incorrectly.
    if real_headers.contains_key("content-length") {
        real_headers.remove("transfer-encoding");
    }

    let out_frame_stream = real_result.into_body().map_frame(|frame| {
        let frame = if let Ok(data) = frame.into_data() {
            data
        } else {
            Bytes::new()
        };

        Frame::data(frame)
    });

    let mut final_result = Response::new(out_frame_stream.boxed());
    *final_result.status_mut() = real_status;
    for (key, val) in real_headers.iter() {
        final_result.headers_mut().insert(key, val.clone());
    }

    Ok(final_result)
}

async fn proxy() {
    eprintln!("[webserver-remote]: Starting proxy asynchronously...");
    let addr: SocketAddrV4 = LISTEN_IFACE.parse().unwrap();

    let listener = TcpListener::bind(addr)
        .await
        .expect("Couldn't start the listener");

    // We start a loop to continuously accept incoming connections
    loop {
        eprintln!("[webserver-remote]: Waiting for connections");
        let (stream, _) = match listener.accept().await {
            Err(_) => continue,
            Ok(a) => a,
        };
        let stream_address = match stream.peer_addr() {
            Err(_) => {
                println!("[webserver-remote]: Couldn't get the peer address!");
                continue;
            }
            Ok(e) => e,
        };

        // Use an adapter to access something implementing `tokio::io` traits as if they implement
        // `hyper::rt` IO traits.
        let io = TokioIo::new(stream);

        tokio::task::spawn(async move {
            // Finally, we bind the incoming connection to our `hello` service
            if let Err(err) = http1::Builder::new()
                .ignore_invalid_headers(true)
                // `service_fn` converts our function in a `Service`
                .serve_connection(
                    io,
                    service_fn(|x| main_service(x, stream_address)),
                )
                .await
            {
                println!("Error serving connection: {:?}", err);
            }
        });
    }
}

pub fn start_proxy() {
    eprintln!("[webserver-remote]: Starting proxy");
    std::thread::spawn(|| {
        tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .unwrap()
        .block_on(async {
            tokio::spawn(proxy()).await.unwrap();
        });
    });
}
