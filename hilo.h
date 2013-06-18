#ifndef HILO_H
#define HILO_H

#include <QObject>
#include <client.h>
#include "riff.h"


class Hilo : public QObject
{
    Q_OBJECT
public:
    explicit Hilo(QObject *parent = 0);
    ~Hilo();
    void initialize(int id, QString k, QString cert, QString dir_, QMutex * m, QMutex * m2, QFile *riff_file);
    int _id;
    int n_clientes;
    QList<Client *> clientes;
    QString key;
    QString certificate;
    QMutex * mutex;
    QMutex * mutexMmap;
    QString dir;
    QFile * file;

    HEADER * cabecera;
signals:
    
public slots:
    void recieve_client(int id,int socket_descriptor);
    void pkg_rcv(std::string &);
    void delete_socket();
    
};

#endif // HILO_H
