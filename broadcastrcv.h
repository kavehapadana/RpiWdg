#ifndef BROADCASTRCV_H
#define BROADCASTRCV_H
#include <QtNetwork>

#include <QObject>

class broadcastRCV : public QObject
{
    Q_OBJECT
public:
    explicit broadcastRCV(QObject *parent = 0);
    QByteArray datagram;
    quint16 brPortRcv;
    QString brAddressRcv;
    broadcastRCV *broadCastRCV_Obj;

signals:
    void Address_Port_Recieved();

public slots:
    void processPendingDatagrams();
private:
    QUdpSocket *udpSocket;

};

#endif // BROADCASTRCV_H
