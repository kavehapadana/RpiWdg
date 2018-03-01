#include "controller.h"

using namespace std;

Controller::Controller(QObject *parent) :
    QObject(parent)
{
    configSettings();
    timerConnectionsCheck = new QTimer();
    // setup signal and slot
    connect(timerConnectionsCheck, SIGNAL(timeout()),
          this, SLOT(timerConnectionsCheckSlot()));
    timerConnectionsCheck->setInterval(Timer_Interval_ConnectionsCheck.toInt()); // Every 10 Sec the Connection Checked
    CntTimerConnectionTest = 0;


    timerTCP_KeepAliveCheck = new QTimer();
    // setup signal and slot
    connect(timerTCP_KeepAliveCheck, SIGNAL(timeout()),
          this, SLOT(timerTCP_KeepAliveCheckSlot()));
    timerTCP_KeepAliveCheck->setInterval(Timer_Interval_TCP_KeepAliveCheck.toInt()); // Every 5 min the Connection Checked

    timerTCP_Test = new QTimer();
    // setup signal and slot
    connect(timerTCP_Test, SIGNAL(timeout()),
          this, SLOT(timerTCP_TestSlot()));
    timerTCP_Test->setInterval(10000); //

    TgListenerInit();


}
Controller::~Controller() {
    //DB_Handler::CloseDB();
    emit FinishAllThreads();

    //wait 200 milisecound
    {
        QTime dieTime = QTime::currentTime().addMSecs(200);
        while( QTime::currentTime() < dieTime )
        {
            QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
        }
    }
    QCoreApplication::quit();
}


void Controller::InitController()
{

    //DB_Handler::path = qApp->applicationDirPath() + "/AppOutput";
    //DB_Handler::DB_Init();
    //if(!DB_Handler::OpenDB())
        //qDebug() << "DataBase Not Opened";

    InitSMS_Cnn();

    initAqcLanConn();
    initCentServerCnn();
    initSPI_Cnn();
    initWdgAqcConn();
    timerConnectionsCheck->start();
    //timerTCP_Test->start();


    timerTCP_KeepAliveCheck->start();
}

void Controller::configSettings()
{
    QString G_SettingsFile = QDir::currentPath() + "/General_Settings.ini";
    QSettings G_Settings(G_SettingsFile, QSettings::NativeFormat);
    //Settings.setValue("text", "sText");
    //Settings.setValue("IP", "192.168.2.18");

    AQC_Program_Port    = G_Settings.value("AQC_Program_Port","6912").toString();
    Wdg_Program_Port    = G_Settings.value("Wdg_Program_port","6969").toString();
    CentServerAlarmPort = G_Settings.value("CentServerAlarmPort","3000").toString();
    CentServerDataPort  = G_Settings.value("CentServerDataPort","3000").toString();
    CentServerHost      = G_Settings.value("CentServerHost","airnow.tehran.ir").toString();
    CentServerIP        = G_Settings.value("CentServerIP","31.24.238.89").toString();
    SuccessSMS_No       = G_Settings.value("SuccessSMS_No","09109213644").toString();
    RegisteredNo1       = G_Settings.value("RegisteredNo1","09109213644").toString();
    RegisteredNo2       = G_Settings.value("RegisteredNo2","09122007950").toString();;
    RegisteredNo3       = G_Settings.value("RegisteredNo3","09109213644").toString();
    RegisteredNo4       = G_Settings.value("RegisteredNo4","09109213644").toString();
    RegisteredNo5       = G_Settings.value("RegisteredNo5","09109213644").toString();
    ContentPSW          = G_Settings.value("ContentPassword","xyz").toString();
    Timer_Interval_ConnectionsCheck     = G_Settings.value("Timer_Interval_ConnectionsCheck","4000").toString();
    Timer_Interval_SMS_UnreadCheck      = G_Settings.value("Timer_Interval_SMS_UnreadCheck","10000").toString();
    Timer_Interval_TCP_KeepAliveCheck   = G_Settings.value("Timer_Interval_TCP_KeepAliveCheck","30000").toString();
    udpBroadCastPort    = G_Settings.value("udpBroadCastPort","45454").toString();

    QString     P_SettingsFile = QDir::currentPath() + "/Private_Settings.ini";
    QSettings   P_Settings(P_SettingsFile, QSettings::NativeFormat);

    Station_Name        = P_Settings.value("Station_Name","--").toString();
    AQC_Program_IP      = P_Settings.value("AQC_Program_IP","10.9.22.50").toString();
    Wdg_Program_IP      = P_Settings.value("Wdg_Program_IP","10.9.22.50").toString();

    QString m_pidSettingsFile = QDir::currentPath() + "/processID.ini";
    QSettings pidSettings(m_pidSettingsFile, QSettings::NativeFormat);

    int pid  = QCoreApplication::applicationPid();
    QString pid_Str = QString::number(pid);
    pidSettings.setValue("pid", pid_Str);
}

void Controller::initAqcLanConn()
{  
    AqcConnThread = new QThread;
    AQC_Client_Obj = new Tcp_Client();
    //AQC_Client_Obj->initClient("192.168.1.28",6912);
    AQC_Client_Obj->initClient(AQC_Program_IP,AQC_Program_Port.toInt());
    QThread::msleep(100);
    AQC_Client_Obj->moveToThread(AqcConnThread);
    connect(AqcConnThread, SIGNAL(started()), AQC_Client_Obj, SLOT(connect2Server()));
    connect(AQC_Client_Obj, SIGNAL(finishObject()), AqcConnThread, SLOT(quit()));
    connect(AQC_Client_Obj, SIGNAL(txClientDataSignal()), this, SLOT(AQC2CtrlDataSlot()));
    connect(this, SIGNAL(Ctrl2AQC_DataSignal()), AQC_Client_Obj, SLOT(rxClientDataSlot()));
    connect(this, SIGNAL(Ctrl2AQC_DataOptSignal()), AQC_Client_Obj, SLOT(rxClientDataSlot()));
    connect(AQC_Client_Obj, SIGNAL(finishObject()), AQC_Client_Obj, SLOT(deleteLater()));
    connect(AqcConnThread, SIGNAL(finished()), AqcConnThread, SLOT(deleteLater()));

    connect(this, SIGNAL(FinishAllThreads()), AQC_Client_Obj, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), AqcConnThread, SLOT(deleteLater()));

    AqcConnThread->start();
    QThread::msleep(100);
    AQC_Client_ObjExist_Bool = true;
}

void Controller::InitSMS_Cnn()
{
    smsClass_Obj = new SmsController();
    smsClass_Obj->Timer_Interval_SmsUnreadCheck = Timer_Interval_SMS_UnreadCheck.toInt();
    {//validNo + Password
        smsClass_Obj->ContentPassword = ContentPSW;
        smsClass_Obj->validNo1 = RegisteredNo1;
        smsClass_Obj->validNo2 = RegisteredNo2;
        smsClass_Obj->validNo3 = RegisteredNo3;
        smsClass_Obj->validNo4 = RegisteredNo4;
        smsClass_Obj->validNo5 = RegisteredNo5;
    }
    SMSConnThread = new QThread;
    QThread::msleep(100);
    smsClass_Obj->moveToThread(SMSConnThread);
    connect(SMSConnThread, SIGNAL(started()), smsClass_Obj, SLOT(init()));
    connect(smsClass_Obj, SIGNAL(finishObject()), SMSConnThread, SLOT(quit()));
    connect(smsClass_Obj, SIGNAL(cmdResetRelays_Signal()), this, SLOT(SmsCmdResetRelays_Slot()));
    connect(smsClass_Obj, SIGNAL(ErrorCurlexecSignal()), this, SLOT(ErrorCurlexecSlot()));
    connect(this, SIGNAL(Ctrl2SMS_DataSignal()), smsClass_Obj, SLOT(Ctrl2SMS_DataSlot()));
    connect(smsClass_Obj, SIGNAL(finishObject()), smsClass_Obj, SLOT(deleteLater()));
    connect(SMSConnThread, SIGNAL(finished()), SMSConnThread, SLOT(deleteLater()));  
    connect(this, SIGNAL(FinishAllThreads()), smsClass_Obj, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), SMSConnThread, SLOT(deleteLater()));
    SMSConnThread->start();
}

void Controller::initWdgAqcConn()
{
    WdgAqcConnThread = new QThread;
    WdgAqc_Client_Obj = new Tcp_Client();
//    WdgAqc_Client_Obj->initClient("192.168.1.28",6969);
    WdgAqc_Client_Obj->initClient(Wdg_Program_IP,Wdg_Program_Port.toInt());

    QThread::msleep(100);
    WdgAqc_Client_Obj->moveToThread(WdgAqcConnThread);
    connect(WdgAqcConnThread, SIGNAL(started()), WdgAqc_Client_Obj, SLOT(connect2Server()));
    connect(WdgAqc_Client_Obj, SIGNAL(finishObject()), WdgAqcConnThread, SLOT(quit()));
    connect(WdgAqc_Client_Obj, SIGNAL(txClientDataSignal()), this, SLOT(WdgAqc2CtrlDataSlot()));
    connect(this, SIGNAL(Ctrl2WdgAqc_DataSignal()), WdgAqc_Client_Obj, SLOT(rxClientDataSlot()));
    connect(WdgAqc_Client_Obj, SIGNAL(finishObject()), WdgAqc_Client_Obj, SLOT(deleteLater()));
    connect(WdgAqcConnThread, SIGNAL(finished()), WdgAqcConnThread, SLOT(deleteLater()));    
    connect(this, SIGNAL(FinishAllThreads()), WdgAqc_Client_Obj, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), WdgAqcConnThread, SLOT(deleteLater()));

    WdgAqcConnThread->start();
    QThread::msleep(100);
    WdgAqc_Client_ObjExist_Bool = true;
}

void Controller::initCentServerCnn()
{
    RPI_CentServer_Cnn_Aware2AQC = false;
    CentServerConnThread = new QThread;
    CentServer_Client_Obj = new TC_CentServer();
    QHostInfo info = QHostInfo::fromName(CentServerHost);
    QString Address;
    if (!info.addresses().isEmpty()) {

        QHostAddress address = info.addresses().first();
        Address= address.toString();
        // use the first IP address
        int A = 0; A++;
    }
    else
        Address = CentServerIP;
//  CentServer_Client_Obj->initClient("10.9.22.198",3000);
    CentServer_Client_Obj->initClient(Address,CentServerDataPort.toInt());

    QThread::msleep(100);
    CentServer_Client_Obj->moveToThread(CentServerConnThread);
    connect(CentServerConnThread, SIGNAL(started()), CentServer_Client_Obj, SLOT(connect2Server()));
    connect(CentServer_Client_Obj, SIGNAL(finishObject()), CentServerConnThread, SLOT(quit()));
    connect(CentServer_Client_Obj, SIGNAL(txClientDataSignal()), this, SLOT(CentServer2CtrlDataSlot()));
    connect(this, SIGNAL(Ctrl2CentServer_DataSignal()), CentServer_Client_Obj, SLOT(rxClientDataOptSlot()));
    connect(CentServer_Client_Obj, SIGNAL(finishObject()), CentServer_Client_Obj, SLOT(deleteLater()));
    connect(CentServerConnThread, SIGNAL(finished()), CentServerConnThread, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), CentServer_Client_Obj, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), CentServerConnThread, SLOT(deleteLater()));
    CentServerConnThread->start();
    QThread::msleep(100);
    CentServer_Client_ObjExist_Bool = true;
}

void Controller::initSPI_Cnn()
{
    SPIConnThread = new QThread;
    SPI_Cnn_Obj = new SPI_Cnn();
    SPI_Cnn_Obj->SPI_RPI_Init();
    QThread::msleep(100);
    SPI_Cnn_Obj->moveToThread(SPIConnThread);
    connect(SPIConnThread, SIGNAL(started()), SPI_Cnn_Obj, SLOT(SPI_RPI_Init()));
    connect(SPI_Cnn_Obj, SIGNAL(finishObject()), SPIConnThread, SLOT(quit()));
    connect(SPI_Cnn_Obj, SIGNAL(txSPI_DataSignal()), this, SLOT(SPI2CtrlDataSlot()));
    connect(this, SIGNAL(Ctrl2SPI_DataSignal()), SPI_Cnn_Obj, SLOT(rxSPI_DataSlot()));
    connect(SPI_Cnn_Obj, SIGNAL(finishObject()), SPI_Cnn_Obj, SLOT(deleteLater()));
    connect(SPIConnThread, SIGNAL(finished()), SPIConnThread, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), SPI_Cnn_Obj, SLOT(deleteLater()));
    connect(this, SIGNAL(FinishAllThreads()), SPIConnThread, SLOT(deleteLater()));
    SPIConnThread->start();
    QThread::msleep(500);
}

void Controller::WdgAqc2CtrlDataSlot()
{
    this->rxCtrlID_WdgAqc = WdgAqc_Client_Obj->txClient_ID;
    this->rxCtrlMsgWdgAqc = WdgAqc_Client_Obj->txClientMsg;
    WdgAqc_KeepAlive_Rcv->setValue(1);
    switch(rxCtrlID_WdgAqc)
    {
        case MemoryStatus_WDG:
            {
                AQC_MemoryOverLoad->setValue(1);
                MemoryStatus_WDG_InCtrl = rxCtrlMsgWdgAqc;
                qDebug() << "MemoryStatus_WDG";
                QString dtStr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
                //DB_Handler::insertAlarm(dtStr,"AQC_MemoryOverLoaded");
                if(false) // Check if Memory is OverLoaded Send to Server
                {
                    CentServer_Client_Obj->txClientMsg = rxCtrlMsgWdgAqc;
                    CentServer_Client_Obj->txClient_ID = MemoryStatus_WDG;
                    emit Ctrl2CentServer_DataSignal();
                }
            }
            break;

        case WdgAqc_KeepAliveID:
            WdgAqc_Client_KeepAliveCnt = 0;
            //qDebug() << "KeepAliveID_WdgAqc";
            break;
        default:
            qDebug() << "Non of ID is Acceptable in WdgAqc2CtrlDataSlot";
            break;
    }
}

void Controller::AQC2CtrlDataSlot()
{
    this->rxCtrlID_AQC = AQC_Client_Obj->txClient_ID;
    this->rxCtrlMsgAQC = AQC_Client_Obj->txClientMsg;
    AQC_MemoryOverLoad->setValue(0);
    AQC_KeepAlive_Rcv->setValue(1);

    switch(rxCtrlID_AQC)
    {
        case primaryAQC_Data2Server:
            primaryAQC_Data2ServerInCtrl = rxCtrlMsgAQC;
            CentServer_Client_Obj->txClientMsg = primaryAQC_Data2ServerInCtrl;
            CentServer_Client_Obj->txClient_ID = primaryAQC_Data2Server;
            emit Ctrl2CentServer_DataSignal();
            qDebug() << "primaryAQC_Data2Server";
            break;

        case AQC_KeepAliveID:
            AQC_Client_KeepAliveCnt = 0;
            //QDateTime LastTimeAQC_Data; // time of Last in keepAlive Handler
            //bool AQC_ServerConnectionStatus = (bool)msg[0];
            CentServer_Client_Obj->txClientMsg = AQC_ServerConnectionStatusInCtrl;
            CentServer_Client_Obj->txClient_ID = AQC_KeepAliveID;
            AQC_ServerConnectionStatus = (bool) rxCtrlMsgAQC[0];
            rxCtrlMsgAQC.remove(0,1);
            LastTimeAQC_Str = QString::fromStdString(rxCtrlMsgAQC.data());
            LastTimeAQC_Data.fromString(LastTimeAQC_Str); //invalide ;((
            //qDebug() << "KeepAliveID_AQC";

            break;

        case 1:
        break;
        default:
            qDebug() << "Non of ID is Acceptable in AQC2CtrlDataSlot";
            break;
    }
}

// A command for check power of rpi ???
void Controller::CentServer2CtrlDataSlot()
{
    this->rxCtrlID_CentServer = CentServer_Client_Obj->txClient_ID;
    this->rxCtrlMsgCentServer = CentServer_Client_Obj->txClientMsg;

    AQC_Client_Obj->txClientMsg = rxCtrlMsgCentServer;
    AQC_Client_Obj->txClient_ID = rxCtrlID_CentServer;
    Ctrl2AQC_DataOptSignal();
    qDebug() << "fnc CentServer2CtrlDataSlot";
    // All of this items launched with SMS
    //    switch(rxCtrlID_CentServer)
//    {
//        case Restart_AQC_Program:
//            WdgAqc_Client_Obj->txClientMsg = rxCtrlMsgCentServer;
//            WdgAqc_Client_Obj->txClient_ID = Restart_AQC_Program;
//            emit Ctrl2WdgAqc_DataSignal();
//            break;

//        case turnOffRelays:
//            SPI_Cnn_Obj->txSPIMsg = rxCtrlMsgCentServer;
//            SPI_Cnn_Obj->txSPI_ID = turnOffRelays;
//            emit Ctrl2SPI_DataSignal();
//            break;

//        case turnOnRelays:
//            SPI_Cnn_Obj->txSPIMsg = rxCtrlMsgCentServer;
//            SPI_Cnn_Obj->txSPI_ID = turnOnRelays;
//            emit Ctrl2SPI_DataSignal();
//            break;

//        default:
//            qDebug() << "Non of ID is Acceptable in CentServer2CtrlDataSlot";
//            break;
//    }
}

void Controller::SPI2CtrlDataSlot()
{
    this->rxCtrlID_SPI = SPI_Cnn_Obj->txSPI_ID;
    this->rxCtrlMsgSPI = SPI_Cnn_Obj->txSPIMsg;
    switch(rxCtrlID_SPI)
    {
        case mainPowerSPI_CentServer:
        CentServer_Client_Obj->txClientMsg = rxCtrlMsgSPI;
        CentServer_Client_Obj->txClient_ID = mainPowerSPI_CentServer;
        emit Ctrl2CentServer_DataSignal();
        qDebug() << "mainPowerSPI_CentServer";
            break;

        default:
            qDebug() << "Non of ID is Acceptable in SPI2CtrlDataSlot";
            break;
    }
}

void Controller::SmsCmdResetRelays_Slot()
{
    u_int8_t resetRelaysBits = smsClass_Obj->bitRelaysCtrl;
    SPI_Cnn_Obj->bitRelaysReset = resetRelaysBits;
    //save SMS To DB
    {
        QString _dt_DB = smsClass_Obj->dt_DB;
        QString _Phone_DB = smsClass_Obj->Phone_DB;
        QString _Content_DB = smsClass_Obj->Content_DB;
        QString _Box_DB = smsClass_Obj->Box_DB;

        //DB_Handler::insertSMS_Event(_dt_DB, _Phone_DB, _Content_DB, _Box_DB);
    }
    emit Ctrl2SPI_DataSignal();

    //Write Command to MCU
    //QThread::msleep(10);
    smsClass_Obj->CtrlSMS_Cont_Num_Qstr[0] = "Successfull موفقیت آمیز بود";
    smsClass_Obj->CtrlSMS_Cont_Num_Qstr[1] = SuccessSMS_No;
    emit Ctrl2SMS_DataSignal();
}

void Controller::ErrorCurlexecSlot()
{
    QByteArray _t;
    _t.append(0x01);
    AQC_Client_Obj->txClientMsg = _t;
    AQC_Client_Obj->txClient_ID = rpi_ModemError;
    QString dtStr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
    //DB_Handler::insertAlarm(dtStr,"Rpi_USB_Modem_Disconnect");
    USB_Modem_Connection->setValue(0);
    emit Ctrl2AQC_DataSignal();
}

int cntTest = 6;
void Controller::timerTCP_TestSlot()
{
//    rxCtrlMsgAQC.append(cntTest);
//    cntTest++;
//    switch(cntTest)
//    {
//        case LastAQC_Data:
//            LastAQC_DataInCtrl = rxCtrlMsgAQC;
//            break;

//        case primaryAQC_Data2Server:
//            primaryAQC_Data2ServerInCtrl = rxCtrlMsgAQC;
//            AQC_Client_Obj->txClientMsg = primaryAQC_Data2ServerInCtrl;
//            AQC_Client_Obj->txClient_ID = primaryAQC_Data2Server;
//            emit Ctrl2AQC_DataSignal();
//            break;

//        case MemoryStatus_WDG:
//            MemoryStatus_WDG_InCtrl = rxCtrlMsgAQC;
//            AQC_Client_Obj->txClientMsg = MemoryStatus_WDG_InCtrl;
//            AQC_Client_Obj->txClient_ID = MemoryStatus_WDG;
//            emit Ctrl2AQC_DataSignal();
//            break;

//        case AQC_ServerConnectionStatus:
//            AQC_ServerConnectionStatusInCtrl = rxCtrlMsgAQC;
//            AQC_Client_Obj->txClientMsg = AQC_ServerConnectionStatusInCtrl;
//            AQC_Client_Obj->txClient_ID = AQC_ServerConnectionStatus;
//            emit Ctrl2AQC_DataSignal();
//            break;
//    }
}

void Controller::timerConnectionsCheckSlot()
{
    try
    {
        //QMutexLocker m1(&AQC_Client_Obj->_mclientConnected);
        if(AQC_Client_Obj->_tcpSocket->state() == QTcpSocket::ConnectedState)
        {
            AQC_Connection->setValue(1);
            //qDebug() << "Connected State AQC";
            QByteArray out_msg;
            AQC_Client_Obj->txClient_ID = RPI_CentServer_Cnn_Aware2AQC_MsgType;
            out_msg.append(RPI_CentServer_Cnn_Aware2AQC);
            AQC_Client_Obj->txClientMsg = out_msg;
            emit Ctrl2AQC_DataSignal();
           // AQC_Client_Obj->transMsgLAN(0X6,  1,out_msg);
        }
        else
        {
            AQC_Connection->setValue(0);
            QString dtStr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
            //DB_Handler::insertAlarm(dtStr,"AQC_Disconnect");
            AQC_Client_ObjExist_Bool = false;
            Ctrl2CentServer_Alarms_Fnc(AQC_TCP_Cnn_Error);
            qDebug() << "DisConnected State AQC";
            emit AQC_Client_Obj->finishObject();
            QThread::msleep(100);
            initAqcLanConn();
        }

        if(CentServer_Client_Obj->_tcpSocket->state() == QTcpSocket::ConnectedState)
        {
            CentServer_Connection->setValue(1);
            //qDebug() << "Connected State CentServer";
            QByteArray out_msg;
            out_msg.append(16);
           // CentServer_Client_Obj->transMsgLAN(0X8,  1,out_msg);
            RPI_CentServer_Cnn_Aware2AQC = true;
            if(!(CntTimerConnectionTest++%5))
                QFuture<void> future = QtConcurrent::run(this,&Controller::CheckCentServerSlot);

        }
        else
        {
            CentServer_Connection->setValue(0);

            CentServer_Client_ObjExist_Bool = false;
            RPI_CentServer_Cnn_Aware2AQC = false;
            qDebug() << "DisConnected State CentServer";
            emit CentServer_Client_Obj->finishObject();
            QThread::msleep(100);
            initCentServerCnn();
        }

        if(WdgAqc_Client_Obj->_tcpSocket->state() == QTcpSocket::ConnectedState)
        {
            WdgAqc_Connection->setValue(1);
            //qDebug() << "Connected State WdgAqc";
            QByteArray out_msg;
            out_msg.append(16);
           // WdgAqc_Client_Obj->transMsgLAN(0X8,  1,out_msg);
        }
        else
        {
            WdgAqc_Connection->setValue(0);
            QString dtStr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
            //DB_Handler::insertAlarm(dtStr,"WdgAqc_Disconnect");

            WdgAqc_Client_ObjExist_Bool = false;
            Ctrl2CentServer_Alarms_Fnc(WdgAqc_TCP_Cnn_Error);
            qDebug() << "DisConnected State WdgAqc";
            emit WdgAqc_Client_Obj->finishObject();
            QThread::msleep(100);
            initWdgAqcConn();
        }
        //qDebug() << "emit spi signal\n";
        emit Ctrl2SPI_DataSignal();

        QFuture<void> future = QtConcurrent::run(this,&Controller::CheckNetSlot);

        //qDebug() << "\n The Signial Strenght is ->" << smsClass_Obj->ReadSignalStrengh()<< "/5" << "\n";
    }
    catch(std::exception &e)
    {
        //qDebug() << e.what();
    }
    catch (const std::string& ex) {
        //qDebug() << "ex";
    }
    catch(...)
    {
        //qDebug() << "Exception Error";
    }
}

void Controller::timerTCP_KeepAliveCheckSlot()
{
    AQC_Client_KeepAliveCnt++;
    WdgAqc_Client_KeepAliveCnt++;
//    WdgAqc_Client_Obj->txClientMsg = rxCtrlMsgCentServer;
//    WdgAqc_Client_Obj->txClient_ID = Restart_AQC_Program;
//    emit Ctrl2WdgAqc_DataSignal();    ,

    if(AQC_Client_KeepAliveCnt == 4)
    {
        Ctrl2CentServer_Alarms_Fnc(AQC_KeepAliveError);
        QString dtStr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
        //DB_Handler::insertAlarm(dtStr,"AQC_KeepAlive_NotRcv");
        AQC_KeepAlive_Rcv->setValue(0);
        AQC_Client_KeepAliveCnt = 0;
    }
    if(WdgAqc_Client_KeepAliveCnt == 4)
    {
        Ctrl2CentServer_Alarms_Fnc(WdgAqc_KeepAliveError);
        QString dtStr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
        //DB_Handler::insertAlarm(dtStr,"WdgAqc_KeepAlive_NotRcv");
        WdgAqc_KeepAlive_Rcv->setValue(0);
        WdgAqc_Client_KeepAliveCnt = 0;
    }
}

void Controller::Ctrl2CentServer_Alarms_Fnc(int _alarm)
{
    QByteArray _msgAlarm;
    _msgAlarm.append(_alarm);
    CentServer_Client_Obj->transMsgLAN(alarms_IDs,_msgAlarm.size(),_msgAlarm);
}

void Controller::CheckNetSlot()
{
    QStringList parameters;
    #if defined(WIN32)
        parameters << "-n" << "1";
    #else
        parameters << "-c 1";
    #endif

    parameters << "google.com";

    QProcess pingProcess;
    pingProcess.start("ping",parameters);
    pingProcess.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    QString output(pingProcess.readAllStandardOutput());

    if (output.contains("192.168.8.1"))
    {
        Internet_Connection->setValue(0);
    }
    else
    {
        Internet_Connection->setValue(1);
        USB_Modem_Connection->setValue(0);
    }
}

void Controller::CheckCentServerSlot(){

    QStringList parameters;
    #if defined(WIN32)
        parameters << "-n" << "1";
    #else
        parameters << "-c 1";
    #endif

    parameters << "airnow.tehran.ir";

    QProcess pingProcess;
    pingProcess.start("ping",parameters);
    pingProcess.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    QString output(pingProcess.readAllStandardOutput());

    if (output.contains("192.168.8.1"))
    {
        CentServer_Connection->setValue(0);
        qDebug() << "DisConnected State CentServer";
        emit CentServer_Client_Obj->finishObject();
        QThread::msleep(100);
        initCentServerCnn();
    }
    else
    {
        CentServer_Connection->setValue(1);
    }
}

void Controller::TgListenerInit(){

    CentServer_Connection = new EventedVariable();
    CentServer_Connection->Desc = "CentServer_Connection";

    AQC_Connection = new EventedVariable();
    AQC_Connection->Desc = "AQC_Connection";

    WdgAqc_Connection = new EventedVariable();
    WdgAqc_Connection->Desc = "WdgAqc_Connection";

    Internet_Connection = new EventedVariable();
    Internet_Connection->Desc = "Internet_Connection";

    USB_Modem_Connection = new EventedVariable();
    USB_Modem_Connection->Desc = "USB_Modem_Connection";

    AQC_KeepAlive_Rcv = new EventedVariable();
    AQC_KeepAlive_Rcv->Desc = "AQC_KeepAlive_Rcv";

    WdgAqc_KeepAlive_Rcv = new EventedVariable();
    WdgAqc_KeepAlive_Rcv->Desc = "WdgAqc_KeepAlive_Rcv";

    AQC_MemoryOverLoad = new EventedVariable();
    AQC_MemoryOverLoad->Desc = "AQC_MemoryOverLoad";

    CentServer_Connection->setValue(0);
    AQC_Connection->setValue(0);
    WdgAqc_Connection->setValue(0);
    Internet_Connection->setValue(0);
    USB_Modem_Connection->setValue(0);
    AQC_KeepAlive_Rcv->setValue(0);
    WdgAqc_KeepAlive_Rcv->setValue(0);
    AQC_MemoryOverLoad->setValue(0);

    connect(CentServer_Connection,SIGNAL(valueChanged(int , QString)),  this,SLOT(SendTgListener(int, QString)));
    connect(AQC_Connection,SIGNAL(valueChanged(int , QString)),         this,SLOT(SendTgListener(int, QString)));
    connect(WdgAqc_Connection,SIGNAL(valueChanged(int , QString)),      this,SLOT(SendTgListener(int, QString)));
    connect(Internet_Connection,SIGNAL(valueChanged(int , QString)),    this,SLOT(SendTgListener(int, QString)));
    connect(USB_Modem_Connection,SIGNAL(valueChanged(int , QString)), this,SLOT(SendTgListener(int, QString)));
    connect(AQC_KeepAlive_Rcv,SIGNAL(valueChanged(int , QString)),      this,SLOT(SendTgListener(int, QString)));
    connect(WdgAqc_KeepAlive_Rcv,SIGNAL(valueChanged(int , QString)),   this,SLOT(SendTgListener(int, QString)));
    connect(AQC_MemoryOverLoad,SIGNAL(valueChanged(int , QString)),   this,SLOT(SendTgListener(int, QString)));

}

void Controller::SendTgListener(int _value,QString _desc){
    QString msgTg = "\n\nStation: " + Station_Name +"\n";
    if(_value == 0)
        msgTg +=  _desc + ": is Falsed"+"\n\n";
    else if(_value == 1)
        msgTg +=  _desc + ": is OK"+"\n\n";

    msgTg += CentServer_Connection->Desc + ":        " + trueFalseName(CentServer_Connection->getValue())+"\n";
    msgTg += AQC_Connection->Desc + ":                    " + trueFalseName(AQC_Connection->getValue())+"\n";
    msgTg += WdgAqc_Connection->Desc + ":             " + trueFalseName(WdgAqc_Connection->getValue())+"\n";
    msgTg += Internet_Connection->Desc + ":              " + trueFalseName(Internet_Connection->getValue())+"\n";
    msgTg += USB_Modem_Connection->Desc + ":     " + trueFalseName(USB_Modem_Connection->getValue())+"\n";
    msgTg += AQC_KeepAlive_Rcv->Desc + ":              " + trueFalseName(AQC_KeepAlive_Rcv->getValue())+"\n";
    msgTg += WdgAqc_KeepAlive_Rcv->Desc + ":       " + trueFalseName(WdgAqc_KeepAlive_Rcv->getValue())+"\n";
    msgTg += AQC_MemoryOverLoad->Desc + ":        " + trueFalseName(AQC_MemoryOverLoad->getValue())+"\n";

    QFuture<void> future = QtConcurrent::run(TelegramBot::SendToChannel, msgTg);

}

QString Controller::trueFalseName(int val){
    if(val)
        return "OK";
    else
        return "False";
}
