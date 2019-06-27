
#include<stdio.h>
#include "hal_sx1301.h"


int main()
{
	hal_sx1301_init();

    uint8_t ver;
    hal_lgw_r(1, &ver);
    printf("Get sx1301 version:%d\n", ver);
}