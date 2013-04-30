#ifndef CONSOLA_H
#define CONSOLA_H

#include <QObject>

#include "sslserver.h"
#include <string>
#include "protocol.pb.h"

class Consola : public QObject
{
    Q_OBJECT
public:
    explicit Consola(QObject *parent = 0);
    ~Consola();
    
signals:
    
public slots:
    void deserilaize(const std::string &);

private:
    sslserver server;
    QString dir;
    qint64 direccion;
};

#endif // CONSOLA_H
