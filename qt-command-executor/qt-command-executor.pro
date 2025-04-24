# Define the project name
TEMPLATE = lib
TARGET = qt-command-executor
CONFIG += shared plugin no_plugin_name_prefix

# Define the Qt modules required
QT += quick qml

# Define the C++ standard version
CONFIG += c++11

# Specify the source files
SOURCES += main.cpp

HEADERS += CommandExecutor.h

QMAKE_CXXFLAGS += -fPIC 

# QMAKE_CXX = aarch64-remarkable-linux-g++
