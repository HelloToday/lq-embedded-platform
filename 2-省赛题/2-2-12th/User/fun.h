#ifndef __FUN_H
#define __FUN_H

#include "headfile.h"

typedef uint8_t u8;

#define LONG_TIME 1000

typedef struct
{
	char type[10];//车辆类型
	char num[10];//车牌号
	double dftime;//时间差
	double pay_price;//最终价格
	double type_price;//停车单价(元/小时)
	char reach_time[60];//到达时间字符串
	char leave_time[60];//离开时间字符串
	int reach;//到达时间时间戳
	int leave;//离开时间时间戳
}Car_TypedefStruct;

extern u8 rx_data;


void led_show(u8 led, u8 mode);
void lcd_show(void);
void key_scan(void);
void Recieve_Data_Proc(char* type_str,char* carnum_str, char* time_str, time_t* time);
uint8_t rec_data_check(char *type_str,char* carnum_str);
void Car_Change(char *type_str,char *carnum_str, char* time_str, time_t *time);
void Settle_Acount(Car_TypedefStruct car_pay);

#endif
