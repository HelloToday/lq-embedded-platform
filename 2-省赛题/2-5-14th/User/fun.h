#ifndef __FUN_H
#define __FUN_H

#include "headfile.h"


void led_show(uint8_t led, uint8_t mode);
void lcd_show(void);
void key_scan(void);
double get_volt(void);
void PWM_Change_Proc(void);
void while_fun(void);
void vmax_ctrl(void);
#endif
