#ifndef SPI_CNN_H
#define SPI_CNN_H

#include <QObject>

class SPI_Cnn : public QObject
{
    Q_OBJECT
public:
    explicit SPI_Cnn(QObject *parent = 0);

    QByteArray  txSPIMsg;
    QByteArray  rxSPIMsg;
    u_int8_t    txSPI_ID;
    u_int8_t    rxSPI_ID;

    u_int8_t bitRelaysReset;

private:

signals:
void txSPI_DataSignal();
void finishObject();

public slots:
void SPI_RPI_Init();
void rxSPI_DataSlot();
unsigned char* spi_ExchangeData(unsigned char* tx_array,unsigned char* rx_array,u_int32_t lent);

};

#endif // SPI_CNN_H
