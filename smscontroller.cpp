#include "smscontroller.h"
#include <QtDebug>
#include <QDomDocument>
#include <CommonDefines.h>

SmsController::SmsController(QObject *parent) :
    QObject(parent)
{
    bitRelaysCtrl = 0;
    CtrlSMS_Cont_Num_Qstr.resize(2);
    timerSMS_Check = new QTimer();
    // setup signal and slot
    connect(timerSMS_Check, SIGNAL(timeout()),
          this, SLOT(timerSMS_CheckSlot()));
}

SmsController::~SmsController()
{
    finishObject();
}
void SmsController::init()
{
    timerSMS_Check->setInterval(Timer_Interval_SmsUnreadCheck);
    timerSMS_Check->start();
}

int cntTimerCheckSlot = 0;
void SmsController::timerSMS_CheckSlot()
{
    qDebug() << "time out elapsed";
    check_Unread_SMS();
    if(!(cntTimerCheckSlot++ % 20))
        ReadAllSMS(Outbox);
}

void SmsController::Ctrl2SMS_DataSlot()
{
    QVector<QString> SMS_Content_Number = CtrlSMS_Cont_Num_Qstr;
    SendSms(SMS_Content_Number.at(0),SMS_Content_Number.at(1));
}

void SmsController::ReadAllSMS(int _box)
{
    QDomNodeList DomNodeList;
    QDomNode n;
    QString myToken = getToken();
    qDebug() << "ReadAllSMS Checked";
    //this Command Read All Sms
    QString  strCommand = " curl -X POST -H \"Content-Type: application/x-www-form; charset=UTF-8\" -H \"__RequestVerificationToken: " + myToken + "\" -d \"<request><PageIndex>1</PageIndex><ReadCount>20</ReadCount><BoxType>"+QString::number(_box)+"</BoxType><SortType>0</SortType><Ascending>0</Ascending><UnreadPreferred>0</UnreadPreferred></request>\" http://192.168.8.1/api/sms/sms-list \" " ;
    QDomDocument domDoc;

    QString CurlOutput = exeCurlCmd(strCommand);
    domDoc.setContent(CurlOutput);
    DomNodeList = domDoc.elementsByTagName("Message");
    for (int i = 0; i < DomNodeList.size(); i++)
    {
        n = DomNodeList.item(i);
        QDomElement Smstat = n.firstChildElement("Smstat");
        QDomElement Index  = n.firstChildElement("Index");
        QDomElement Phone  = n.firstChildElement("Phone");
        QDomElement Content = n.firstChildElement("Content");

        if (Smstat.isNull() || Index.isNull() || Phone.isNull() || Content.isNull())
            qDebug() << "Error in isNull";
        msgSMS = Content.text();
        bool passValid = msgSMS.contains(ContentPassword,Qt::CaseInsensitive);
        bool NumberValid = false;
        QString s = Phone.text();
        if(((s == validNo1) | (s == validNo2) | (s == validNo3) | (s == validNo4) |  (s == validNo5)))
            NumberValid = true;
        if(!NumberValid)
        {
            qDebug() << " number is unvalid";
            return;
        }
        if(!passValid)
        {
            qDebug() << "Pass is unvalid";
            return;
        }

        if(msgSMS.contains("reset",Qt::CaseInsensitive) && Smstat.text() == "0" && _box == Inbox)
        {
            bitRelaysCtrl = 0;
            for(int i = 0; i < 5; i++)
            {
                QString No = QString::number(i+1);
                if(msgSMS.contains(No,Qt::CaseInsensitive))
                    setMask(bitRelaysCtrl,i);
            }

            qDebug() << bitRelaysCtrl;
            dt_DB = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
            Phone_DB = Phone.text();
            Content_DB = msgSMS;
            Box_DB = QString::number(_box);

            emit cmdResetRelays_Signal();
            SetReadMsgRcv(Index.text());
        }
        if(i >= 10)
            DeleteMsg(Index.text());
    }
}

void SmsController::SetReadMsgRcv(QString smsIndex)
{
    QString myToken = getToken();
    QString smsBody = "<request><Index>"+ smsIndex +"</Index></request>";
    QString  strCommand = " curl -X POST -H \"Content-Type: application/x-www-form; charset=UTF-8\" -H \"__RequestVerificationToken: " + myToken + "\" -d \""+ smsBody +"\" http://192.168.8.1/api/sms/set-read \" " ;
    exeCurlCmd(strCommand);
}

QString SmsController::ReadSignalStrengh()
{
    QString myToken = getToken();
    QString  strCommand = " curl -X GET  http://192.168.8.1/api/monitoring/status" ;
    QDomDocument domDoc;
    domDoc.setContent(exeCurlCmd(strCommand));
    QDomNodeList DomNodeList = domDoc.elementsByTagName("response");
    QDomNode n = DomNodeList.item(0);
    QDomElement SigIco_Msg = n.firstChildElement("SignalIcon");
    QString SigIcoNo = SigIco_Msg.text();
    return SigIcoNo;
}

void SmsController::DeleteMsg(QString smsIndex)
{
    QString myToken = getToken();
    QString smsBody = "<request><Index>"+ smsIndex +"</Index></request>";
    QString  strCommand = " curl -X POST -H \"Content-Type: application/x-www-form; charset=UTF-8\" -H \"__RequestVerificationToken: " + myToken + "\" -d \""+ smsBody +"\" http://192.168.8.1/api/sms/delete-sms \" " ;
    exeCurlCmd(strCommand);
}

void SmsController::SendSms(QString smsContent,QString smsNo)
{
    //this Command Read All Sms
    QString myToken = getToken();
    QDateTime   dt;
    QString CurrentDateTime = dt.currentDateTime().toString("yyyy-mm-dd hh:mm:ss");
    QString smsBody =  "<request><Index>-1</Index><Phones><Phone>"+smsNo+"</Phone></Phones><Sca></Sca><Content>"+ smsContent +"</Content><Length>"+smsContent.size()+"</Length><Reserved>1</Reserved><Date>"+CurrentDateTime+"</Date></request>";
    QString  strCommand = " curl -X POST -H \"Content-Type: application/x-www-form; charset=UTF-8\" -H \"__RequestVerificationToken: " + myToken + "\" -d \""+ smsBody +"\" http://192.168.8.1/api/sms/send-sms \" " ;
    exeCurlCmd(strCommand);
}

void SmsController::check_Unread_SMS()
{
    QString strCommand = "curl http://192.168.8.1/api/monitoring/check-notifications";
    QDomDocument domDoc;
    domDoc.setContent(exeCurlCmd(strCommand));
    QDomNodeList DomNodeList = domDoc.elementsByTagName("response");
    QDomNode n = DomNodeList.item(0);
    QDomElement UnR_Msg = n.firstChildElement("UnreadMessage");
    QString UnreadMessage = UnR_Msg.text();
    if(UnreadMessage != "0")
    {
        ReadAllSMS(Inbox);
    }
}

QString SmsController::getToken()
{
    QString strCommand = "curl http://192.168.8.1/api/webserver/token";

    QDomDocument domDoc;
    QString tokenOutput =exeCurlCmd(strCommand);
    domDoc.setContent(tokenOutput);
    QDomNodeList DomNodeList = domDoc.elementsByTagName("response");
    QDomNode n = DomNodeList.item(0);
    QDomElement tokenElement = n.firstChildElement("token");
    QString myToken = tokenElement.text();
    return myToken;
}

QString SmsController::exeCurlCmd(QString  cmd)
{
    QProcess process;
    QString strCommand = cmd;
    process.start(strCommand);
    process.waitForFinished(-1); // will wait forever until finished

    QString strProcessOutput = process.readAllStandardOutput();
    QString strProcessError = process.readAllStandardError();

    if(strProcessError.contains("failure"))
    {
        QString errStr = "Error In exeCurlCmd in smscontroller Class \n ";
        QString errStr2 = "Probably the Modem Does not work";
        qDebug() << errStr + errStr2;
        emit ErrorCurlexecSignal();
        return "failure";
    }

    return strProcessOutput;
}

