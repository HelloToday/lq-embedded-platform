#ifndef __FUN_H
#define __FUN_H

#include "stm32g4xx_hal.h"
extern int8_t rom_time[5][6];

void led_show(uint8_t led, uint8_t mode);
void lcd_show(void);
void lcd_sub(void);
void key_scan(void);
#endif
