
#include "wiringPi.h"
#include "stdio.h"

//gcc drv_hcsr04.c -o drv_hcsr04 -lwiringPi

int main()
{
	int trig = 28;
	int echo = 29;
	unsigned int tms;
	unsigned int timer_start, timer_spend;
	unsigned int distance;
	
	wiringPiSetup();
	
	tms = millis();
	while(1)
	{
		if( (millis()-tms) > 1000)
		{
			tms = millis();
			
			pinMode(trig, OUTPUT);
			pinMode(echo, INPUT);
			
			timer_spend = micros();
			digitalWrite(trig, HIGH);
			delayMicroseconds(10);
			digitalWrite(trig, LOW);
			
			while(digitalRead(echo)==0);
			timer_start = micros();
			while(digitalRead(echo)==1)
			{
				if((micros()-timer_start)>23500){
					break;
				}
			}
			timer_spend = micros() - timer_start;
			if(timer_spend<116 || timer_spend>23500) continue;
			distance = timer_spend/58;
			printf("current distance:%d\n", distance);
		}
	}
}



