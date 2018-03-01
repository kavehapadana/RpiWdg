#include <getopt.h>
#include <sys/ioctl.h>
//#include <bcm2835.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <ctime>
#include <sys/time.h>
//#include <bcm2835.h>

class cSPI
{
private:
void init_gpio();
protected:
///we dont want to drived class from cSPI
public:
cSPI();
void init_spi_bcm2835();
};

