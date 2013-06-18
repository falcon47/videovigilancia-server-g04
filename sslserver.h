#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QObject>

#include <QtNetwork>
#include <string>
#include "hilo.h"

class Server : public QTcpServer
{
  Q_OBJECT

public:
  virtual void incomingConnection(int socketDescriptor);
signals:
    void new_conecction(int);
};



class sslserver : public QObject
{
    Q_OBJECT
public:
    int port;
    QString key;
    QString certificate;
    QString dir;
    int cores;
    QMutex * mutex;
    QMutex * mutexMmap;
    QFile * riff;

    explicit sslserver(QObject *parent = 0);
    ~sslserver();
    void listen();
    void create_threads();

signals:
    void new_client(int,int);

public slots:
    void repartir_cliente(int);

private:
    Server * server;
    QList<Hilo *> Pool_Threads;
    QList<QThread* > hilos;

};

#endif // SSLSERVER_H
