#include "broadcastrcv.h"
broadcastRCV::broadcastRCV(QObject *parent) :
    QObject(parent)
{
    udpSocket = new QUdpSocket();
    udpSocket->bind(45454,QUdpSocket::ShareAddress);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    brAddressRcv = "127.0.0.1";
    brPortRcv = 7007;
}
void broadcastRCV::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress add; quint16 port;
        udpSocket->readDatagram(datagram.data(),datagram.size(),&add,&port);
        brAddressRcv = add.toString();
        brPortRcv = port;
        emit Address_Port_Recieved();
        qDebug() << "Server Address is" << add.toString() << "port is" << port;
    }
}
