
#include "wiringPi.h"
#include "stdio.h"
#include "hardware.h"

//gcc drv_24c16.c -o drv_24c16 -lwiringPi

#define IO_MODE_OUT 0
#define IO_MODE_IN 1

#define IO_VAL_HIGH 1
#define IO_VAL_LOW 0

int clk = 9;
int sda = 8;

void drv_24c16_init(void)
{
	wiringPiSetup();
	pinMode(clk, OUTPUT);
	pinMode(sda, OUTPUT);
	digitalWrite(clk, LOW);
}
void drv_24c16_clk(uint8_t sta)
{
	if(IO_VAL_HIGH==sta){
		digitalWrite(clk, HIGH);
	}else{
		digitalWrite(clk, LOW);
	}
}
void drv_24c16_dout(uint8_t sta)
{
	if(IO_VAL_HIGH==sta){
		digitalWrite(sda, HIGH);
	}else{
		digitalWrite(sda, LOW);
	}
}
uint8_t drv_24c16_din(void)
{
	return digitalRead(sda);
}
void drv_24c16_dset(uint8_t sta)
{
	if(IO_MODE_OUT==sta){
		pinMode(sda, OUTPUT);
	}else{
		pinMode(sda, INPUT);
	}
}
void drv_24c16_dus(void)
{
	delayMicroseconds(5);
}
void drv_24c16_dms(void)
{
	delay(5);
}
void drv_24c16_start(void)
{
	drv_24c16_clk(IO_VAL_LOW);
    drv_24c16_dout(IO_VAL_HIGH);
    drv_24c16_clk(IO_VAL_HIGH);
    drv_24c16_dus();
    drv_24c16_dout(IO_VAL_LOW);
    drv_24c16_dus();
    drv_24c16_clk(IO_VAL_LOW);
    drv_24c16_dus();
	drv_24c16_dout(IO_VAL_HIGH);
}
void drv_24c16_stop(void)
{
    drv_24c16_dout(IO_VAL_LOW);
    drv_24c16_clk(IO_VAL_HIGH);
    drv_24c16_dus();
    drv_24c16_dout(IO_VAL_HIGH);
    drv_24c16_dus();
    drv_24c16_clk(IO_VAL_LOW);
    drv_24c16_dus();
}
uint8_t drv_24c16_getchar(uint8_t ack)
{
	uint8_t i;
    uint8_t data = 0;
    drv_24c16_dset(IO_MODE_IN);
    for(i=0; i<8; i++){
        drv_24c16_dus();
        drv_24c16_clk(IO_VAL_HIGH);
        drv_24c16_dus();
        data <<= 1;
        if(drv_24c16_din()){
            data += 1;
		}
        drv_24c16_clk(IO_VAL_LOW);
	}
	drv_24c16_dus();
    drv_24c16_dset(IO_MODE_OUT);
	if(ack){
		drv_24c16_dout(IO_VAL_HIGH);
	}else{
		drv_24c16_dout(IO_VAL_LOW);
	}
	drv_24c16_dus();
    drv_24c16_clk(IO_VAL_HIGH);
    drv_24c16_dus();
    drv_24c16_clk(IO_VAL_LOW);
    drv_24c16_dus();
	drv_24c16_dout(IO_VAL_HIGH);
	drv_24c16_dus();
    return data;
}
void drv_24c16_putchar(uint8_t data)
{
	uint8_t i;
    for(i=0; i<8; i++){
        if(data & 0x80){
            drv_24c16_dout(IO_VAL_HIGH);
		}else{
            drv_24c16_dout(IO_VAL_LOW);
		}
        data <<= 1;
		drv_24c16_dus();
        drv_24c16_clk(IO_VAL_HIGH);
        drv_24c16_dus();
        drv_24c16_clk(IO_VAL_LOW);
	}
	drv_24c16_dus();
    drv_24c16_dout(IO_VAL_LOW);
	drv_24c16_dus();
    drv_24c16_clk(IO_VAL_HIGH);
    drv_24c16_dus();
    drv_24c16_clk(IO_VAL_LOW);
	drv_24c16_dus();
	drv_24c16_dout(IO_VAL_HIGH);
	drv_24c16_dus();
}
uint8_t drv_24c16_readbyte(uint16_t addr)
{
	uint8_t data;
    drv_24c16_start();
    drv_24c16_putchar(0xA0 | ((addr>>8)<<1));
    drv_24c16_putchar(addr & 0xFF);
    drv_24c16_start();
    drv_24c16_putchar(0xA0 | 0x01);
    data = drv_24c16_getchar(1);
    drv_24c16_stop();
    return data;
}
void drv_24c16_writebyte(uint16_t addr, uint8_t data)
{
    drv_24c16_start();
    drv_24c16_putchar(0xA0 | ((addr>>8)<<1));
    drv_24c16_putchar(addr & 0xFF);
    drv_24c16_putchar(data);
    drv_24c16_stop();
    drv_24c16_dms();
}

int main()
{
	uint16_t i;
	uint16_t tms;
	
	drv_24c16_init();
	
	// tms = millis();
	// while(1)
	// {
	// 	if( (millis()-tms) > 1000)
	// 	{
	// 		tms = millis();
			

	// 	}
	// }
	for(i=0; i<256; i++)
	{
		// drv_24c16_writebyte(i, i);
		if(i%16 == 0 && i != 0){
			printf("\r\n");
		}
		printf("%02X ", drv_24c16_readbyte(i));
	}
	printf("\r\n");
}



