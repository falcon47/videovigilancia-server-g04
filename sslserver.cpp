#include "sslserver.h"
#include <stdint.h>

// Intercepta la nueva coneccion y habilita SSL
void Server::incomingConnection(int socketDescriptor)
{
  QSslSocket *serverSocket = new QSslSocket();
  if (serverSocket->setSocketDescriptor(socketDescriptor))
  {
    addPendingConnection (serverSocket);
  }
  else
  {
    delete serverSocket;
  }
}

sslserver::sslserver(QObject *parent) :
    QObject(parent)
{
     connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

void sslserver::listen()
{
    server.listen(QHostAddress::Any, port);
    qDebug() << "servidor escuchando en puerto" << port;
}

/*#####################################
//
//              SLOTS
//
#######################################*/

// Acepta la coneccion y inicia el handshake
void sslserver::acceptConnection()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(server.nextPendingConnection());

  Client * cliente = new Client;
  cliente->initialize(socket,key,certificate);

  connect(cliente,SIGNAL(received(std::string &)),
          this,SLOT(pkg_rcv(std::string &)));
  connect(cliente,SIGNAL(socket_down()),
          this,SLOT(delete_socket()));
  Clientes.push_back(cliente);
}

void sslserver::pkg_rcv(std::string & pkg)
{
    emit received(pkg);
}

void sslserver::delete_socket()
{
    Client * client = dynamic_cast<Client*> (QObject::sender());
    Clientes.removeOne(client);
    client->deleteLater();
}

// Recibe la notificacion de que el handshake esta terminado y
// mete el nuevo socket en la lista de sockets
