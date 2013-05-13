#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QObject>

#include <QtNetwork>
#include <string>

#include "client.h"

class Server : public QTcpServer
{
  Q_OBJECT

public:
  virtual void incomingConnection(int socketDescriptor);
};



class sslserver : public QObject
{
    Q_OBJECT
public:
    int ip;
    int port;
    QString key;
    QString certificate;

    explicit sslserver(QObject *parent = 0);
    void listen();

signals:
    void received(const std::string &);

public slots:
    void acceptConnection();
    void pkg_rcv(std::string & pkg);
    void delete_socket();

private:
    Server server;
    QList<Client *> Clientes;
};

#endif // SSLSERVER_H
