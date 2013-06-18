#include "consola.h"

#include <QDebug>

#include <QImage>

#include <QSettings>
// socket
#include <sys/socket.h>
// write read setuid
#include <unistd.h>
// sigaction
#include <signal.h>
// umask
#include <sys/types.h>
#include <sys/stat.h>
// Syslog
 #include <syslog.h>

#include <fstream>

//needed to not get an undefined reference to static members
int Consola::sigHupSd[2];
int Consola::sigTermSd[2];


Consola::Consola(QObject *parent) :
    QObject(parent)
{
    server = new sslserver;

    //CONFIGURACION DEL SERVIDOR USANDO QSETTINGS
            //QSettings config(APP_CONFFILE, QSettings::IniFormat);
            QSettings config("config", QSettings::IniFormat);

            server->port = config.value("puerto", "").toInt();
            server->key = config.value("key", APP_DATADIR + QString("/SSL/server.key")).toString();
            server->certificate = config.value("cert", APP_DATADIR + QString("/SSL/server.ctr")).toString();
            server->dir = config.value("directorio",APP_VARDIR).toString();
            server->cores = config.value("cores",8).toInt();
            // Cambia el usuario
            setuid(config.value("uid",0).toInt());

            // Cambia el grupo
            setgid(config.value("gid",0).toInt());

            // Cambia la umask
            bool ok;
            umask(config.value("umask",0).toString().toUInt(&ok,8));


   //FIN DE CONFIGURACIÓN DEL SERVER.

   // POSIX SIGNALS
    // Crear las parejas de sockets UNIX
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupSd))
            qFatal("Couldn't create HUP socketpair");
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermSd))
            qFatal("Couldn't create TERM socketpair");

        // Crear los objetos para monitorizar uno de los socket
        // de cada pareja.
        sigHupNotifier = new QSocketNotifier(sigHupSd[1],
            QSocketNotifier::Read, this);
        sigTermNotifier = new QSocketNotifier(sigTermSd[1],
            QSocketNotifier::Read, this);

        // Conectar la señal activated() de cada objeto
        // QSocketNotifier con el slot correspondiente. Esta señal
        // será emitida cuando hayan datos para ser leidos en el
        // socket monitorizado.
        connect(sigHupNotifier, SIGNAL(activated(int)), this,
            SLOT(handleSigHup()));
        connect(sigTermNotifier, SIGNAL(activated(int)), this,
            SLOT(handleSigTerm()));
     // FIN POSIX SIGNALS
     // ARCHIVO DE METADATOS


        if(!QFile(server->dir + "/metadatos.vi").exists())
        {
            QDir d;
            d.mkpath(server->dir);

            riff = new QFile(server->dir + "/metadatos.vi");
            if(!riff->open(QFile::ReadWrite)) qDebug() << "Fallo abriendo archivo";

            qDebug() << (server->dir + "/metadatos.vi").toStdString().c_str();
            //std::fstream out((server->dir + "/metadatos.vi").toStdString().c_str(),std::ios_base::binary | std::fstream::out);
            char buffer[SIZEOF_RIFF + SIZEOF_HEADER];

            if(!riff->resize(SIZEOF_RIFF + SIZEOF_HEADER))
            {
                qDebug() << "Fallo cambiando tamaño de archivo";
                riff->write(buffer,SIZEOF_RIFF + SIZEOF_HEADER);
                riff->flush();
            }
            //out.write(buffer,SIZEOF_RIFF + SIZEOF_HEADER);
            //out.close();

            RIFF * riff_head = reinterpret_cast<RIFF *> (riff->map(0,SIZEOF_RIFF + SIZEOF_HEADER));
            if(riff_head == 0) qDebug() << "Fallo mapenado el archivo";

            HEADER * Header = reinterpret_cast<HEADER *>(riff_head + 1);

            riff_head->type = char2int("RIFF");
            riff_head->size = SIZEOF_RIFF - 8 + SIZEOF_HEADER;
            riff_head->listType = char2int("VIVI");

            Header->num_img = 0;
            Header->num_clist = 0;
            Header->size = SIZEOF_HEADER - 8;
            Header->type = char2int("HEAD");

            riff->unmap(reinterpret_cast<uchar*>(riff_head));
        }
        else
        {
            riff = new QFile(server->dir + "/metadatos.vi");
            if(!riff->open(QFile::ReadWrite)) qDebug() << "Fallo abriendo archivo";
        }
     // FIN ARCHIVO DE METADATOS
    server->riff = riff;
    server->create_threads();
    server->listen();
}

Consola::~Consola()
{
    server->deleteLater();
    sigHupNotifier->deleteLater();
    sigTermNotifier->deleteLater();
    riff->close();
    riff->deleteLater();
}


//
// Manejador de la señal SIGHUP
//
void Consola::hupSignalHandler(int)
{
    char a = 1;
    ::write(sigHupSd[0], &a, sizeof(a));
}

//
// Manejador de la señal SIGTERM
//
void Consola::termSignalHandler(int)
{

    char a = 1;
    ::write(sigTermSd[0], &a, sizeof(a));
}

//
// Configurar los manejadores de señal
//

int setupUnixSignalHandlers()
{
    struct ::sigaction hup, term;

    hup.sa_handler = &Consola::hupSignalHandler;
    ::sigemptyset(&hup.sa_mask);
    hup.sa_flags = SA_RESTART;

    // Establecer manejador de la señal SIGHUP
    if (::sigaction(SIGHUP, &hup, 0) > 0)
    return 1;

    term.sa_handler = &Consola::termSignalHandler;
    ::sigemptyset(&term.sa_mask);
    term.sa_flags = SA_RESTART;

    // Establecer manejador de la señal SIGTERM
    if (::sigaction(SIGTERM, &term, 0) > 0)
    return 2;

    return 0;
}

void Consola::handleSigHup()
{
    sigHupNotifier->setEnabled(false);
    char tmp;
    ::read(sigHupSd[1], &tmp, sizeof(tmp));

    // Elimina el servidor para usar uno nuevo con la nueva configuracion
    server->deleteLater();

    server = new sslserver;


    //QSettings config(APP_CONFFILE, QSettings::IniFormat);
    QSettings config("config", QSettings::IniFormat);

    server->port = config.value("puerto", "").toInt();
    server->key = config.value("key", APP_DATADIR + QString("/SSL/server.key")).toString();
    server->certificate = config.value("cert", APP_DATADIR + QString("/SSL/server.ctr")).toString();
    server->dir = config.value("directorio",APP_VARDIR).toString();
    server->cores = config.value("cores",8).toInt();

    server->create_threads();
    server->listen();


    sigHupNotifier->setEnabled(true);
}

void Consola::handleSigTerm()
{
    sigTermNotifier->setEnabled(false);
    char tmp;
    ::read(sigTermSd[1], &tmp, sizeof(tmp));

    deleteLater();

    sigTermNotifier->setEnabled(true);
}

