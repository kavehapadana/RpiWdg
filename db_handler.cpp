#include "db_handler.h"
#include <QDebug>

DB_Handler::DB_Handler(QObject *parent) :
    QObject(parent)
{

}
QString DB_Handler::path = "";
QSqlDatabase DB_Handler::m_db;
QMutex DB_Handler::mutex;
int DB_Handler::worked;
int DB_Handler::unworked;

void DB_Handler::DB_Init()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(DB_Handler::path);
}
QSqlQuery DB_Handler::executeQuery(QSqlQuery _sqlQuery)
{
    QMutexLocker lock(&mutex);
    worked++;
    if(m_db.isOpen())
    {
        if(!_sqlQuery.exec())
        {
            unworked++;
            qDebug() << m_db.lastError().text() << "work = " << worked << "--- unworked = " << unworked;
        }
    }
    return _sqlQuery;
}

bool DB_Handler::OpenDB()
{
    bool rt= false;
    m_db.open();
    if(m_db.isOpen())
    {
            rt = true;
        QSqlQuery query;
        QString qryStr = "BEGIN TRANSACTION";

//        if(!query.exec(qryStr))
//            qDebug() << m_db.lastError().text();
    }

    return rt;
}
bool DB_Handler::CloseDB()
{
    QSqlQuery query;
    QString qryStr = "END TRANSACTION";

//    if(!query.exec(qryStr))
//        qDebug() << m_db.lastError().text();

    bool rt= false;
    if(m_db.isOpen())
        m_db.close();

    if(!m_db.isOpen())
        rt = true;
    return rt;
}

QSqlQuery DB_Handler::execStrQuery(QSqlQuery query)
{
    QMutexLocker lock(&mutex);

    QString str = query.lastQuery();
    QMapIterator<QString, QVariant> it(query.boundValues());

    it.toBack();

    while (it.hasPrevious())
    {
        it.previous();
        str.replace(it.key(),it.value().toString());
    }

    if(m_db.isOpen())
    {
        QSqlQuery query_;
        if(!query_.exec(str))
            qDebug() << m_db.lastError().text();
        return query_;
    }
    else
    {
        QSqlQuery query_;
        return query_;
    }
}

QSqlQuery DB_Handler::getDataDB(QString TableName, QString Fields, QString WhereStr, QString OrderStr, QString dbPath)
{
    QString qryStr = "SELECT " + Fields +" FROM " + TableName;

    if(WhereStr != "")
        qryStr += " WHERE " + WhereStr;

    if(OrderStr != "")
        qryStr += " ORDER BY " + OrderStr;

    DB_Handler::path = dbPath;
    DB_Handler::DB_Init();
    DB_Handler::OpenDB();
    if(m_db.isOpen())
    {
        QSqlQuery query;
        if(!query.exec(qryStr))
            qDebug() << m_db.lastError().text();
        DB_Handler::CloseDB();
        return query;
    }
    else
    {
        QSqlQuery query;
        return query;
    }
}

QSqlQuery DB_Handler::insertAlarm(QString _dt, QString _alarmType)
{
    QSqlQuery query;
    query.prepare(
    "INSERT INTO Alarms"
    "([DateTime_], [Alarm_id_])"
    "VALUES (:datetime , :alarm_id_)"
                );
    _dt = "'" + _dt + "'";
    _alarmType = "'" + _alarmType + "'";

    query.bindValue(":datetime",_dt);
    query.bindValue(":alarm_id_",_alarmType);

    return DB_Handler::execStrQuery(query);
}

QSqlQuery DB_Handler::insertSMS_Event(QString _dt, QString _phone, QString _content, QString _Box)
{
    QSqlQuery query;
    query.prepare(
    "INSERT INTO SMS_Events"
    "([DateTime_], [PhoneNo_], [Content_], [BOX_])"
    "VALUES (:datetime , :phoneNo_ , :content_ , :box_)"
                );
    _dt = "'" + _dt + "'";
    _phone = "'" + _phone + "'";
    _content = "'" + _content + "'";
    _Box = "'" + _Box + "'";

    query.bindValue(":datetime",_dt);
    query.bindValue(":phoneNo_",_phone);
    query.bindValue(":content_",_content);
    query.bindValue(":box_",_Box);
    return DB_Handler::execStrQuery(query);
}

QSqlQuery DB_Handler::getDataDB(QString TableName, QString Fields, QString WhereStr, QString OrderStr)
{
    QSqlQuery query;
    QString qryStr = "SELECT " + Fields +" FROM " + TableName;

    if(WhereStr != "")
        qryStr += " WHERE " + WhereStr;

    if(OrderStr != "")
        qryStr += " ORDER BY " + OrderStr;

    query.prepare(qryStr);
    return executeQuery(query);
}
