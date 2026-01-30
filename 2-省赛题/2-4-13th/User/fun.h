#ifndef __FUN_H
#define __FUN_H

#include "headfile.h"
extern uint8_t rec_data;

void led_show(uint8_t led, uint8_t mode);
void lcd_show(void);
void key_scan(void);
void Rec_Buff_Proc(void);

#endif
