#include <QCoreApplication>
#include "consola.h"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

#include <errno.h>
// Open
#include <sys/stat.h>
#include <fcntl.h>
// Syslog
#include <syslog.h>
//ofstream
#include <fstream>
//remove
#include <stdio.h>

int main(int argc, char *argv[])
{
    bool daemon = false;
    if(argc >= 2)
    {
        for(int i = 1; i < argc;++i)
        {
            if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0)
            {
                std::cout << argv[0] << std::endl;
                std::cout << "Servidor de videovigilancia multiconexion multihilo asincrono demonizado" << std::endl;
                std::cout << "[-d | --daemon] Iniciar en modo demonio" << std::endl;
                std::cout << "[-h | --help | -?] Muestra esta ayuda" << std::endl;
                exit(0);
            }
            else if( strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0)
            {
                daemon = true;
                pid_t pid;

                // Nos clonamos a nosotros mismos creando un proceso hijo
                pid = fork();

                // Si pid es < 0, fork() falló
                if (pid < 0) {
                    // Mostrar la descripción del error y terminar
                    std::cerr << strerror(errno) << std::endl;
                    exit(10);
                }

                // Si pid es > 0, estamos en el proceso padre
                if (pid > 0) {
                    // Terminar el proceso
                    exit(0);
                }

                // Abrir una conexión al demonio syslog
                openlog(argv[0], LOG_NOWAIT | LOG_PID, LOG_USER);

                // Intentar crear una nueva sesión
                if (setsid() < 0) {
                    syslog(LOG_ERR, "No fue posible crear una nueva sesión\n");
                    exit(11);
                }

                // Cambiar directorio de trabajo
                if ((chdir("/")) < 0) {
                    syslog(LOG_ERR, "No fue posible cambiar el directorio de "
                                    "trabajo a /\n");
                    exit(12);
                }

                // Cerrar los descriptores de la E/S estándar
                close(STDIN_FILENO);            // fd 0
                close(STDOUT_FILENO);           // fd 1
                close(STDERR_FILENO);           // fd 2


                // Abrir nuevos descriptores de E/S
                int fd0 = open("/dev/null", O_RDONLY);  // fd0 == 0
                int fd1 = open("/dev/null", O_WRONLY);  // fd0 == 1
                int fd2 = open("/dev/null", O_WRONLY);  // fd0 == 2

                // Cambiar umask
                umask(0);

                syslog(LOG_NOTICE, "Demonio iniciado con éxito\n");

                // Hacer archivo con PID
                std::fstream out( (QString("/var/run/") + argv[0] + ".pid").toStdString().c_str());
                out << pid;
                out.close();

            }
            else
            {
                std::cout << "Parametro " << argv[i] << " incorrecto" << std::endl;
                std::cout << argv[0] << " [-h | --help | -?] Para mostrar ayuda" << std::endl;
                exit(0);
            }
        }

    }
    // Inicio del programa
    QCoreApplication a(argc, argv);
    Consola cmd;
    setupUnixSignalHandlers();

    int ret = a.exec();

    if(daemon){
        // Eliminar archivo con PID
        remove((QString("/var/run/") + argv[0] + ".pid").toStdString().c_str());
        // Cuando el demonio termine, cerrar la conexión con
        // el servicio syslog
        closelog();
    }

    return ret;
}
