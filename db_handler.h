#ifndef DB_HANDLER_H
#define DB_HANDLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QMutex>

class DB_Handler : public QObject
{
    Q_OBJECT
public:
    explicit DB_Handler(QObject *parent = 0);
    static QString path;
    void static DB_Init();
    QSqlQuery static executeQuery(QSqlQuery _sqlQuery);
    QSqlQuery static getDataDB(QString TableName, QString Fields, QString WhereStr, QString OrderStr, QString dbPath);

    QSqlQuery static insertAlarm(QString _dt, QString _alarmType);
    QSqlQuery static insertSMS_Event(QString _dt, QString _phone, QString _content, QString _Box);
    QSqlQuery static getDataDB(QString TableName, QString Fields, QString WhereStr, QString OrderStr);

    bool static OpenDB();
    bool static CloseDB();

    QSqlQuery static execStrQuery(QSqlQuery query);

public slots:

signals:
    void finished();

private slots:

private:
    static QSqlDatabase m_db;
    static QMutex mutex;
    static int worked;
    static int unworked;

};
typedef enum
{
    DateTime_Enm = 1,
    Final_Deg_Enm,
    Sum_anlz_Enm,
    Delta_Anlz_Enm,
    Doppler_Enm,
    Detect_Enm,
    Ratio_Enm,
    Final_Deg_Anlz_Enm,
    Final_Rev_Enm,
    Enc_Deg_Enm,
    Enc_Rev_Enm,
    Ref_Deg_Enm,
    Ref_Rev_Enm,
    Sum_Enm,
    Delta_Enm,
    Freq_Enm
}RssiDataClassItems_t;
extern RssiDataClassItems_t  RssiDataClassItems;

typedef enum
{
    Azi_Deg_Log_Enm,
    Ratio_Log_Enm,
    Sum_Anlz_Log_Enm,
    Delta_Anlz_Log_Enm,
    Doppler_Anlz_Log_Enm,
    Time_Log_Enm,
    Freq_Log_Enm
}RiseLogClassItems_t;
extern RiseLogClassItems_t RiseLogClassItems;
#endif // DB_HANDLER_H
