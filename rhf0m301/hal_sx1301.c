
#include "hal_spi.h"
#include "wiringPi.h"


int spi_rst = 6;
int spi_cs0 = 10;

void hal_sx1301_dus(unsigned int ts)
{
	delayMicroseconds(ts);
}
void hal_sx1301_dms(unsigned int ts)
{
	delay(ts);
}

void hal_sx1301_nss(int val)
{
    hal_sx1301_dus(10);
    digitalWrite(spi_cs0, val);
    hal_sx1301_dus(10);
}
void hal_sx1301_rst(int val)
{
    digitalWrite(spi_rst, val);
}

void hal_sx1301_init(void)
{
    wiringPiSetup();
	pinMode(spi_rst, OUTPUT);
	pinMode(spi_cs0, OUTPUT);

    hal_sx1301_rst(0);
    hal_sx1301_dms(10);
    hal_sx1301_rst(1);
    hal_sx1301_dms(500);
    spi_open();
}

uint8_t hal_sx1301_wr(uint8_t val)
{
    uint8_t rx;
    spi_tranfer(&val, &rx, 1);
    return rx;
}

void hal_lgw_w(uint8_t address, uint8_t data)
{
    hal_sx1301_nss(0);
	hal_sx1301_wr( 0x80 | address );
	hal_sx1301_wr( data );
	hal_sx1301_nss(1);
}
void hal_lgw_r(uint8_t address, uint8_t *data)
{
    hal_sx1301_nss(0);
	hal_sx1301_wr( (~0x80) & address );
	*data = hal_sx1301_wr( 0x00 );
	hal_sx1301_nss(1);
}
void hal_lgw_wb(uint8_t address, uint8_t* data, uint16_t size)
{
    uint16_t i;

	hal_sx1301_nss(0);
    hal_sx1301_wr( 0x80 | address );
	for(i = 0; i<size; i++){
		hal_sx1301_wr( *data++ );
	}
	hal_sx1301_nss(1);
}
void hal_lgw_rb(uint8_t address, uint8_t* data, uint16_t size)
{
    uint16_t i;

	hal_sx1301_nss(0);
	hal_sx1301_wr( (~0x80) & address );
	for(i = 0; i<size; i++){
		data[i] = hal_sx1301_wr( 0x00 );
	}
	hal_sx1301_nss(1);
}
