#ifndef SMSCONTROLLER_H
#define SMSCONTROLLER_H

#include <QObject>
#include <qnetworkaccessmanager.h>
#include <QtNetwork>

//#define timerCheckClock 10000


class SmsController : public QObject
{
    Q_OBJECT
public:
    explicit SmsController(QObject *parent = 0);
    ~SmsController();
    u_int8_t bitRelaysCtrl;
    QVector<QString> CtrlSMS_Cont_Num_Qstr;
    void check_Unread_SMS();
    int Timer_Interval_SmsUnreadCheck;
    QString ContentPassword;
    QString validNo1;
    QString validNo2;
    QString validNo3;
    QString validNo4;
    QString validNo5;

    QString dt_DB;
    QString Phone_DB;
    QString Content_DB;
    QString Box_DB;

public slots:
    void init();
    void Ctrl2SMS_DataSlot();
    QString ReadSignalStrengh();

signals:
    void cmdResetRelays_Signal();
    void finishObject();
    void ErrorCurlexecSignal();

private:
    QTimer * timerSMS_Check;
    QString msgSMS;
    typedef enum{
        Inbox = 1,
        Outbox
    }SMS_box_Types;
private slots:
    void SetReadMsgRcv(QString smsIndex);
    void DeleteMsg(QString smsIndex);
    QString getToken();
    QString exeCurlCmd(QString  cmd);
    void SendSms(QString    smsContent,QString smsNo);
    void timerSMS_CheckSlot();
    void ReadAllSMS(int _box);

};

#endif // SMSCONTROLLER_H
