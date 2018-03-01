#include "Tcp_Client.h"
#include <QDebug>
#include <CommonDefines.h>

Tcp_Client::Tcp_Client(QObject *parent) :
    QObject(parent)
{
    clientConnected = false;
    _tcpSocket = new QTcpSocket( this );
    msgStatus = UINIT;
    tcpCnnStatus_b = false;
}

void Tcp_Client::initClient(QString _add, quint16 _port)
{
    IP_Address = _add;// broadCastRCV_Obj->brAddressRcv;
    IP_Port = _port;// broadCastRCV_Obj->brPortRcv;
}

void Tcp_Client::initClient()
{

}

void Tcp_Client::broadCastRcvInit()
{
//    broadCastRCV_Obj = new broadcastRCV();
//    connect(broadCastRCV_Obj,SIGNAL(Address_Port_Recieved()),this,SLOT(connect2AQC_Server()));
}

void Tcp_Client::connect2Server()
{
    QByteArray data;
    data.append("Hi Server");

    QString _add = IP_Address;// broadCastRCV_Obj->brAddressRcv;
    quint16 _port = IP_Port;// broadCastRCV_Obj->brPortRcv;
    //_port = 6912;
    try
    {
        _tcpSocket->connectToHost(_add, _port);
    }
    catch(std::exception &e)
    {
        qDebug() << e.what();
    }
    catch(...)
    {
        qDebug() << "Exception Error";
    }

    if(_tcpSocket->waitForConnected(2000))
    {
        QMutexLocker m1(&_mclientConnected);
        connect( _tcpSocket, SIGNAL(readyRead()),this, SLOT(readTcpData()));
        clientConnected = true;
        _tcpSocket->write(data);
    }
    else
    {
        //QMutexLocker m1(&_mclientConnected);
        disconnect( _tcpSocket, SIGNAL(readyRead()),this, SLOT(readTcpData()));
        clientConnected = false;
        //broadCastRCV_Obj = NULL;
        //qDebug()<< "No Connetcion Stablished in fanc: connect2Server : ";
        //emit finishObject();
    }
}

void Tcp_Client::rxClientDataSlot()
{
    transMsgLAN(txClient_ID,  txClientMsg.size(),txClientMsg);
}

void Tcp_Client::rxClientDataOptSlot()
{
    transMsgOpt(txClientMsg);
    qDebug() << "Send Data tx Opt";
}

void Tcp_Client::readTcpData()
{
    QByteArray data = _tcpSocket->readAll();

    for(int i = 0; i < data.size(); i++)
        Parse_Byte(data[i]);

}

void Tcp_Client::Parse_Byte(u_int8_t DATA)
{
    switch (msgStatus)
    {
        case UINIT:
            if (DATA == LAN_Rx_Sync1)
                msgStatus = GOT_SYNC1;
            break;

        case GOT_SYNC1:
            if (DATA == LAN_Rx_Sync2)
            {
                msgStatus = GOT_SYNC2;
                CheckSumm = 0;
            }
            else
                goto error;
            break;

        case GOT_SYNC2:
            msgID = DATA;
            msgStatus = GOT_ID;
            CheckSumm+=DATA;
            break;

        case GOT_ID:
            msgLen = DATA;
            msgStatus = GOT_LEN;
            msgIndex = 0;
            msg.clear();
            CheckSumm += DATA;
            break;

        case GOT_LEN:
            msg.append(DATA);
            CheckSumm +=DATA;
            msgIndex++;

            if (msgIndex >= msgLen)
            {
                msgStatus = GOT_PAYLOAD;
            }
          break;

        case GOT_PAYLOAD:
            if(DATA != CheckSumm)
            {
                goto error;
                qDebug() << "Error in parsebyte of TCP_Client" << this->objectName();
            }
            this->txClient_ID = msgID;
            this->txClientMsg = msg;
            emit this->txClientDataSignal();
            //Parse_Message();
            goto restart;
    }
    return;
    error: errCnt++;
    restart: msgStatus = UINIT;
    return;
}

void Tcp_Client::transMsgLAN(u_int8_t Msg_ID,  uint msg_len,QByteArray out_msg)
{
    u_int8_t ck_sum = 0;
    QByteArray txData;

    txData.append(LAN_Tr_Sync1);
    txData.append(LAN_Tr_Sync2);
    txData.append(Msg_ID);

    QByteArray _byteArray;
    QDataStream stream(&_byteArray, QIODevice::WriteOnly);
    stream << msg_len;
    txData.append(_byteArray[3]);
    txData.append(_byteArray[2]);
    txData.append(_byteArray[1]);
    txData.append(_byteArray[0]);

    for(char cc = 2; cc < 7; cc++)
        ck_sum += txData[cc];

    for(uint j = 0; j < msg_len; j++)
    {
        txData.append(out_msg[j]);
        ck_sum += out_msg[j];
    }

    txData.append(ck_sum);
    if(_tcpSocket->state() == QTcpSocket::ConnectedState)
    {
        QMutexLocker m2(&_mclientConnected);
        _tcpSocket->write(txData);
        tcpCnnStatus_b = true;
    }
    else
        tcpCnnStatus_b = false;
}

void Tcp_Client::transMsgOpt(QByteArray msg)
{
    if(_tcpSocket->state() == QTcpSocket::ConnectedState)
    {
        QMutexLocker m3(&_mclientConnected);
        _tcpSocket->write(msg);
        tcpCnnStatus_b = true;
    }
    else
        tcpCnnStatus_b = false;
}

