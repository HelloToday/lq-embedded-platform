#ifndef __I2C_HAL_H
#define __I2C_HAL_H

#include "stm32g4xx_hal.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);
void eep_write_u8(uint8_t adr, uint8_t data);
uint8_t eep_read_u8(uint8_t adr);
void eep_write_arr(uint8_t adr, uint8_t* w_arr, uint8_t w_len);
void eep_read_arr(uint8_t adr, uint8_t* r_arr, uint8_t r_len);
void eep_write_double(uint8_t adr, double* w_double);
void eep_read_double(uint8_t adr, double* r_double);
void mcp_write(uint8_t data);
uint8_t mcp_read(void);
void eep_write_u16(uint8_t adr, uint16_t* data_u16, uint8_t w_len);
void eep_read_u16(uint8_t adr, uint16_t* data_u16, uint8_t r_len);
#endif
