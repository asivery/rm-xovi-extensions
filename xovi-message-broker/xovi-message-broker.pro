TEMPLATE = lib
TARGET = xovi-message-broker
CONFIG += shared plugin no_plugin_name_prefix

xoviextension.target = xovi.cpp
xoviextension.commands = python3 $$(XOVI_REPO)/util/xovigen.py -o xovi.cpp -H xovi.h xovi-message-broker.xovi
xoviextension.depends = xovi-message-broker.xovi

QMAKE_EXTRA_TARGETS += xoviextension
PRE_TARGETDEPS += xovi.cpp

QT += quick qml

CONFIG += c++11

SOURCES += src/main.cpp src/pipes.cpp src/messagebroker.cpp xovi.cpp

HEADERS += src/XoviMessageBroker.h

QMAKE_CXXFLAGS += -fPIC 
