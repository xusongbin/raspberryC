
#include "wiringPi.h"
#include "stdio.h"

#define DS1302_SECOND_W     0x80
#define DS1302_SECOND_R     0x81
#define DS1302_MINUTE_W     0x82
#define DS1302_MINUTE_R     0x83
#define DS1302_HOUR_W       0x84
#define DS1302_HOUR_R       0x85
#define DS1302_DATE_W       0x86
#define DS1302_DATE_R       0x87
#define DS1302_MONTH_W      0x88
#define DS1302_MONTH_R      0x89
#define DS1302_WEEK_W       0x8A
#define DS1302_WEEK_R       0x8B
#define DS1302_YEAR_W       0x8C
#define DS1302_YEAR_R       0x8D
#define DS1302_PROTECT_W    0x8E
#define DS1302_PROTECT_R    0x8F

#define UCHAR_TO_BCD(n)     (((n / 10) << 4) | (n % 10))
#define BCD_TO_UCHAR(n)     ((n >> 4) * 10 + (n & 0xF))

#ifndef IO_MODE_OUT
#define IO_MODE_OUT         0
#endif
#ifndef IO_MODE_IN
#define IO_MODE_IN          1
#endif
#ifndef IO_VAL_HIGH
#define IO_VAL_HIGH         1
#endif
#ifndef IO_VAL_LOW
#define IO_VAL_LOW          0
#endif

typedef struct{
    unsigned char year;
    unsigned char month;
    unsigned char date;
    unsigned char week;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
}drv_ds1302_time_t;

int pin_clk = 22;
int pin_dio = 23;
int pin_rst = 24;

void drv_ds1302_init(void)
{
    pinMode(pin_clk, OUTPUT);
    pinMode(pin_dio, OUTPUT);
    pinMode(pin_rst, OUTPUT);
}
void drv_ds1302_clk(unsigned char val)
{
    if(val){
        digitalWrite(pin_clk, HIGH);
    }else{
        digitalWrite(pin_clk, LOW);
    }
}
void drv_ds1302_rst(unsigned char val)
{
    if(val){
        digitalWrite(pin_rst, HIGH);
    }else{
        digitalWrite(pin_rst, LOW);
    }
}
void drv_ds1302_dout(unsigned char val)
{
    if(val){
        digitalWrite(pin_dio, HIGH);
    }else{
        digitalWrite(pin_dio, LOW);
    }
}
void drv_ds1302_dset(unsigned char val)
{
    if(IO_MODE_OUT == val){
        pinMode(pin_dio, OUTPUT);
    }else{
        pinMode(pin_dio, INPUT);
    }
}
unsigned char drv_ds1302_din(void)
{
    return digitalRead(pin_dio);
}
void drv_ds1302_delay(void)
{
    delayMicroseconds(5);
}

void drv_ds1302_putchar(unsigned char data)
{
    int i;
    drv_ds1302_dset(IO_MODE_OUT);
    drv_ds1302_delay();
    for(i=0; i<8; i++)
    {
        drv_ds1302_dout(data & 0x01);
        drv_ds1302_clk(IO_VAL_HIGH);
        drv_ds1302_delay();
        drv_ds1302_clk(IO_VAL_LOW);
        drv_ds1302_delay();
        data >>= 1;
    }
}
unsigned char drv_ds1302_getchar(void)
{
    int i;
    unsigned char data=0;
    drv_ds1302_dset(IO_MODE_IN);
    drv_ds1302_delay();
    for(i=0; i<8; i++)
    {
        data >>= 1;
        if(drv_ds1302_din()){
            data |= 0x80;
        }
        drv_ds1302_clk(IO_VAL_HIGH);
        drv_ds1302_delay();
        drv_ds1302_clk(IO_VAL_LOW);
        drv_ds1302_delay();
    }
    return data;
}
void drv_ds1302_write_addr(unsigned char addr, unsigned char data)
{
    drv_ds1302_rst(IO_VAL_LOW);
    drv_ds1302_clk(IO_VAL_LOW);
    drv_ds1302_rst(IO_VAL_HIGH);
    drv_ds1302_putchar(addr);
    drv_ds1302_putchar(data);
    drv_ds1302_rst(IO_VAL_LOW);
    drv_ds1302_clk(IO_VAL_HIGH);
}
unsigned char drv_ds1302_read_addr(unsigned char addr)
{
    unsigned char data=0;
    drv_ds1302_rst(IO_VAL_LOW);
    drv_ds1302_clk(IO_VAL_LOW);
    drv_ds1302_rst(IO_VAL_HIGH);
    drv_ds1302_putchar(addr);
    data = drv_ds1302_getchar();
    drv_ds1302_clk(IO_VAL_HIGH);
    drv_ds1302_rst(IO_VAL_LOW);
    return data;
}
void drv_ds1302_time_write(drv_ds1302_time_t* tt)
{
    drv_ds1302_write_addr(DS1302_PROTECT_W, 0x00);
    drv_ds1302_write_addr(DS1302_YEAR_W, UCHAR_TO_BCD(tt->year));
    drv_ds1302_write_addr(DS1302_MONTH_W, UCHAR_TO_BCD(tt->month));
    drv_ds1302_write_addr(DS1302_DATE_W, UCHAR_TO_BCD(tt->date));
    drv_ds1302_write_addr(DS1302_WEEK_W, UCHAR_TO_BCD(tt->week));
    drv_ds1302_write_addr(DS1302_HOUR_W, UCHAR_TO_BCD(tt->hour));
    drv_ds1302_write_addr(DS1302_MINUTE_W, UCHAR_TO_BCD(tt->minute));
    drv_ds1302_write_addr(DS1302_SECOND_W, UCHAR_TO_BCD(tt->second));
    drv_ds1302_write_addr(DS1302_PROTECT_W, 0x80);
}
void drv_ds1302_time_read(drv_ds1302_time_t* tt)
{
    tt->year = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_YEAR_R));
    tt->month = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_MONTH_R));
    tt->date = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_DATE_R));
    tt->week = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_WEEK_R));
    tt->hour = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_HOUR_R));
    tt->minute = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_MINUTE_R));
    tt->second = BCD_TO_UCHAR(drv_ds1302_read_addr(DS1302_SECOND_R));
}
void drv_ds1302_time_init(void)
{
    drv_ds1302_time_t tt;
    tt.year = 19;
    tt.month = 4;
    tt.date = 25;
    tt.week = 4;
    tt.hour = 22;
    tt.minute = 36;
    tt.second = 00;
    drv_ds1302_time_write(&tt);
}

void app_ds1302_evt(void)
{
    drv_ds1302_time_t tt;
    drv_ds1302_time_read(&tt);
    printf("App ds1302=>%02d-%02d-%02d %02d:%02d:%02d %d\r\n", tt.year, tt.month, tt.date, tt.hour, tt.minute, tt.second, tt.week);
}

int main()
{
    unsigned int tms;

    wiringPiSetup();
    drv_ds1302_init();
	
    printf("Start...\r\n");
	tms = millis();
	while(1)
	{
		if( (millis()-tms) >= 1000)
		{
			tms = millis();
			
			app_ds1302_evt();
		}
	}
}