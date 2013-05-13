#include "client.h"

Client::Client(QObject *parent) :
    QObject(parent)
{
    read_buffer_sz = 0;
}

void Client::initialize(QSslSocket *socket,QString key,QString certificate)
{
    s = socket;
    QObject::connect(s, SIGNAL(encrypted()), this, SLOT(handshakeComplete()));
    QObject::connect(s, SIGNAL(sslErrors(const QList<QSslError> &)),
                     this, SLOT(sslErrors(const QList<QSslError> &)));
    QObject::connect(s, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(connectionFailure()));

    s->setPrivateKey(key);
    s->setLocalCertificate(certificate);
    s->setPeerVerifyMode(QSslSocket::VerifyNone);
    s->setProtocol(QSsl::SslV3);

    s->startServerEncryption();
}
void Client::handshakeComplete()
{
  QObject::connect(s, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  QObject::connect(s, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
}

void Client::sslErrors(const QList<QSslError> &errors)
{
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
  s->ignoreSslErrors();
}

void Client::receiveMessage()
{
    qDebug() << "Datos recividos!";
    if(read_buffer_sz == 0 && s->bytesAvailable () > sizeof(int64_t))
    {
        s->read((char *)&read_buffer_sz, sizeof(read_buffer_sz));
    }

 do {
      qDebug() << "Tamanyo del primer paquete" << read_buffer_sz;
      qDebug() << "Tamanyo del buffer" << s->bytesAvailable ();

          if(s->bytesAvailable () > read_buffer_sz && read_buffer_sz != -1)
          {
              std::string buffer;
              buffer.resize(read_buffer_sz);


              s->read(const_cast<char*>(buffer.c_str()), read_buffer_sz);
              qDebug() << "Buffer leido";

              emit received(buffer);
              read_buffer_sz = 0;
          }
          if(read_buffer_sz == 0 && s->bytesAvailable () > sizeof(int64_t))
          {
              s->read((char *)&read_buffer_sz, sizeof(read_buffer_sz));
          }

 } while(read_buffer_sz < s->bytesAvailable () && read_buffer_sz != 0);

}

void Client::connectionClosed()
{
  s->disconnect();
  s->deleteLater();
  qDebug() << "Coneccion cerrada";
  emit socket_down();
}

void Client::connectionFailure()
{
  qDebug() << "Fallo en la conexion" << s->errorString();
  s->disconnect();
  s->deleteLater();
  emit socket_down();
}
