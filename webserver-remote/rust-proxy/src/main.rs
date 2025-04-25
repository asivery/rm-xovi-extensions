mod proxy;

#[path = "feedback_test.rs"]
mod feedback;

fn main() {
    // Tests!
    proxy::add_address("127.0.0.1".into(), proxy::IpRefAction::Allowed);
    proxy::add_address("192.168.11.1".into(), proxy::IpRefAction::Blocked);
    proxy::start_proxy();
}
