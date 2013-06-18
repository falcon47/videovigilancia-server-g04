#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtNetwork>
#include <string>
#include "riff.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    ~Client();
signals:
    void received(std::string &);
    void socket_down();
public slots:
    void handshakeComplete();
    void sslErrors(const QList<QSslError> &);
    void connectionFailure();
    void receiveMessage();
    void connectionClosed();
public:
    QSslSocket * s;
    void initialize(QSslSocket*,QString key,QString certificate);
    int64_t read_buffer_sz;
    C_list * cam_list;
};

#endif // CLIENT_H
