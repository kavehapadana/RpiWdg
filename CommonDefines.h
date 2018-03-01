#include <stdint.h>
#include <QThread>
#include <QDebug>
//#define SPI_Raspberri
#include <QDataStream>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QDateTime>

#define setMask(data, index) 		(data |= 1 << index)
#define unsetMask(data, index)      (data &= ~(1 << index))
#define ToggleMask(data, index)     (data ^= 1 << index)
#define getMask(data, index) 		((data >> index) & 1)

typedef enum{
    primaryAQC_Data2Server = 6,
    AQC_KeepAliveID, // in msg can get status of ADC or Wdg
    primaryResponseFromServer,
    RPI_CentServer_Cnn_Aware2AQC_MsgType
}AQC_Ctrl_CentServer_Message_Types;


typedef enum{
    MemoryStatus_WDG = 48,
    Restart_AQC_Program,
    WdgAqc_KeepAliveID,
}WdgAqc_Ctrl_CentServer_Message_Types;


typedef enum
{
    mainPowerSPI_CentServer = 36,
    turnOffRelays,
    turnOnRelays
}CentServer_Ctrl_SPI_Message_IDs;

typedef enum
{
    AQC_TCP_Cnn_Error = 0xE0,
    AQC_KeepAliveError,
    AQC_CentServerCnnError,
    WdgAqc_KeepAliveError,
    WdgAqc_TCP_Cnn_Error,
    rpi_ModemError
}Alarms_Ctrl_CentServer_Message_IDs;

typedef enum
{
    primatyAQC_Data_ID = 0xA0,
    alarms_IDs
}Ctrl_CentServer_Message_IDs;

typedef enum
{
    CentServer_Disconnect = 1,
    CentServer_Connect,
    AQC_Disconnect,
    AQC_Connect,
    WdgAqc_Disconnect,
    WdgAqc_Connect,
    Internet_Disconnect,
    Internet_Connect,
    Rpi_USB_Modem_Disconnect,
    Rpi_USB_Modem_Connect,
    AQC_KeepAlive_NotRcv,
    AQC_KeepAlive_Rcv,
    WdgAqc_KeepAlive_NotRcv,
    WdgAqc_KeepAlive_Rcv,
    AQC_MemoryOverLoaded
}DB_AlarmsIDs;

class EventedVariable : public QObject
{
    Q_OBJECT
    Q_PROPERTY( int value READ getValue WRITE setValue NOTIFY valueChanged )
public:
    explicit EventedVariable( QObject* parent = nullptr ) :
        QObject{ parent }, i_{ 0 } {}
    virtual ~EventedVariable() {}

    int getValue() const { return i_;}
    QString Desc;
public slots:
    void setValue( int value )
    {
        if ( value != i_ ) {
            i_ = value;
            emit valueChanged( i_, Desc);
        }
    }
signals:
    void valueChanged( int value , QString Desc);
private:
    int i_;
};
//RestartPC Checked
