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
            QVariant rawReturn;
            QMetaObject::invokeMethod(
                ref,
                "onSignalReceived",
                Qt::BlockingQueuedConnection,
                Q_RETURN_ARG(QVariant, rawReturn),
                Q_ARG(QVariant, qSignal),
                Q_ARG(QVariant, qValue)
            );
            returnValue = rawReturn.toString();
            qDebug() << "[xovi-message-broker] Response for" << qSignal << ":" << qValue << " is " << returnValue;
            ++i;
        }
    }

    if(i == 1 && !returnValue.isEmpty()) {
        std::string stdRet = returnValue.toStdString();
        const char *data = stdRet.c_str();
        return strdup(data);
    }
    return NULL;
}

extern "C" char *broadcastToNative(const char *signal, const char *value, int *countOfHits) {
    struct ExtensionMetadataIterator iter;
    struct XoviMetadataEntry *entry;
    Environment->createMetadataSearchingIterator(&iter, "xovi-message-broker$simpleSignal");
    char *ret = NULL;
    int i = 0;
    while((entry = Environment->nextFunctionMetadataEntry(&iter))) {
        if(strncmp(entry->value.s, signal, entry->value.sLength) == 0) {
            struct XoviMetadataEntry *versionMetadata = Environment->getMetadataEntryForFunction(
                iter.extensionName,
                iter.functionName,
                LP1_F_TYPE_EXPORT,
                "xovi-message-broker$version"
            );
            if(versionMetadata == NULL) {
                printf("[xovi-message-broker]: Version undefined for simpleSignal %s\n", signal);
                continue;
            }
            int version = versionMetadata->value.i;
            // Found. Invoke.
            if(version == 1) {
                if(ret != NULL) free(ret);
                ret = ((char *(*)(const char *)) iter.functionAddress)(value);
                ++i;
            }
        }
    }
    if(countOfHits) *countOfHits = i;
    return ret;
}
