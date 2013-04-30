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
     read_buffer_sz = 0;
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

  QObject::connect(socket, SIGNAL(encrypted()), this, SLOT(handshakeComplete()));
  QObject::connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)),
                   this, SLOT(sslErrors(const QList<QSslError> &)));
  QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                   this, SLOT(connectionFailure()));

  socket->setPrivateKey(key);
  socket->setLocalCertificate(certificate);

  socket->setPeerVerifyMode(QSslSocket::VerifyNone);
  socket->setProtocol(QSsl::SslV3);

  socket->startServerEncryption();
}

// Recibe la notificacion de que el handshake esta terminado y
// mete el nuevo socket en la lista de sockets

void sslserver::handshakeComplete()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(QObject::sender());

  QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

  sockets.push_back(socket);
}

void sslserver::sslErrors(const QList<QSslError> &errors)
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(QObject::sender());


  QString errorStrings;
  foreach (QSslError error, errors)
  {
    errorStrings += error.errorString();
    if (error != errors.last())
    {
      errorStrings += ';';
    }
  }
  qDebug() << "ERROR" << errorStrings;
  socket->ignoreSslErrors();
}

void sslserver::receiveMessage()
{
    QSslSocket *socket = dynamic_cast<QSslSocket *>(QObject::sender());

    qDebug() << "Datos recividos!";
    if(read_buffer_sz == 0 && socket->bytesAvailable () > sizeof(int64_t))
    {
        socket->read((char *)&read_buffer_sz, sizeof(read_buffer_sz));
    }

 do {
      qDebug() << "Tamanyo del primer paquete" << read_buffer_sz;
      qDebug() << "Tamanyo del buffer" << socket->bytesAvailable ();

          if(socket->bytesAvailable () > read_buffer_sz && read_buffer_sz != -1)
          {
              std::string buffer;
              buffer.resize(read_buffer_sz);


              socket->read(const_cast<char*>(buffer.c_str()), read_buffer_sz);
              qDebug() << "Buffer leido";

              emit received(buffer);
              read_buffer_sz = 0;
          }
          if(read_buffer_sz == 0 && socket->bytesAvailable () > sizeof(int64_t))
          {
              socket->read((char *)&read_buffer_sz, sizeof(read_buffer_sz));
          }

 } while(read_buffer_sz < socket->bytesAvailable () && read_buffer_sz != 0);

}

void sslserver::connectionClosed()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
  qDebug() << "Coneccion cerrada";
}

void sslserver::connectionFailure()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  qDebug() << "Fallo en la conexion" << socket->errorString();
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}
