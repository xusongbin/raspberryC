
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

//gcc drv_i2c_24c16.c -o drv_i2c_24c16 -lwiringPi

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// SD卡类型定义
#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06	   
// SD卡指令表	   
#define CMD0    0       //卡复位
#define CMD1    1
#define CMD8    8       //命令8 ,SEND_IF_COND
#define CMD9    9       //命令9 ,读CSD数据
#define CMD10   10      //命令10,读CID数据
#define CMD12   12      //命令12,停止数据传输
#define CMD16   16      //命令16,设置SectorSize 返回0x00
#define CMD17   17      //命令17,读sector
#define CMD18   18      //命令18,读 Multi sector
#define CMD23   23      //命令23,设置多sector写入前预先擦除N个block
#define CMD24   24      //命令24,写sector
#define CMD25   25      //命令25,写Multi sector
#define CMD41   41      //命令41,返回0x00
#define CMD55   55      //命令55,返回0x01
#define CMD58   58      //命令58,读OCR信息
#define CMD59   59      //命令59,使能/禁止CRC,应返回0x00
//数据写入回应字意义
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
//SD卡回应标志字
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF


static void pabort(const char *s)  
{  
    perror(s);  
    abort();  
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 1000000;
static uint16_t delay;
static int spi_fd;
static uint8_t sd_type;

uint8_t spi_readwrite_byte(uint8_t byte)
{
	uint8_t tx[1];
	uint8_t rx[1];
	int ret;

	tx[0] = byte;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 1,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
	};
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret<1){
		pabort("can't read/write byte spi message");
	}
	return rx[0];
}
void spi_readwrite_buf(uint8_t *buf, uint16_t plen)
{
	uint8_t rx[1024];
	int ret;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)rx,
		.len = plen,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
	};
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret<1){
		pabort("can't read/write buf spi message");
	}
	memcpy(buf, rx, plen);
}
int spi_read_cmp(uint8_t res, uint32_t num)
{
	int i;
	for(i=0; i<num; i++){
		if(spi_readwrite_byte(0xFF) == res) return 0;
	}
	return -1;
}

void spi_set_speed(uint32_t sp)
{
	speed = sp;
}
void sd_disselect(void)
{
	spi_readwrite_byte(0xFF);
}
int sd_waitready(void)
{
	return spi_read_cmp(0xFF, 0xFFFFFF);
}
int sd_select(void)
{
	if(sd_waitready()==0) return 0;
	sd_disselect();
	return -1;
}
int sd_get_response(uint8_t res)
{
	if(spi_read_cmp(res, 0xFFFF)==0) return 0;
	return 0xFF;
}
int sd_recvdata(uint8_t *buf, uint16_t plen)
{
	if(sd_get_response(0xFE)!=0) return -1;
	spi_readwrite_buf(buf, plen);
	spi_readwrite_byte(0xFF);
	spi_readwrite_byte(0xFF);
	return 0;
}
int sd_sendblock(uint8_t *buf, uint8_t cmd)
{
	if(sd_waitready()!=0) return -1;
	if(cmd != 0xFD){
		spi_readwrite_buf(buf, 512);
		spi_readwrite_byte(0xFF);
		spi_readwrite_byte(0xFF);
		if((spi_readwrite_byte(0xFF) & 0x1F) != 0x05)
			return -1;
	}
	return 0;
}
int sd_sendcmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	int i;
	uint8_t rx;

	sd_disselect();
	if(sd_select()!=0) return 0xFF;
	spi_readwrite_byte(cmd | 0x40);
	spi_readwrite_byte(arg >> 24);
    spi_readwrite_byte(arg >> 16);
    spi_readwrite_byte(arg >> 8);
    spi_readwrite_byte(arg);
    spi_readwrite_byte(crc);
	if(cmd == CMD12){
		spi_readwrite_byte(0xFF);
	}
	for(i=0; i<0x1F; i++){
		rx = spi_readwrite_byte(0xFF);
		if((rx & 0x80) != 0x80) break;
	}
	return rx;
}
int sd_get_cid(uint8_t *buf)
{
	if(sd_sendcmd(CMD10, 0, 0x01) == 0x00){
		sd_recvdata(buf, 16);
		return 0;
	}
	return -1;
}
int sd_get_csd(uint8_t *buf)
{
	if(sd_sendcmd(CMD9, 0, 0x01) == 0x00){
		sd_recvdata(buf, 16);
		return 0;
	}
	return -1;
}
uint32_t sd_get_sectorcount(void)
{
	uint8_t csd[16];
	uint16_t csize;
	uint8_t n;

	if(sd_get_csd(csd) != 0) return 0;
	if((csd[0] & 0xC0) == 0x40){	// V2.00 card
		csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
		return (uint32_t)(csize) << 10;
	}else{
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
		return (uint32_t)csize << (n - 9);
	}
}
int sd_init_config(void)
{
	int i;
	uint8_t r1;
	uint16_t retry;
	uint8_t buf[10];

	memset(buf, 0xFF, ARRAY_SIZE(buf));
	spi_set_speed(400000);
	printf("debug->set spi speed to 400k\r\n");
    spi_readwrite_buf(buf, 10);
    printf("debug->write 10 byte 0xFF to sd\r\n");
	for(i=0; i<20; i++){
		r1 = sd_sendcmd(CMD0, 0, 0x95);
		if(r1 == 0x01) break;
	}
	sd_type = SD_TYPE_ERR;
	printf("debug->start get sd type, r1:%d\r\n", r1);
	if(r1 == 0x01){
		if(sd_sendcmd(CMD8, 0x1AA, 0x87) == 1){	// sd v2.0
			printf("debug->get sd is v2.0\r\n");
			memset(buf, 0xFF, 4);
			spi_readwrite_buf(buf, 4);	// Get trailing return value of R7 resp
			if((buf[2] == 0x01) && (buf[3] == 0xAA)){	// check card support 2.7~3.6V or not
				printf("debug->check card support 2.7~3.6V or not\r\n");
                retry = 0xFFFE;
                while(retry){
					sd_sendcmd(CMD55, 0, 0x01);	// send CMD55
					if(sd_sendcmd(0x41, 0x40000000, 0x01) == 0){
						break;
					}
					retry --;
				}
				if(retry && sd_sendcmd(CMD58, 0, 0x01) == 0){	// start check SD2.0
					memset(buf, 0xFF, 4);
					spi_readwrite_buf(buf, 4);	// get OCR
					if(buf[0] & 0x40){	// check CCS
						printf("debug->check card type V2HC\r\n");
                        sd_type = SD_TYPE_V2HC;
					}else{
						printf("debug->check card type V2\r\n");
                        sd_type = SD_TYPE_V2;
					}
				}
			}else{	// SD V1.x  MMC  V3
				printf("debug->check card type V1.x MMC V3\r\n");
                sd_sendcmd(CMD55, 0, 0x01);
                r1 = sd_sendcmd(CMD41, 0, 0x01);
				if(r1 <= 1){
					printf("debug->check card type V1\r\n");
                    sd_type = SD_TYPE_V1;
                    retry = 0xFFFE;
                    while(retry){
						sd_sendcmd(CMD55, 0, 0x01);
                        r1 = sd_sendcmd(CMD41, 0, 0x01);
                        if(r1 == 0) break;
                        retry --;
					}
				}else{
					printf("debug->check card type MMC\r\n");
                    sd_type = SD_TYPE_MMC;
                    retry = 0xFFFE;
                    while(retry){
						r1 = sd_sendcmd(CMD1, 0, 0x01);
                        if(r1 == 0) break;
                        retry --;
					}
				}
				if(retry == 0 || sd_sendcmd(CMD16, 512, 0x01) != 0){
					printf("debug->check card type ERR\r\n");
                    sd_type = SD_TYPE_ERR;
				}
			}
		}
	}
	sd_disselect();
    spi_set_speed(30000000);
    printf("debug->set spi speed to 30M\r\n");
    if(sd_type) return 0;
	else if(r1) return r1;
	else return 0xAA;
}
int sd_read_disk2(uint8_t *buf, uint32_t sector, uint8_t cnt)
{
	uint8_t r1=0xFF;

	if(sd_type != SD_TYPE_V2HC) sector <<= 9;
    if(cnt == 1){
		r1 = sd_sendcmd(CMD17, sector, 0x01);
        if(r1==0) sd_recvdata(buf, 512);
	}else{
		r1 = sd_sendcmd(CMD18, sector, 0x01);
		do{
			r1 = sd_recvdata(buf, 512);
			buf += 512;
		}while(--cnt && r1==0);
		sd_sendcmd(CMD12, 0, 0X01);
	}
    sd_disselect();
    return r1;
}
int SD_WriteDisk2(uint8_t*buf,uint32_t sector,uint8_t cnt)
{
	uint8_t r1;

	if(sd_type != SD_TYPE_V2HC) sector *= 512;
	if(cnt==1){
		r1 = sd_sendcmd(CMD24, sector, 0X01);
		if(r1==0){
			r1 = sd_sendblock(buf, 0xFE);  
		}
	}else{
		if(sd_type == SD_TYPE_MMC){
			sd_sendcmd(CMD55,0,0X01);	
			sd_sendcmd(CMD23,cnt,0X01);
		}
		r1 = sd_sendcmd(CMD25, sector, 0X01);
		if(r1 == 0){
			do{
				r1 = sd_sendblock(buf, 0xFC);  
				buf += 512;  
			}while(--cnt && r1==0);
			r1 = sd_sendblock(0,0xFD);
		}
	}   
	sd_disselect();
	return r1;
}

int main()
{
	uint16_t i, j;
	uint16_t tms;
	uint8_t rx[1024];

	spi_fd = open(device, O_RDWR);
	if(spi_fd<0){
		printf("can't open device\r\n");
	}

	if(sd_init_config() == 0){
		printf("debug->get capacity:%d\r\n", sd_get_sectorcount());
		if(sd_read_disk2(rx, 0, 1) == 0){
			printf("debug->get block:\r\n");
			for(i=0; i<512;i ++){
				if(i%16==0 && i!=0) printf("\r\n");
				printf("%02X ", rx[i]);
			}
			printf("\r\n");
		}else{
			printf("debug->get block err\r\n");
		}
	}
}



