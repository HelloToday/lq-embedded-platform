#include "fun.h"

u8 part_mode = 0;
u8 lcd_text[10][20];
u8 stop_cnbr = 0;
u8 stop_vnbr = 0;
u8 stop_idle = 8;
float cnbr_pay = 3.5;
float vnbr_pay = 2.0;
uint32_t fre = 0;
u8 pwm_mode = 0;
u8 led1_mode = 0;
u8 led2_mode = 0;

u8 rx_data = 0;
u8 rx_lenth = 0;
u8 rx_buff[3][12]={0,0,0};
u8 buff_line = 0;
u8 buff_col = 0;
u8 rx_error = 0;//1:error
u8 rx_success = 0;//1:入库，2：出库
int recYear = 0;
int recMon = 0;
int recDay = 0;
int recHour = 0;
int recMin = 0;
int recSec = 0;
time_t mytime;
char my_type[10];
char my_carnum[10];
char my_cartime[60];

Car_TypedefStruct car_struct[10];//车辆存储结构体数组
uint8_t car_dex = 0;						 //存储数组下标

u8 error_str[10]={"Error\r\n"};
u8 outcar_str[40];

void led_show(u8 led, u8 mode)
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	if(mode)
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8<<(led - 1),GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8<<(led - 1),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void lcd_part0(void)
{
	sprintf((char*)lcd_text[0],"                  ");
	sprintf((char*)lcd_text[1],"       Data       ");
	sprintf((char*)lcd_text[2],"                  ");
	sprintf((char*)lcd_text[3],"   CNBR:%d        ",stop_cnbr);
	sprintf((char*)lcd_text[4],"                  ");
	sprintf((char*)lcd_text[5],"   VNBR:%d        ",stop_vnbr);//stop_vnbr
	sprintf((char*)lcd_text[6],"                  ");
	sprintf((char*)lcd_text[7],"   IDLE:%d        ",stop_idle);//
	sprintf((char*)lcd_text[8],"                  ");
	sprintf((char*)lcd_text[9],"                  ");
	
}
void lcd_part1(void)
{
	sprintf((char*)lcd_text[0],"                  ");
	sprintf((char*)lcd_text[1],"       Para       ");
	sprintf((char*)lcd_text[2],"                  ");
	sprintf((char*)lcd_text[3],"   CNBR:%.2f        ",cnbr_pay);
	sprintf((char*)lcd_text[4],"                  ");
	sprintf((char*)lcd_text[5],"   VNBR:%.2f        ",vnbr_pay);
	sprintf((char*)lcd_text[6],"                  ");
	sprintf((char*)lcd_text[7],"                  ");
	sprintf((char*)lcd_text[8],"                  ");
	sprintf((char*)lcd_text[9],"                  ");
	
}
void lcd_show(void)
{
	if(part_mode == 0)
	{
		lcd_part0();
	}
	else if(part_mode == 1)
	{
		lcd_part1();
	}
	for(int i = 0; i< 10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
	
	if(stop_idle>0)
	{
		led1_mode = 1;
	}
	else
	{
		led1_mode = 0;
	}
	led_show(1,led1_mode);
	led_show(2,led2_mode);
	
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t counter7 = 0;
	static uint32_t counter15 = 0;
	
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{
			counter7 = 0;
			key_scan();
			if(rx_success)
			{
				Recieve_Data_Proc(my_type,my_carnum,my_cartime,&mytime);
				rx_error = rec_data_check(my_type,my_carnum);
				if(rx_error == 0)
				{
					Car_Change(my_type,my_carnum,my_cartime,&mytime);
				}
				
				if(rx_error == 0 && rx_success == 2)
				{
					HAL_UART_Transmit_IT(&huart1,outcar_str,30);
				}	
				rx_success = 0;
			}
			
			if(rx_error == 1)
			{
				HAL_UART_Transmit_IT(&huart1,error_str,8);
				//printf("error\r\n");
				rx_error = 0;
			}
				
		}
	}
	else if(htim->Instance == TIM15)
	{
		counter15++;
		if(counter15 >= 10)
		{
			HAL_TIM_Base_Stop_IT(&htim15);
			HAL_UART_Receive_IT(&huart1,&rx_data,1);
			
			if(rx_lenth != 22)
			{
				rx_error = 1;
			}
			else
			{
				rx_success = 1;
			}
			
			rx_lenth = 0;
			buff_line = 0;
			buff_col = 0;
			
			counter15 = 0;
		}
	}
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		uint32_t temp = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
		TIM2->CNT = 0;
		fre = 80000000/(80*temp);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{	
		HAL_TIM_Base_Stop_IT(&htim15);
		rx_lenth++;
		rx_buff[buff_line][buff_col++]=rx_data;
		
		if(rx_data == ':')
		{
			buff_line ++;
			buff_col = 0;
		}
		
		__HAL_TIM_SET_COUNTER(&htim15,0);
		HAL_TIM_Base_Start_IT(&htim15);
		HAL_UART_Receive_IT(&huart1,&rx_data,1);
	}
}

void Recieve_Data_Proc(char* type_str,char* carnum_str, char* time_str, time_t* time)
{
	struct tm time_struct;
	sprintf(type_str,"%c%c%c%c",rx_buff[0][0], rx_buff[0][1], rx_buff[0][2], rx_buff[0][3]);
	sprintf(carnum_str,"%c%c%c%c",rx_buff[1][0], rx_buff[1][1], rx_buff[1][2], rx_buff[1][3]);
	sprintf(time_str,"%c%c%c%c%c%c%c%c%c%c%c%c",	rx_buff[2][0], rx_buff[2][1],\
																								rx_buff[2][2], rx_buff[2][3],\
																								rx_buff[2][4], rx_buff[2][5],\
																								rx_buff[2][6], rx_buff[2][7],\
																								rx_buff[2][8], rx_buff[2][9],\
																								rx_buff[2][10], rx_buff[2][11]);
	
	//时间结构体存储规则，月份减1，年份减1900
	recYear = 2000+((rx_buff[2][0]-48)*10 + (rx_buff[2][1]-48))-1900;
	recMon = (rx_buff[2][2]-48)*10 + (rx_buff[2][3]-48) - 1;
	recDay = (rx_buff[2][4]-48)*10 + (rx_buff[2][5]-48);
	recHour = (rx_buff[2][6]-48)*10 + (rx_buff[2][7]-48);
	recMin = (rx_buff[2][8]-48)*10 + (rx_buff[2][9]-48);
	recSec = (rx_buff[2][10]-48)*10 + (rx_buff[2][11]-48);
	
	time_struct.tm_year = recYear;
	time_struct.tm_mon = recMon;
	time_struct.tm_mday = recDay;
	time_struct.tm_hour = recHour;
	time_struct.tm_min = recMin;
	time_struct.tm_sec = recSec;
	*time = mktime(&time_struct);
}

uint8_t rec_data_check(char *type_str,char* carnum_str)
{
	recMon+=1;//月份在上面为了转换减了1 这里需要加回来 recYear同理
	recYear-=100;
	if(strcmp(type_str,"CNBR")!=0 && strcmp(type_str,"VNBR")!=0)
	{
		return 1;
	}
	
	if(rx_buff[0][4] != ':' && rx_buff[1][4] != ':')
	{
		return 1;
	}
	
	if(recMon>12 || recMon<0)
	{
		return 1;
	}
	else if(recMon == 2)
	{
		if(recYear%4 == 0)//闰年
		{
			if(recDay > 28 || recDay < 0)
			{
				return 1;
			}
		}
    else
    {
      if(recDay > 29 || recDay < 0)
      {
        return 1;
      }			
    }    
	}
	else if(recMon == 1 || recMon == 3 || recMon == 5 || recMon == 7 \
											|| recMon == 8 || recMon == 10 || recMon == 12)//大月
	{
		if(recDay > 31 || recDay < 0)
		{
			return 1;
		}
	}
	else if(recMon == 2 || recMon == 4 || recMon == 6 || recMon == 11)
	{
		if(recDay > 30 || recDay < 0)
		{
			return 1;
		}		
	}
	
	if(recHour > 23 || recHour < 0)
	{
		return 1;
	}
	
	if(recMin > 59 || recMin < 0)
	{
		return 1;
	}
	
	if(recSec > 59 || recSec < 0)
	{
		return 1;
	}
	
	return 0;
	
}

void Car_Change(char *type_str,char *carnum_str, char* time_str, time_t *time)
{
	uint8_t car_dir = 1;//方向标志位 1为进 0为出
	uint8_t car_outdex = 0;//离开车辆下标

	for(int i = 0; i< 10; i++)//判断停车场是否存在该车
	{
		if(strcmp(car_struct[i].num,carnum_str)==0)//存在
		{
			car_dir = 0;//方向为出
			car_outdex = i;//记录下标
			if(*time < car_struct[car_outdex].reach)
			{
				rx_error = 1;
				return;
			}
			else if(strcmp(car_struct[car_outdex].type,type_str)!=0)
			{
				rx_error = 1;
				return;				
			}
			break;
		}
	}
	
	if(car_dir == 1)
	{
		if(stop_idle == 0)
		{
			return;
		}
		else
		{
			if(strcmp(type_str,"CNBR")==0)
			{
				stop_cnbr++;
				stop_idle--;
				car_struct[car_dex].type_price=cnbr_pay;
			}
			else if(strcmp(type_str,"VNBR")==0)
			{
				stop_vnbr++;
				stop_idle--;
				car_struct[car_dex].type_price=vnbr_pay;
			}				
		}
		strcpy(car_struct[car_dex].type,type_str);	
		strcpy(car_struct[car_dex].num,carnum_str);
		strcpy(car_struct[car_dex].reach_time,time_str);
		car_struct[car_dex].reach = *time;
		
		car_dex++;		
	}
	else if(car_dir == 0)
	{		
		if(strcmp(type_str,"CNBR")==0)
		{
			stop_cnbr--;
			stop_idle++;
		}
		else if(strcmp(type_str,"VNBR")==0)
		{
			stop_vnbr--;
			stop_idle++;
		}
		strcpy(car_struct[car_outdex].leave_time,time_str);
		car_struct[car_outdex].leave = *time;
		Settle_Acount(car_struct[car_outdex]);
		for(int i = car_outdex; i < car_dex; i++)
		{
			car_struct[i] = car_struct[i+1];
		}
		car_dex--;
	}
}

void Settle_Acount(Car_TypedefStruct car_pay)
{
	rx_success = 2;
	car_pay.dftime = difftime(car_pay.leave,car_pay.reach)/60/60;
	car_pay.dftime = car_pay.dftime == (int)car_pay.dftime ? car_pay.dftime : (int)car_pay.dftime+1;//如1.1小时算成2小时
	if(car_pay.dftime == 0)//不足一小时按一小时算
	{
		car_pay.dftime = 1.0f;
	}
	car_pay.pay_price = car_pay.dftime*car_pay.type_price;
	sprintf((char *)outcar_str,"%s:%s:%.0f:%.2f\r\n",car_pay.type,car_pay.num,car_pay.dftime,car_pay.pay_price);
}


u8 key1_state = 0;
u8 key2_state = 0;
u8 key3_state = 0;
u8 key4_state = 0;

u8 key1_last_state = 1;
u8 key2_last_state = 1;
u8 key3_last_state = 1;
u8 key4_last_state = 1;

uint32_t key1_press_time = 0;
uint32_t key2_press_time = 0;
uint32_t key3_press_time = 0;
uint32_t key4_press_time = 0;

u8 key1_mode = 0;
u8 key2_mode = 0;
u8 key3_mode = 0;
u8 key4_mode = 0;

void key_scan(void)
{
	key1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
//__________________________key1_state_______________________________//
	if(key1_state == 0)
	{
		if(key1_last_state)
		{
			key1_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key1_last_state == 0)
		{
			if(HAL_GetTick() - key1_press_time > LONG_TIME)
			{
				key1_mode = 2;
			}
			else
			{
				key1_mode = 1;
			}
		}
	}
//__________________________key2_state_______________________________//
	if(key2_state == 0)
	{
		if(key2_last_state)
		{
			key2_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key2_last_state == 0)
		{
			if(HAL_GetTick() - key2_press_time > LONG_TIME)
			{
				key2_mode = 2;
			}
			else
			{
				key2_mode = 1;
			}
		}
	}	
//__________________________key3_state_______________________________//
	if(key3_state == 0)
	{
		if(key3_last_state)
		{
			key3_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key3_last_state == 0)
		{
			if(HAL_GetTick() - key3_press_time > LONG_TIME)
			{
				key3_mode = 2;
			}
			else
			{
				key3_mode = 1;
			}
		}
	}	
//__________________________key4_state_______________________________//
	if(key4_state == 0)
	{
		if(key4_last_state)
		{
			key4_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key4_last_state == 0)
		{
			if(HAL_GetTick() - key4_press_time > LONG_TIME)
			{
				key4_mode = 2;
			}
			else
			{
				key4_mode = 1;
			}
		}
	}
	
//______________________________key_mode_________________________//
	
	if(key1_mode)
	{
		if(key1_mode==1)
		{
			part_mode++;
			part_mode %= 2;
		}
		key1_mode = 0;
	}
	
	if(key2_mode)
	{
		if(key2_mode==1&&part_mode)
		{
			cnbr_pay+=0.5f;
			vnbr_pay+=0.5f;
		}
		key2_mode = 0;
	}

	if(key3_mode)
	{
		if(key3_mode==1&&part_mode)
		{
			cnbr_pay-=0.5f;
			vnbr_pay-=0.5f;
			if(cnbr_pay<0)
			{
				cnbr_pay = 0.0f;
			}
			if(vnbr_pay<0)
			{
				vnbr_pay = 0.0f;
			}
			
		}
		key3_mode = 0;
	}
	
	if(key4_mode)
	{
		if(key4_mode==1)
		{
			pwm_mode++;
			pwm_mode %= 2;
			if(pwm_mode == 0)
			{
				__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,0);
				led2_mode = 0;
			}
			else
			{
				__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,2000*0.2);
				led2_mode = 1;
			}
		}
		key4_mode = 0;
	}	
	
	
	key1_last_state = key1_state;
	key2_last_state = key2_state;
	key3_last_state = key3_state;
	key4_last_state = key4_state;
	
	
}


