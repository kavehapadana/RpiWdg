#include "tc_centserver.h"
#include <CommonDefines.h>

TC_CentServer::TC_CentServer()
{

}

void TC_CentServer::initClient()
{

}

void TC_CentServer::Parse_Byte(u_int8_t DATA)
{
    switch (msgStatus)
    {
        case UINIT:
            if (DATA == CentServer_Rx_Sync1)
                msgStatus = GOT_SYNC1;
            break;

        case GOT_SYNC1:
            if (DATA == CentServer_Rx_Sync2)
            {
                msgStatus = GOT_SYNC2;
                CheckSumm = 0;
            }
            else
                goto error;
            break;

        case GOT_SYNC2:

            msgID = DATA;
            msgStatus = GOT_ID;
            CheckSumm+=DATA;
            break;

        case GOT_ID:
            msgDeviceID = DATA;
            msgStatus = GOT_DeviceID;
            CheckSumm += DATA;
            break;

        case GOT_DeviceID:
            msgLen = DATA;
            msgStatus = GOT_LEN;
            msgIndex = 0;
            msg.clear();
            CheckSumm += DATA;
            break;

        case GOT_LEN:
            msg.append(DATA);
            CheckSumm +=DATA;
            msgIndex++;

            if (msgIndex >= msgLen)
            {
                msgStatus = GOT_PAYLOAD;
            }
          break;

        case GOT_PAYLOAD:
            if(DATA != CheckSumm)
            {
                goto error;
                qDebug() << "Error in parsebyte of TCP_Client" << this->objectName();
            }
            msgOptFull.clear();
            msgOptFull.append(CentServer_Rx_Sync1);
            msgOptFull.append(CentServer_Rx_Sync2);
            msgOptFull.append(msgID);
            msgOptFull.append(msgDeviceID);
            msgOptFull.append(msgLen);
            msgOptFull += msg;
            msgOptFull.append(CheckSumm);
            this->txClient_ID = primaryResponseFromServer;
            this->txClientMsg = msgOptFull;

            emit this->txClientDataSignal();
            qDebug() << "response from Server";
            // Parse_Message();
            goto restart;
    }
    return;
    error: errCnt++;
    restart: msgStatus = UINIT;
    return;
}
