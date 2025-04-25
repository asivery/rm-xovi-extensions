#include <QObject>
#include <QProcess>
#include <QString>
#include <QQmlApplicationEngine>
#include <dlfcn.h>
#include "XoviMessageBroker.h"


extern "C" void _xovi_construct(){
    qmlRegisterType<XoviMessageBroker>("net.asivery.XoviMessageBroker", 1, 0, "XoviMessageBroker");
}

extern "C" char _xovi_shouldLoad() {
    // Only attach self to GUI applications
    void *resFunc = dlsym(RTLD_DEFAULT, "_Z21qRegisterResourceDataiPKhS0_S0_");
    if(resFunc == NULL) {
        printf("[xovi-message-broker]: Not a GUI application. Refusing to load.\n");
        return 0;
    }
    return 1;
}
