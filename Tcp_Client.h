#ifndef Tcp_Client_H
#define Tcp_Client_H
#include <QTcpSocket>

#include <QObject>
#include <sys/socket.h>
#include <stdint.h>
#include <QtCore>

#define	LAN_Rx_Sync1            0x1A
#define	LAN_Rx_Sync2            0x2B
#define	CentServer_Rx_Sync1     0xFA
#define	CentServer_Rx_Sync2     0x75

#define	LAN_Tr_Sync1            0xAE
#define	LAN_Tr_Sync2            0xEA


class Tcp_Client : public QObject
{
    Q_OBJECT
public:
    explicit Tcp_Client(QObject *parent = 0);
    bool clientConnected;
    QMutex _mclientConnected;
    QTcpSocket *_tcpSocket;
    QString IP_Address;
    quint16 IP_Port;

    QByteArray  txClientMsg;
    QByteArray  rxClientMsg;
    u_int8_t    txClient_ID;
    u_int8_t    rxClient_ID;

    bool tcpCnnStatus_b;

private:

protected:

    typedef enum{
        UINIT = 0,
        GOT_SYNC1,
        GOT_SYNC2,
        GOT_ID,
        GOT_LEN,
        GOT_PAYLOAD,
        GOT_DeviceID
    } Message_Status_Type;
    Message_Status_Type msgStatus;

    typedef union{
        uint8_t u08[2];
        uint16_t u16;
    }   _uni16_8;

    typedef union{
        uint8_t u08[4];
        int u32;
    }   _uni32_8;



    QByteArray msg;
    u_int8_t byteCnt;
    u_int8_t CheckSumm,msgID,msgDeviceID,msgLen,errCnt,msgIndex;

    _uni16_8    DATA16to8;
    _uni32_8    DATA32to8;
    QByteArray msgOptFull;

signals:
    void finishObject();
    void txClientDataSignal();
public slots:
    void readTcpData();
    void transMsgLAN(u_int8_t Msg_ID,  uint msg_len,QByteArray out_msg);
    void rxClientDataSlot();
    void rxClientDataOptSlot();
    void broadCastRcvInit();
    void connect2Server();
    virtual void initClient(QString _add, quint16 _port);
    virtual void Parse_Byte(u_int8_t DATA);
    virtual void initClient();
    void transMsgOpt(QByteArray msg);

private slots:

};

#endif // Tcp_Client_H
