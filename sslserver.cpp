#include "sslserver.h"
#include <stdint.h>

// Intercepta la nueva coneccion y habilita SSL
void Server::incomingConnection(int socketDescriptor)
{
    emit new_conecction(socketDescriptor);
}

sslserver::sslserver(QObject *parent) :
    QObject(parent)
{
    server = new Server;
    mutex = new QMutex;
    mutexMmap = new QMutex;
    connect(server,SIGNAL(new_conecction(int)),this,SLOT(repartir_cliente(int)));
}

void sslserver::listen()
{
    if(!server->listen(QHostAddress::Any, port))
    {
        qDebug() << "No se ha podido poner a escuchar el servidor (puerto ocupado)";
        exit(53);
    }
}

void sslserver::create_threads()
{
    for(int i = 0;i < cores - 1;++i)
    {
        QThread * tr = new QThread;
        Hilo * h = new Hilo();
        h->initialize(i,key,certificate,dir,mutex,mutexMmap,riff);

        connect(this,SIGNAL(new_client(int,int)),h,SLOT(recieve_client(int,int)));

        h->moveToThread(tr);
        tr->start();
        Pool_Threads.push_back(h);
        hilos.push_back(tr);
    }
}
void sslserver::repartir_cliente(int socketDescriptor)
{
    int min = Pool_Threads[0]->n_clientes;
    int ii = 0;
    for(int i = 0; i < Pool_Threads.size();i++)
        if(Pool_Threads[i]->n_clientes < min)
        {
            min = Pool_Threads[i]->n_clientes;
            ii = i;
        }
    emit new_client(Pool_Threads[ii]->_id,socketDescriptor);
}

sslserver::~sslserver()
{
    server->disconnect();
    server->deleteLater();
    delete mutex;
    delete mutexMmap;

    for(int i = 0; i < Pool_Threads.size();++i)
        Pool_Threads[i]->deleteLater();

    for(int i = 0; i < hilos.size();++i)
        hilos[i]->deleteLater();
}
