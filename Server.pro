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
    sslserver.cpp \
    client.cpp \
    hilo.cpp \
    riff.cpp

HEADERS += \
    consola.h \
    sslserver.h \
    client.h \
    hilo.h \
    riff.h

PROTOS += protocol.proto

include(protobuf.pri)

LIBS	+= -lprotobuf

OTHER_FILES += \
    protocol.proto \

unix {          # Esta configuración específica de Linux y UNIX
    # Variables
    #
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    BINDIR  = $$PREFIX/bin
    DATADIR = /var/lib/$${TARGET}
    CONFDIR = /etc
    isEmpty(VARDIR) {
        VARDIR  = /var/lib/$${TARGET}
    }

    DEFINES += APP_DATADIR=\\\"$$DATADIR\\\"
    DEFINES += APP_VARDIR=\\\"$$VARDIR\\\"
    DEFINES += APP_CONFFILE=\\\"$$CONFDIR/$${TARGET}.conf\\\"

    # Install
    #
    INSTALLS += target config ssl vardir

    ## Instalar ejecutable
    target.path = $$BINDIR

    ## Instalar archivo de configuración
    config.path = $$CONFDIR
    config.files += $${TARGET}.ini

    ## Instalar acceso directo en el menú del escritorio
    # desktop.path = $$DATADIR/applications
    # desktop.files += $${TARGET}.desktop

    ## Instalar ssl
    ssl.path = $$DATADIR/SSL
    ssl.files += ./SSL/gen_ssl \
                 ./SSL/server.crt \
                 ./SSL/server.key

    ## Crear directorio de archivos variables
    vardir.path = $$VARDIR
    vardir.commands = :
}
