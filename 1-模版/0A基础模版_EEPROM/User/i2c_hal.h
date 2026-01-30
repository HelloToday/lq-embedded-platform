#ifndef __I2C_HAL_H
#define __I2C_HAL_H

#include "stm32g4xx_hal.h"
#include "stdlib.h"
void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);
void eeprom_write_buff(uint8_t adr, uint8_t* w_buff, uint8_t lenth);

void eeprom_read_buff(uint8_t adr, uint8_t* r_buff, uint8_t lenth);
void eep_write(uint8_t adr,uint8_t data);
uint8_t eep_read(uint8_t adr);

void eeprom_write_buff_float(uint8_t adr,double* w_double);
void eeprom_read_buff_float(uint8_t adr,double* r_double);

void eep_read_double(uint8_t adr,double * r_double);//与eeprom_write_buff_float(uint8_t adr,double* w_double);相同，eep_write_double如此
void eep_write_double(uint8_t adr,double* w_double);
#endif
