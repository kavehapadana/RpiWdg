#include "cSPI.h"
#include "bcm2835.h"

using namespace std;

cSPI::cSPI(){

}

void cSPI::init_spi_bcm2835()
{
    init_gpio();
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0,LOW);
    usleep(100);

}


void cSPI::init_gpio(){

    if(!bcm2835_init())
        printf(" init bcm_2835 not work \n");

        /*Init gpios needed
        bcm2835_gpio_fsel(number of gpio-bcm2835.h,BCM2835_GPIO_FSEL_OUTP) can be outbut
        bcm2835_gpio_write(number of gpio-bcm2835.h,LOW) or High
        */

}
