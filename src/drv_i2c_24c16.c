
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "stdio.h"
#include "hardware.h"

//gcc drv_i2c_24c16.c -o drv_i2c_24c16 -lwiringPi

uint8_t drv_24c16_readbyte(int fd, uint16_t addr)	// I2C_address + data_addr
{
    return wiringPiI2CReadReg16(fd, addr);
}
void drv_24c16_writebyte(int fd, uint16_t addr, uint8_t data)
{
	wiringPiI2CWriteReg16(fd, addr, data);
}

int main()
{
	int fd;
	uint16_t i, j;
	uint16_t tms;
	
	wiringPiSetup();
	// tms = millis();
	// while(1)
	// {
	// 	if( (millis()-tms) > 1000)
	// 	{
	// 		tms = millis();
			

	// 	}
	// }
	for(j=0;j<8;j++){
		fd = wiringPiI2CSetup(0x50 + j);
		for(i=0; i<256; i++)
		{
			// drv_24c16_writebyte(i, i);
			if(i%16 == 0 && i != 0){
				printf("\r\n");
			} 
			printf("%02X ", drv_24c16_readbyte(fd, i));
		}
		printf("\r\n");
	}
}



