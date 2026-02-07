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
        int hitCount;
        char *ret = broadcastToNative(cStrName, cStrParam, &hitCount);
        QString r;
        if(hitCount == 1) {
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

private:
    QStringList _listeningFor;
};
