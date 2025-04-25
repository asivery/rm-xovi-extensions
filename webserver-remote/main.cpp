#include <QNetworkInterface>
#include <QString>

#include "xovi.h"

extern "C" {
    // Redirect usb0, usb1 => lo
    QNetworkInterface override$_ZN17QNetworkInterface17interfaceFromNameERK7QString(QString const& name) {
        if(name == "usb0" || name == "usb1") {
            QString tmp("lo");
            return $_ZN17QNetworkInterface17interfaceFromNameERK7QString(&tmp);
        }
        return $_ZN17QNetworkInterface17interfaceFromNameERK7QString(&name);
    }

    // Required for a check within xochitl
    bool override$_ZNK12QHostAddress8isGlobalEv(QHostAddress *that) {
        return that->isLoopback() ? true : $_ZNK12QHostAddress8isGlobalEv(that);
    }

    // Start the HTTP server on port 81 instead of 80.
    void override$_ZN10QTcpServer6listenERK12QHostAddresst(void *that, QHostAddress const& ha, unsigned short port) {
        $_ZN10QTcpServer6listenERK12QHostAddresst(that, ha, port == 80 ? 81 : port);
    }


    // For rust code:
    void broadcast_to_frontend(const char *ip) {
        xovi_message_broker$broadcast("webserver-remote$newIP", ip);
    }

    void start_proxy_server();
    void _xovi_construct() {
        qt_resource_rebuilder$qmldiff_add_external_diff(r$notification, "Notification for remote file access");
        start_proxy_server();
    }
}
