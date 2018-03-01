#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <Tcp_Client.h>
#include <spi_cnn.h>
#include <QThread>
#include <smscontroller.h>
#include "tc_centserver.h"
#include <db_handler.h>
#include <telegrambot.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <CommonDefines.h>

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = 0);
    ~Controller();
    int kk;
private:
    QDnsLookup dns;
    QDateTime LastTimeAQC_Data; // time of Last in keepAlive Handler
    QString LastTimeAQC_Str;
    bool AQC_ServerConnectionStatus;
    bool RPI_CentServer_Cnn_Aware2AQC;
    QTimer * timerConnectionsCheck;
    QTimer * timerTCP_KeepAliveCheck;

    QTimer * timerTCP_Test;
    QThread* AqcConnThread;
    QThread* WdgAqcConnThread;
    QThread* CentServerConnThread;
    QThread* SPIConnThread;
    QThread* SMSConnThread;

    Tcp_Client *AQC_Client_Obj;
    Tcp_Client *WdgAqc_Client_Obj;
    Tcp_Client *CentServer_Client_Obj;
    SPI_Cnn    *SPI_Cnn_Obj;
    SmsController *smsClass_Obj;

    bool AQC_Client_ObjExist_Bool;
    bool WdgAqc_Client_ObjExist_Bool;
    bool CentServer_Client_ObjExist_Bool;

    u_int8_t AQC_Client_KeepAliveCnt;
    u_int8_t WdgAqc_Client_KeepAliveCnt;

    QByteArray  txCtrlMsgCentServer;
    QByteArray  txCtrlMsgAQC;
    QByteArray  txCtrlMsgWdgAqc;
    QByteArray  txCtrlMsgSPI;

    u_int8_t    txCtrlID_CentServer;
    u_int8_t    txCtrlID_AQC;
    u_int8_t    txCtrlID_WdgAqc;
    u_int8_t    txCtrlID_SPI;

    QByteArray  rxCtrlMsgCentServer;
    QByteArray  rxCtrlMsgAQC;
    QByteArray  rxCtrlMsgWdgAqc;
    QByteArray  rxCtrlMsgSPI;

    u_int8_t    rxCtrlID_CentServer;
    u_int8_t    rxCtrlID_AQC;
    u_int8_t    rxCtrlID_WdgAqc;
    u_int8_t    rxCtrlID_SPI;

    /* Internal Variable for hold Datas*/
    QByteArray  LastAQC_DataInCtrl;
    QByteArray  primaryAQC_Data2ServerInCtrl;
    QByteArray  MemoryStatus_WDG_InCtrl;
    QByteArray  AQC_ServerConnectionStatusInCtrl;

    QString Station_Name;
    QString AQC_Program_IP;
    QString AQC_Program_Port;
    QString Wdg_Program_IP;
    QString Wdg_Program_Port;
    QString CentServerAlarmPort;
    QString CentServerDataPort;
    QString CentServerHost;
    QString CentServerIP;
    QString SuccessSMS_No;
    QString RegisteredNo1;
    QString RegisteredNo2;
    QString RegisteredNo3;
    QString RegisteredNo4;
    QString RegisteredNo5;
    QString ContentPSW;
    QString Timer_Interval_ConnectionsCheck;
    QString Timer_Interval_SMS_UnreadCheck;
    QString Timer_Interval_TCP_KeepAliveCheck;
    QString udpBroadCastPort;

    //TelegramAlarms
    EventedVariable *CentServer_Connection;
    EventedVariable *AQC_Connection;
    EventedVariable *WdgAqc_Connection;
    EventedVariable *Internet_Connection;
    EventedVariable *USB_Modem_Connection;
    EventedVariable *AQC_KeepAlive_Rcv;
    EventedVariable *WdgAqc_KeepAlive_Rcv;
    EventedVariable *AQC_MemoryOverLoad;

    int CntTimerConnectionTest;

signals:
    void Ctrl2AQC_DataSignal();
    void Ctrl2AQC_DataOptSignal();
    void Ctrl2WdgAqc_DataSignal();
    void Ctrl2CentServer_DataSignal();
    void Ctrl2SPI_DataSignal();
    void Ctrl2SMS_DataSignal();
    void FinishAllThreads();

public slots:
    void initAqcLanConn();
    void initWdgAqcConn();
    void initCentServerCnn();
    void initSPI_Cnn();
    void InitController();
    void InitSMS_Cnn();

private slots:
    void AQC2CtrlDataSlot();
    void WdgAqc2CtrlDataSlot();
    void CentServer2CtrlDataSlot();
    void SPI2CtrlDataSlot();
    void SmsCmdResetRelays_Slot();
    void TgListenerInit();
    void configSettings();

    void timerConnectionsCheckSlot();
    void timerTCP_KeepAliveCheckSlot();
    void timerTCP_TestSlot();
    void Ctrl2CentServer_Alarms_Fnc(int _alarm);

    void SendTgListener(int _value,QString _desc);
    QString trueFalseName(int val);
    void CheckNetSlot();
    void CheckCentServerSlot();


    void ErrorCurlexecSlot();
};

#endif // CONTROLLER_H
