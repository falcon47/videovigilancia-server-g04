#-------------------------------------------------
#
# Project created by QtCreator 2013-04-16T13:31:47
#
#-------------------------------------------------

QT       += core network gui


TARGET = Server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    consola.cpp \
    sslserver.cpp

HEADERS += \
    consola.h \
    sslserver.h

PROTOS += protocol.proto

include(protobuf.pri)

LIBS	+= -lprotobuf

OTHER_FILES += \
    protocol.proto \
