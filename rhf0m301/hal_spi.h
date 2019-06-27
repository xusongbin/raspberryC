
#ifndef __HAL_SPI_H__
#define __HAL_SPI_H__

#include "hardware.h"

int spi_tranfer(const uint8_t *txbuf, uint8_t *rxbuf, uint16_t len);
int spi_write(uint8_t* txbuf, int len);
int spi_read(uint8_t* rxbuf, int len);
int spi_open(void);
void spi_close(void);

#endif
