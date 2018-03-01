#include <QCoreApplication>
#include <controller.h>
#include <iostream>
#include <smscontroller.h>
#include <QtNetwork>
#include <QNetworkRequest>
#include <QSettings>
#include <QCoreApplication>
#include <QSettings>
#include <telegrambot.h>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    SmsController *sc = new SmsController();
//    sc->ReadSignalStrengh();

    Controller *myController;
    myController = new Controller();
    myController->InitController();

    return a.exec();

}
