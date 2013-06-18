#include "hilo.h"
#include "protocol.pb.h"
#include <QImage>
#include <fstream>


#define CHAR2INT(x) (u_int32_t) x

Hilo::Hilo(QObject *parent):
    QObject(parent)
{

}
void Hilo::initialize(int id,QString k,QString cert,QString dir_,QMutex * m,QMutex * m2,QFile * riff_file)
{
    _id = id;
    n_clientes = 0;
    key = k;
    certificate = cert;
    dir = dir_;
    mutex = m;
    mutexMmap = m2;
    file = riff_file;
    cabecera = reinterpret_cast<HEADER*> (file->map(SIZEOF_RIFF,SIZEOF_HEADER));
}

void Hilo::recieve_client(int id,int socket_descriptor)
{
    if (id == _id)
    {
         QSslSocket *serverSocket = new QSslSocket();
         serverSocket->setSocketDescriptor(socket_descriptor);
         Client * c = new Client;
         c->initialize(serverSocket,key,certificate);

         connect(c,SIGNAL(received(std::string &)),
                 this,SLOT(pkg_rcv(std::string &)));
         connect(c,SIGNAL(socket_down()),
                 this,SLOT(delete_socket()));
         clientes.push_back(c);
         ++n_clientes;
    }
}

void Hilo::pkg_rcv(std::string & pkg)
{
    Client * client = dynamic_cast<Client*> (QObject::sender());
    paquete msg;
    Cuadros cuadros;
    int direccion;
    msg.ParseFromString(pkg);

    QImage imagen;
    imagen.loadFromData(reinterpret_cast<const uchar *>(msg.img().c_str()), msg.img().size(), "JPEG");

    QString name;
    QString cero;

    mutex->lock();

        direccion = cabecera->num_img;
        cabecera->num_img++;

    mutex->unlock();

    // Guardar imagen
    name.setNum(direccion,16);
    name = cero.fill('0',16 - name.length()) + name;

    QDir d;
    d.mkpath(dir + "/" + name.mid(0, 4) + "/" +
             name.mid(5, 4) + "/" + name.mid(9, 4));

    imagen.save(QDir::toNativeSeparators(dir + "/" +
                                         name.mid(0, 4) + "/" +
                                         name.mid(5, 4) + "/" +
                                         name.mid(9, 4) + "/" +
                                         name + ".jpeg"),"JPEG");
    // Protocolbuffer para los rectangulos
    for(int i = 0; i < msg.recta_size(); ++i)
    {
         Cuadros_rect * r = cuadros.add_recta();
         r->set_x(msg.recta(i).x());
         r->set_y(msg.recta(i).y());
         r->set_width(msg.recta(i).width());
         r->set_height(msg.recta(i).height());
    }

    /// RIFF
    ///Si el archivo aun no esta mapeado lo mapea
    if(client->cam_list == NULL)
    {
        mutexMmap->lock();

            RIFF * riff_head = reinterpret_cast<RIFF *> (file->map(0,SIZEOF_RIFF));
            HEADER * header = reinterpret_cast<HEADER *> (file->map(SIZEOF_RIFF, SIZEOF_HEADER));

            C_list * cam_list;
            CAM * cam;

            // Busca la entrada correspondiente a esta camara
            for(unsigned i = 0; i < header->num_clist; ++i)
            {
                cam_list = reinterpret_cast<C_list*> (file->map(SIZEOF_RIFF + SIZEOF_HEADER + (SIZEOF_CLIST * i),SIZEOF_CLIST));
                cam = reinterpret_cast<CAM*> ((reinterpret_cast<u_int8_t*> (cam_list)) + 12);

                // Si la encuentra sale si no pasa a la siguiente
                if(QString(reinterpret_cast<char*>(cam->name)) == QString(msg.devicename().c_str()) && cam->ultimo == 1)
                {
                    client->cam_list = cam_list;
                    break;
                }
                else
                {
                    file->unmap(reinterpret_cast<uchar*>(cam_list));
                }
            }


            // Si no encontro la entrada para esta camara la crea
            if(client->cam_list == NULL)
            {

                file->resize(riff_head->size + 8 + SIZEOF_CLIST);

                cam_list = reinterpret_cast<C_list*> (alignPointer(file->map(riff_head->size + 8 ,SIZEOF_CLIST),2));

                QString listype;
                listype.setNum(header->num_clist,10);
                listype = "C" + cero.fill('0',3 - listype.length()) + listype;

                cam_list->listType = char2int(listype.toStdString().c_str());
                cam_list->type = char2int("LIST");
                cam_list->size = SIZEOF_CLIST - 8;

                cam = reinterpret_cast<CAM*> (alignPointer((reinterpret_cast<u_int8_t*> (cam_list)) + 12, 2));

                strcpy(reinterpret_cast<char*>(cam->name), msg.devicename().c_str());
                cam->size_taken = 0;
                cam->type = char2int("CAM ");
                cam->size = SIZEOF_CAM - 8;
                cam->ultimo = 1;
                client->cam_list = cam_list;

                riff_head->size = riff_head->size + SIZEOF_CLIST;
                header->num_clist++;


           }

            file->unmap(reinterpret_cast<uchar*>(riff_head));
            file->unmap(reinterpret_cast<uchar*>(header));

        mutexMmap->unlock();

    }

    // Busca donde meter la imagen
    //img = primera imagen
    IMG * img = reinterpret_cast<IMG*> (alignPointer(reinterpret_cast<u_int8_t*> (client->cam_list) + 12 + SIZEOF_CAM,2));
    CAM * cam = reinterpret_cast<CAM*> (alignPointer(reinterpret_cast<u_int8_t*>(client->cam_list) + 12,2));

    // No cabe la imagen hay que crear un nuevo C_list
    if(SIZEOF_CLIST - SIZEOF_CAM - cam->size_taken < 24 + cuadros.SerializeAsString().size() + (cuadros.SerializeAsString().size() % 2))
    {
        mutexMmap->lock();

            // Mapea la cabecera riff para usarla y actualizarla
            RIFF * riff_head = reinterpret_cast<RIFF *> (file->map(0,SIZEOF_RIFF));
            HEADER * header = reinterpret_cast<HEADER *> (file->map(SIZEOF_RIFF, SIZEOF_HEADER));

            file->resize(riff_head->size + 8 + SIZEOF_CLIST);

            // Crea el nuevo C_list
            C_list * cam_list = reinterpret_cast<C_list*> (alignPointer(file->map(riff_head->size + 8 ,SIZEOF_CLIST),2));
            // Setea el nuevo C_list
            cam_list->listType = client->cam_list->listType;
            cam_list->type = char2int("LIST");
            cam_list->size = SIZEOF_CLIST - 8;

            CAM * cam_ = reinterpret_cast<CAM*> (alignPointer((reinterpret_cast<u_int8_t*> (cam_list)) + 12, 2));

            // Setea el nuevo CAM
            strcpy(reinterpret_cast<char*>(cam_->name), msg.devicename().c_str());
            cam_->size_taken = 0;
            cam_->type = char2int("CAM ");
            cam_->size = SIZEOF_CAM - 8;
            cam_->ultimo = 1;

            // Quita como ultimo el anterior C_list
            cam_ = reinterpret_cast<CAM*> ((reinterpret_cast<u_int8_t*> (client->cam_list)) + 12);
            cam_->ultimo = 0;

            // Desmapea el C_list lleno
            file->unmap(reinterpret_cast<uchar*>(client->cam_list));

            // Le pone al cliente el nuevo C_list vacio
            client->cam_list = cam_list;

            // Actualiza el tamaño de riff
            riff_head->size = riff_head->size + SIZEOF_CLIST;
            header->num_clist++;

            file->unmap(reinterpret_cast<uchar*>(riff_head));
            file->unmap(reinterpret_cast<uchar*>(header));


        mutexMmap->unlock();

        // Actualizamos cam
        cam = reinterpret_cast<CAM*> (alignPointer(reinterpret_cast<u_int8_t*>(client->cam_list) + 12, 2));

    }


    // Nos desplazamos hasta la ultima IMG
    img = reinterpret_cast<IMG*> (alignPointer(reinterpret_cast<u_int8_t*>(img) + cam->size_taken, 2));

    img->num_img = direccion;
    img->timestamp = msg.timestamp();
    img->type = char2int("IMG ");
    if(cuadros.recta_size() != 0)
    {
        strcpy(reinterpret_cast<char*>(img->cuadros), cuadros.SerializeAsString().c_str());
        img->cuadros_size = cuadros.SerializeAsString().size();
        img->size = SIZEOF_IMG + cuadros.SerializeAsString().size() - 8 + (img->cuadros_size % 2);
        cam->size_taken += SIZEOF_IMG + cuadros.SerializeAsString().size() + (img->cuadros_size % 2);
    }
    else
    {
        img->cuadros_size = 0;
        img->size = SIZEOF_IMG - 8;
        cam->size_taken += SIZEOF_IMG;
    }


}

void Hilo::delete_socket()
{
    Client * client = dynamic_cast<Client*> (QObject::sender());
    clientes.removeOne(client);

    if (client->cam_list != NULL)
        file->unmap(reinterpret_cast<uchar*>(client->cam_list));

    client->deleteLater();

    --n_clientes;
}

Hilo::~Hilo()
{
    for(int i = 0; i < clientes.size();++i)
        clientes[i]->deleteLater();

    file->deleteLater();
}
