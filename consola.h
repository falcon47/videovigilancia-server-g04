#ifndef CONSOLA_H
#define CONSOLA_H

#include <QObject>

#include "sslserver.h"
#include <string>
#include "protocol.pb.h"


#include <QtGlobal>

#include "riff.h"


int setupUnixSignalHandlers();


class Consola : public QObject
{
    Q_OBJECT
public:
    explicit Consola(QObject *parent = 0);
    ~Consola();
    
    // Manejadores de señal POSIX
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

public slots:
    // Slots Qt donde atender las señales POSIX
    void handleSigHup();
    void handleSigTerm();

private:
    // Pares de sockets. Un par por señal a manejar
    static int sigHupSd[2];
    static int sigTermSd[2];

   // Objetos para monitorizar los pares de sockets
   QSocketNotifier *sigHupNotifier;
   QSocketNotifier *sigTermNotifier;

   // Servidor ssl pool de hilos asincrono
   sslserver * server;

   // Archivo mapeado en memoria
   QFile * riff;
};

#endif // CONSOLA_H
