#include "fun.h"
u16 gpioc_odr = 0;

u8 lcd_mode = 0;
u8 lcd_text[10][40];

float temper = 0;
u8 ctrl_mode[2][8]= {"Auto","Manu"};
u8 ctrl_mode_flag = 0;
u8 gear = 1;

float r37_volt = 0;

u8 rec_data = 0;
u8 rec_buf[10];
u8 rec_lenth = 0;
u32 counter15 = 0;
u8 rec_suc_flag = 0;
u8 rec_err_flag = 0;
u8 led4_flag = 0;

u16 counter_led = 0;
u16 counter_lcd = 0;

u32 fre = 0;
float pwm_duty = 0;

void led_show(u8 led, u8 mode)
{
	u16 temp = GPIOC->ODR;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	GPIOC->ODR = gpioc_odr;
	if(mode)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_SET);
	
	gpioc_odr = GPIOC->ODR;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	GPIOC->ODR = temp;
}

void lcd_part0(void)
{
	sprintf((char *)lcd_text[0],"                      ");
	sprintf((char *)lcd_text[1],"        DATA          ");
	sprintf((char *)lcd_text[2],"                      ");
	sprintf((char *)lcd_text[3],"     TEMP:%.1f        ",temper);
	sprintf((char *)lcd_text[4],"     MODE:%s          ",ctrl_mode[ctrl_mode_flag]);
	sprintf((char *)lcd_text[5],"     GEAR:%d          ",gear);
	sprintf((char *)lcd_text[6],"                      ");
	sprintf((char *)lcd_text[7],"                      ");
	sprintf((char *)lcd_text[8],"                      ");
	sprintf((char *)lcd_text[9],"                      ");
	//sprintf((char *)lcd_text[7],"    %.1f              ",pwm_duty);
	//sprintf((char *)lcd_text[8],"    %d                ",fre);
	//sprintf((char *)lcd_text[9],"    %.2f              ",r37_volt);
}
void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"                      ");
	sprintf((char *)lcd_text[1],"                      ");
	sprintf((char *)lcd_text[2],"                      ");
	sprintf((char *)lcd_text[3],"                      ");
	sprintf((char *)lcd_text[4],"     SLEEPING         ");
	sprintf((char *)lcd_text[5],"     TEMP:%.1f        ",temper);
	sprintf((char *)lcd_text[6],"                      ");
	sprintf((char *)lcd_text[7],"                      ");
	sprintf((char *)lcd_text[8],"                      ");
	sprintf((char *)lcd_text[9],"                      ");
}
void lcd_show(void)
{
	if(lcd_mode == 0)
	{
		lcd_part0();
	}
	else if(lcd_mode == 1)
	{
		lcd_part1();
	}
	
	for(int i = 0; i<10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
}

void adc_volt_get(void)
{
	HAL_ADC_Start(&hadc2);
	uint16_t temp = HAL_ADC_GetValue(&hadc2);
	r37_volt = temp * 3.3 / 4096;
	
}

void temper_get(void)
{
	adc_volt_get();
	float temp = r37_volt;
	
	if(temp > 3.0f)
		temp = 3.0f;
	else if(temp < 1.0f)
		temp = 1.0f;
	
	temper = temp * 10 + 10;
	
}

void temper_auto_ctrl(void)
{
	if(ctrl_mode_flag == 0)
	{
		if(temper < 25.0f)
		{
			gear = 1;
		}
		else if((temper >= 25.0f) && (temper <= 30.0f))
		{
			gear = 2;
		}		
		else
		{
			gear = 3;
		}
	}
}

void uart_ctrl(void)
{
	if(rec_suc_flag)
	{
		led4_flag = 1;
		counter_led = 0;
		counter_lcd = 0;
		
		if(rec_suc_flag == 1)
		{
			if(lcd_mode == 1)
				lcd_mode = 0;
			else if(lcd_mode == 0)
			{
				ctrl_mode_flag++;
				ctrl_mode_flag %= 2;
			}		
		}
		else if(rec_suc_flag == 2)
		{
			if(lcd_mode == 1)
				lcd_mode = 0;
			else if(lcd_mode == 0  && ctrl_mode_flag == 1)
			{
				gear++;
				if(gear > 3)
					gear = 1;
			}	
		}
		else if(rec_suc_flag == 3)
		{
			if(lcd_mode == 1)
				lcd_mode = 0;	
			else if(lcd_mode == 0  && ctrl_mode_flag == 1)
			{
				gear--;
				if(gear < 1)
					gear = 3;				
			}				
		}		
		rec_suc_flag = 0;
	}
	else if(rec_err_flag)
	{
		printf("NULL\r\n");
		rec_err_flag = 0;
	}
}

void led_ctrl(void)
{
	led_show(4,led4_flag);
	
	if(ctrl_mode_flag == 0)
	{
		led_show(8,1);
	}
	else
	{
		led_show(8,0);
	}
	
	if(gear == 1)
	{
		led_show(1,1);
		led_show(2,0);
		led_show(3,0);
	}
	else if(gear == 2)
	{
		led_show(1,0);
		led_show(2,1);
		led_show(3,0);
	}	
	else if(gear == 3)
	{
		led_show(1,0);
		led_show(2,0);
		led_show(3,1);
	}		
}

void pwm_duty_ctrl(void)
{
	static u8 last_gear = 0;
	if(last_gear != gear)
	{
		if(gear == 1)
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(0.1*500)-1);
			//__HAL_TIM_SET_COUNTER(&htim2,0);
		}
		else if(gear == 2)
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(0.4*500)-1);
			//__HAL_TIM_SET_COUNTER(&htim2,0);			
		}
		else if(gear == 3)
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(0.8*500)-1);
			//__HAL_TIM_SET_COUNTER(&htim2,0);
		}
	}
	
	last_gear = gear;
}

void while_fun(void)
{
	temper_get();
	temper_auto_ctrl();
	uart_ctrl();
	led_ctrl();
	pwm_duty_ctrl();
	lcd_show();
}


//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//测试pwm频率和占空比
//{
//	static u32 tim3_ch1 = 0;
//	static u32 tim3_ch2 = 0;
//	if(htim->Instance == TIM3)
//	{
//		if(htim ->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//		{
//			tim3_ch1 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1);
//		}
//		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
//		{
//			tim3_ch2 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_2);
//			__HAL_TIM_SET_COUNTER(&htim3,0);
//			fre = 80000000/(80*(tim3_ch2));
//			pwm_duty = (tim3_ch1 + 1) * 100.0f /(float)(tim3_ch2+1);
//		}
//	}
//	
//}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static u32 counter7 = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{
			key_scan();
			
			if(led4_flag)
			{
				counter_led++;
				if(counter_led % 60 == 0)
				{
					led4_flag = 0;
					counter_led = 0;
				}
			}
			if(lcd_mode == 0)
			{
				counter_lcd++;
				if(counter_lcd % 100 == 0)
				{
					counter_lcd = 0;
					lcd_mode = 1;
				}
			}
			
		}
	}
	else if(htim->Instance == TIM15)
	{
		counter15++;
		if(counter15 % 10 == 0)
		{
			HAL_TIM_Base_Stop_IT(&htim15);
			counter15 = 0;
			if(rec_buf[0] == 'B')
			{
				if(rec_buf[1] == '1')
				{
					rec_suc_flag = 1;
				}
				else if(rec_buf[1] == '2')
				{
					rec_suc_flag = 2;
				}
				else if(rec_buf[1] == '3')
				{
					rec_suc_flag = 3;
				}
				else
				{
					rec_err_flag = 1;
				}
			}
			else
			{
				rec_err_flag = 1;
			}
			
			rec_lenth = 0;
			memset(rec_buf,0,sizeof(rec_buf));//
			
			HAL_UART_Receive_IT(&huart1, &rec_data, 1);	
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		HAL_TIM_Base_Stop_IT(&htim15);
		__HAL_TIM_SET_COUNTER(&htim15,0);
		counter15 = 0;
		
		if(rec_lenth < 10)
			rec_buf[rec_lenth++] = rec_data;
		
		HAL_TIM_Base_Start_IT(&htim15);
		HAL_UART_Receive_IT(&huart1, &rec_data, 1);		
	}
}

u8 key_state[4] = {0};
u8 key_last_state[4] = {1,1,1,1};
u32 key_press_time[4] = {0};
u8 key_mode[4] = {0};
u8 key_step[4] = {0};

void key_scan(void)
{
	key_state[0] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key_state[1] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key_state[2] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key_state[3] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
	
	for(int i = 0; i < 4; i++)
	{
		switch(key_step[i])
		{
			case 0:
			{
				if(key_state[i] == 0 && key_last_state[i] == 1)
				{
					key_press_time[i] = HAL_GetTick();
					key_step[i] = 1;
				}
			}break;
			case 1:
			{
				if(key_state[i] == 1 && key_last_state[i] == 1 && (HAL_GetTick() - key_press_time[i] > 1000))
				{
					key_mode[i] = 2;
					key_step[i] = 0;
				}
				else if(key_state[i] == 1 && key_last_state[i] == 1)
				{
					key_mode[i] = 1;
					key_step[i] = 0;					
				}
			}break;
		}
	}
	
//__________________________key1_mode______________________________//
	if(key_mode[0])
	{
		if(key_mode[0] == 1)
		{
			counter_lcd = 0;
			if(lcd_mode == 0)
			{
				ctrl_mode_flag++;
				ctrl_mode_flag %= 2;
			}
			else if(lcd_mode == 1)
			{
				lcd_mode = 0;
			}	
		}
		key_mode[0] = 0;
	}
//__________________________key2_mode______________________________//
	if(key_mode[1])
	{
		if(key_mode[1] == 1)
		{
			counter_lcd = 0;
			if(lcd_mode == 0  && ctrl_mode_flag == 1)
			{
				gear++;
				if(gear > 3)
					gear = 1;
			}	
			else if(lcd_mode == 1)
			{
				lcd_mode = 0;
			}			
		}

		key_mode[1] = 0;
	}
//__________________________key3_mode______________________________//
	if(key_mode[2])
	{
		if(key_mode[2] == 1)
		{
			counter_lcd = 0;
			if(lcd_mode == 0  && ctrl_mode_flag == 1)
			{
				gear--;
				if(gear < 1)
					gear = 3;
			}
			else if(lcd_mode == 1)
			{
				lcd_mode = 0;
			}						
		}
		key_mode[2] = 0;
	}
//__________________________key4_mode______________________________//
	if(key_mode[3])
	{
		if(key_mode[3] == 1)
		{
			
		}
		key_mode[3] = 0;
	}	
}



