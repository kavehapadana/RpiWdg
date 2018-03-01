#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <QObject>

class TelegramBot : public QObject
{
    Q_OBJECT
public:
    explicit TelegramBot(QObject *parent = 0);

signals:

public slots:

};

#endif // TELEGRAMBOT_H
