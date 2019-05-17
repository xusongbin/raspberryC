
#include "wiringPi.h"
#include "stdio.h"

//gcc drv_dht11.c -o drv_dht11 -lwiringPi

int pin_dio = 28;

void drv_dht11_output(void)
{
    pinMode(pin_dio, OUTPUT);
}
void drv_dht11_input(void)
{
    pinMode(pin_dio, INPUT);
}
void drv_dht11_write(unsigned char val)
{
    if(val)
    {
        digitalWrite(pin_dio, HIGH);
    }
    else
    {
        digitalWrite(pin_dio, LOW);
    }
    
}
unsigned char drv_dht11_read(void)
{
    return digitalRead(pin_dio);
}
void  drv_dht11_release(void)
{
    drv_dht11_output();
    drv_dht11_write(1);
}

int drv_dht11_get(float *temp, float *humy)
{
    unsigned char buf[5];
    unsigned char val;
    unsigned long tus;
    int i;

    memset(buf, 0, sizeof(buf));

    drv_dht11_output();
    drv_dht11_write(0);
    delay(20);
    drv_dht11_write(1);
    delayMicroseconds(20);
    drv_dht11_input();

    tus = micros();
    while(drv_dht11_read()==0)
    {
        if((micros() - tus) > 80){
            LOG(DEBUG, "ack time out");
            drv_dht11_release();
            return -1;
        }
    }
    tus = micros();
    while(drv_dht11_read()==1)
    {
        if((micros() - tus) > 100){
            LOG(DEBUG, "notice time out");
            drv_dht11_release();
            return -2;
        } 
    }

    for(i=0; i<40; i++){
        if(i%8 == 0){
            val = 0;
        }
        tus = micros();
        while(drv_dht11_read()==0){
            if((micros() - tus) > 60){
                LOG(DEBUG, "read data err");
                drv_dht11_release();
                return -3;
            }
        }
        delayMicroseconds(40);
        if(drv_dht11_read()){
            val = (val<<1) + 1 ;
        }else{
            val <<= 1;
        }
        buf[i/8] = val;
        tus = micros();
        while(drv_dht11_read()==1){
            if((micros() - tus) > 40){
                LOG(DEBUG, "read data err");
                drv_dht11_release();
                return -3;
            }
        }
    }
    drv_dht11_release();

    val = (buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF;
    if(val ==  buf[4]){
        *temp = buf[0];
        *humy = buf[2];
        return 0;
    }else{
        LOG(DEBUG, "check sum err!");
        return -4;
    }
}

int main()
{
    unsigned int tms;

    wiringPiSetup();
    tms = millis();
    while(1)
    {
		if( (millis()-tms) > 1000)
		{
            float temp, humy;
			tms = millis();
            if(0 == drv_dht11_get(&temp, &humy)){
                printf("current=>temp:%.1f humy:%.1f\n", temp, humy);
            }
        }
    }
}