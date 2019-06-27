
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define SPI_DEBUG 0

static const char *device = "/dev/spidev0.0";
static uint8_t bits = 8;
static uint32_t speed = 8000000;
static uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
static int spi_fd;


static void pabort(const char *s)  
{  
    perror(s);  
    abort();  
}

int spi_tranfer(const uint8_t *txbuf, uint8_t *rxbuf, uint16_t len)
{
	int ret;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)txbuf,
		.rx_buf = (unsigned long)rxbuf,
		.len = len,
        .delay_usecs = 0,
	};
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret<1){
		printf("spi spi_tranfer error!\n");
	}else{
		#if SPI_DEBUG
		int i;
		printf('SPI TX:');
		for(i=0;i<len;i++){
			printf("%02X ", txbuf[i]);
		}
		printf('\n');
		printf('SPI RX:');
		for(i=0;i<len;i++){
			printf("%02X ", rxbuf[i]);
		}
		printf('\n');
		#endif
	}
	return ret;
}

int spi_write(uint8_t* txbuf, int len)
{
	int ret;
	int fd = spi_fd;

	ret = write(fd, txbuf, len);
	if(ret<0){
		printf("spi write error!\n");
	}else{
		#if SPI_DEBUG
		int i;
		printf('SPI TX:');
		for(i=0;i<len;i++){
			printf("%02X ", txbuf[i]);
		}
		printf('\n');
		#endif
	}
	return ret;
}

int spi_read(uint8_t* rxbuf, int len)
{
	int ret;
	int fd = spi_fd;

	ret = read(fd, rxbuf, len);
	if(ret<0){
		printf("spi read error!\n");
	}else{
		#if SPI_DEBUG
		int i;
		printf('SPI RX:');
		for(i=0;i<len;i++){
			printf("%02X ", txbuf[i]);
		}
		printf('\n');
		#endif
	}
	return ret;
}

int spi_open(void)
{
	int fd;
	int ret=0;

	if(spi_fd!=0){
		return 0xF1;
	}
	fd = open(device, O_RDWR);
	if(fd<0){
		printf("spi open error!\n");
	}else{
		printf("spi open successful!\n");
	}
	spi_fd = fd;

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
	pabort("can't get spi mode");

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
	pabort("can't get bits per word");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("spi bits: %d\n", bits);
	printf("spi speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);

	return ret;
}

void spi_close(void)
{
	close(spi_fd);
}

