#include "messagebroker.h"
#include "XoviMessageBroker.h"
#include <vector>
#include <cstring>

std::vector<XoviMessageBroker *> brokers;

void messagebroker::addBroadcastListener(XoviMessageBroker *ref) {
    // Cannot have more than one.
    if(std::find(brokers.begin(), brokers.end(), ref) == brokers.end()) {
        brokers.push_back(ref);
    }
}

void messagebroker::removeBroadcastListener(XoviMessageBroker *ref) {
    std::vector<XoviMessageBroker *>::iterator iter;
    if((iter = std::find(brokers.begin(), brokers.end(), ref)) != brokers.end()) {
        brokers.erase(iter);
    }
}


extern "C" char *broadcast(const char *signal, const char *value) {
    int i = 0;
    QString returnValue;
    QString qSignal(signal), qValue(value);
    for(auto &ref : brokers) {
        if(ref->getListeningFor().contains(qSignal)) {
            returnValue = ref->signalReceived(qSignal, qValue);
            ++i;
        }
    }
    
    if(i == 1) {
        const char *data = returnValue.toStdString().c_str();
        return strdup(data);
    }
    return NULL;
}
