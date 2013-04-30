#include "consola.h"

#include <QDebug>

#include <QImage>


#include <QSettings>

#include "protocol.pb.h"

Consola::Consola(QObject *parent) :
    QObject(parent)
{
    // Registra QVector<QRect> como tipo en qt para reconocerlo al hacer connect
        qRegisterMetaType< QVector<QRect> >("QVector<QRect>");


    QObject::connect(&server, SIGNAL(received(const std::string &)),
                     this, SLOT(deserilaize(const std::string &)));

    //CONFIGURACION DEL SOCKET USANDO QSETTINGS
        // Creamos el objeto de acceso a archivo datos
            QSettings config("config", QSettings::IniFormat);
            server.port = config.value("puerto", "").toInt();
            server.key = config.value("key", "").toString();
            server.certificate = config.value("cert", "").toString();
            dir = config.value("directorio","").toString();
            direccion = config.value("direccion_actual","").toInt();
            qDebug() << server.port << " " <<
                        server.key  <<" " <<
                        server.certificate <<" " <<
                        dir <<" " <<
                        direccion;
   //FIN DE CONFIGURACIÓN DEL SERVER.

    server.listen();
}

Consola::~Consola()
{

}

void Consola::deserilaize(const std::string & buffer)
{
    qDebug() << "Deserializando";
    paquete msg;
    msg.ParseFromString(buffer);

    QImage imagen;
    imagen.loadFromData((const uchar *)msg.img().c_str(), msg.img().size(), "JPEG");

    QVector<QRect> cuadros;
    for(int i = 0; i < msg.recta_size(); ++i)
    {
        QRect recta(msg.recta(i).x(), msg.recta(i).y(), msg.recta(i).width(), msg.recta(i).height());
        cuadros.push_back(recta);
    }

    QString name;
    QString cero;
    //RELLENAR CON 00
    name.setNum(direccion,16);
    name = cero.fill('0',16 - name.length()) + name;

    qDebug() << "Guardando imagen en " << dir + "/" +
                name.mid(0, 4) + "/" +
                name.mid(5, 4) + "/" +
                name.mid(9, 4) + "/" +
                name + ".jpeg" << " " << direccion;
    QDir d;
    d.mkpath(dir + "/" +
             name.mid(0, 4) + "/" +
             name.mid(5, 4) + "/" +
             name.mid(9, 4));
    imagen.save(QDir::toNativeSeparators(dir + "/" +
                                         name.mid(0, 4) + "/" +
                                         name.mid(5, 4) + "/" +
                                         name.mid(9, 4) + "/" +
                                         name + ".jpeg"),"JPEG");
    ++direccion;
    QSettings config("config", QSettings::IniFormat);
    config.setValue("direccion_actual",direccion);
}


