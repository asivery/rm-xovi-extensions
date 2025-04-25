#pragma once

#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStringList>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

#include "../xovi.h"
#include "messagebroker.h"

class XoviMessageBroker : public QObject
{
    Q_OBJECT
public:
    explicit XoviMessageBroker(QObject *parent = nullptr) : QObject(parent) {
        messagebroker::addBroadcastListener(this);
    }

    ~XoviMessageBroker() {
        messagebroker::removeBroadcastListener(this);
    }

    Q_INVOKABLE QString sendSimpleSignal(const QString &signal, const QString &message)
    {
        std::string stdSignal = signal.toStdString(), stdParam = message.toStdString();
        const char *cStrName = stdSignal.c_str();
        const char *cStrParam = stdParam.c_str();
        struct ExtensionMetadataIterator iter;
        struct XoviMetadataEntry *entry;
        Environment->createMetadataSearchingIterator(&iter, "xovi-message-broker$simpleSignal");
        char *ret = NULL;
        int i = 0;
        while((entry = Environment->nextFunctionMetadataEntry(&iter))) {
            if(strncmp(entry->value.s, cStrName, entry->value.sLength) == 0) {
                struct XoviMetadataEntry *versionMetadata = Environment->getMetadataEntryForFunction(
                    iter.extensionName,
                    iter.functionName,
                    LP1_F_TYPE_EXPORT,
                    "xovi-message-broker$version"
                );
                if(versionMetadata == NULL) {
                    qDebug() << "[xovi-message-broker]: Version undefined for simpleSignal" << cStrName;
                }
                int version = versionMetadata->value.i;
                // Found. Invoke.
                if(version == 1) {
                    if(ret != NULL) free(ret);
                    ret = ((char *(*)(const char *)) iter.functionAddress)(cStrParam);
                    ++i;
                }
            }
        }
        QString r;
        if(i == 1) {
            r = ret ? QString(ret) : QString();
        }
        if(ret) free(ret);

        return r;
    }

    void setListeningFor(const QStringList &l) {
        _listeningFor = l;
    }

    const QStringList& getListeningFor() {
        return _listeningFor;
    }

    Q_PROPERTY(QStringList listeningFor READ getListeningFor WRITE setListeningFor);

signals:
    QString signalReceived(const QString &signal, const QString &message);
private:
    QStringList _listeningFor;
};

