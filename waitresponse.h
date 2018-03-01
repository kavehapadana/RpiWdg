#ifndef WAITRESPONSE_H
#define WAITRESPONSE_H

#include <QObject>

class WaitResponse : public QObject
{
    Q_OBJECT
public:
    explicit WaitResponse(QObject *parent = 0);
    bool Response;

signals:

public slots:


private:
    //QTimer * timerWait;

};

#endif // WAITRESPONSE_H
