
#ifndef __HAL_SX1301_H__
#define __HAL_SX1301_H__

#include "hardware.h"

void hal_sx1301_nss(int val);
void hal_sx1301_rst(int val);
void hal_sx1301_init(void);
uint8_t hal_sx1301_wr(uint8_t val);

void hal_sx1301_dus(unsigned int ts);
void hal_sx1301_dms(unsigned int ts);

void hal_lgw_w(uint8_t address, uint8_t data);
void hal_lgw_r(uint8_t address, uint8_t *data);
void hal_lgw_wb(uint8_t address, uint8_t* data, uint16_t size);
void hal_lgw_rb(uint8_t address, uint8_t* data, uint16_t size);

#endif
