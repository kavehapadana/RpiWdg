#include "telegrambot.h"
#include <CommonDefines.h>
#include <QUrl>


TelegramBot::TelegramBot(QObject *parent) :
    QObject(parent)
{
}
QString TelegramBot::BotID = "524042954:AAHqTPoUCeo6jDUPiDiC-8xi5Rvbzz6_sX4";
int TelegramBot::ChatID = 1322830441;

void TelegramBot::SendToChannel(QString msg)
{
    QString ChatIDstr = QString::number(ChatID);
    QString HttpRequest = "https://api.telegram.org/bot"+BotID+"/sendMessage?chat_id=-100"+ChatIDstr+"&text="+msg;
    QUrl url(HttpRequest);

    QString myUrl = QString::fromUtf8(url.toEncoded().constData());
    QString CurlCmd = "curl "+ myUrl;

    exeCurlCmd(CurlCmd);
}

QString TelegramBot::exeCurlCmd(QString  cmd)
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
        TelegramBot *tg;
        tg = new TelegramBot();
        emit tg->ErrorCurlTgBotexecSignal();
        return "failure";
    }
    return strProcessOutput;
}
