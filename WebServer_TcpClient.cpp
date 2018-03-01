#include "WebServer_TcpClient.h"
#include "CommonDefines.h"
#include <QDebug>
WebServer_TcpClient::WebServer_TcpClient(QObject *parent) :
    QObject(parent)
{
    clientConnected = false;
    _aqcSocket = new QTcpSocket( this );
}

void WebServer_TcpClient::broadCastRcvInit()
{
//    broadCastRCV_Obj = new broadcastRCV();
//    connect(broadCastRCV_Obj,SIGNAL(Address_Port_Recieved()),this,SLOT(connect2AQC_Server()));
}

void WebServer_TcpClient::connect2WebServer()
{
    QByteArray data; // <-- fill with data
    data.append("Hi Server");

    QString _add = "192.168.1.17";// broadCastRCV_Obj->brAddressRcv;
    quint16 _port = 6912;// broadCastRCV_Obj->brPortRcv;
    _port = 6912;
    _aqcSocket->connectToHost(_add, _port);

    if(_aqcSocket->waitForConnected(2000))
    {
        //QMutexLocker m1(&_mclientConnected);
        connect( _aqcSocket, SIGNAL(readyRead()),this, SLOT(readTcpData()));
        clientConnected = true;
        _aqcSocket->write(data);
    }
    else
    {
        //QMutexLocker m1(&_mclientConnected);
        disconnect( _aqcSocket, SIGNAL(readyRead()),this, SLOT(readTcpData()));
        clientConnected = false;
        //broadCastRCV_Obj = NULL;
        qDebug()<< "No Connetcion Stablished in fanc: connect2AQC_Server";
        //emit finishObject();
    }
}

void WebServer_TcpClient::txLanDataSlot()
{

}
void WebServer_TcpClient::readTcpData()
{
    QByteArray data = _aqcSocket->readAll();
    qDebug() << data;
    for(int i = 0; i < data.size(); i++)
        Parse_Byte(data[i]);
}


void WebServer_TcpClient::transMsgLAN(u_int8_t Msg_ID,  uint msg_len,u_int8_t out_msg[])
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

    for(int j = 0; j < msg_len; j++)
    {
        txData.append(out_msg[j]);
        ck_sum += out_msg[j];
    }

    txData.append(ck_sum);
    _aqcSocket->write(txData);
}

void WebServer_TcpClient::transmitMessage_LAN(unsigned char Msg_ID,  int  msg_len,unsigned char out_msg[])
{
    u_int8_t ck_sum = 0;
    PayloadLAN[0] = LAN_Tr_Sync1;
    PayloadLAN[1] = LAN_Tr_Sync2;
    PayloadLAN[2] = Msg_ID;
    DATA32to8.u32 = msg_len;
    memcpy(&PayloadLAN[3],DATA32to8.u08,4);
    for(char cc =2; cc<7;cc++)
    ck_sum += PayloadLAN[cc];

    for(int j = 0; j < msg_len; j++)
    {
        PayloadLAN[j+7]=out_msg[j];
        ck_sum+= PayloadLAN[j+7];
    }

    PayloadLAN[msg_len+7] = ck_sum;
    send(connfd, PayloadLAN, (msg_len+8), 0); //must be overWrited
}

void WebServer_TcpClient::Parse_Byte(u_int8_t DATA)
{
    byteCnt++;
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
            CheckSumm += DATA;
            break;

        case GOT_LEN:
            msg[msgIndex] = DATA;
            CheckSumm +=DATA;
            msgIndex++;

            if (msgIndex >= msgLen)
            {
                msgStatus = GOT_PAYLOAD;
            }
          break;

        case GOT_PAYLOAD:
            if ( DATA != CheckSumm)
            {
                goto error;
                //QDebug() << "Error in Lan Conn";
            }
            Parse_Message();
            goto restart;
    }
    return;
    error: errCnt++;
    restart: msgStatus = UINIT;
    return;
}

void WebServer_TcpClient::Parse_Message()
{
    switch (msgID)
    {
        case LastAQC_Data:
            memcpy(msgConf, msg, sizeof(msg));
        break;
        case primartAQC_Data2Server:
            memcpy(msgConf, msg, sizeof(msg));
        break;
        case MemoryStatus_WDG:
            memcpy(msgConf, msg, sizeof(msg));
        break;
        case AQC_ServerConnectionStatus:
            memcpy(msgConf, msg, sizeof(msg));
        break;
    }
}
