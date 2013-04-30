#include <QCoreApplication>
#include "consola.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Consola cmd;
    return a.exec();
}
