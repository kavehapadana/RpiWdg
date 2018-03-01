#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <QObject>

class TelegramBot : public QObject
{
    Q_OBJECT
public:
    explicit TelegramBot(QObject *parent = 0);
    void static SendToChannel(QString msg);
    static QString  exeCurlCmd(QString  cmd);
    static QString BotID;
    static int ChatID;
signals:
    void ErrorCurlTgBotexecSignal();
private:

public slots:
};

#endif // TELEGRAMBOT_H
