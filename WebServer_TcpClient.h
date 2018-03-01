#ifndef WebServer_TcpClient_H
#define WebServer_TcpClient_H
#include <QTcpSocket>

#include <QObject>
#include <sys/socket.h>
#include <stdint.h>
#include <QtCore>

#define	LAN_Rx_Sync1            0x1A
#define	LAN_Rx_Sync2            0x2B
#define	LAN_Tr_Sync1            0xAE
#define	LAN_Tr_Sync2            0xEA


class WebServer_TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit WebServer_TcpClient(QObject *parent = 0);
    bool clientConnected;
    QMutex _mclientConnected;
    QTcpSocket *_aqcSocket;

private:

signals:
    void finishObject();
    void LANrcvDataSignal();
public slots:
    void readTcpData();
    void Parse_Message();
    void transmitMessage_LAN(unsigned char Msg_ID,  int  msg_len,unsigned char out_msg[]);
    void transMsgLAN(u_int8_t Msg_ID,  uint msg_len,u_int8_t out_msg[]);

    void txLanDataSlot();
    void broadCastRcvInit();
    void connect2WebServer();
private slots:
    void Parse_Byte(u_int8_t DATA);

};

#endif // WebServer_TcpClient_H
