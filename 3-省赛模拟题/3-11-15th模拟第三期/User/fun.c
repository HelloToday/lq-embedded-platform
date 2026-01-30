#include "fun.h"

u16 gpio_odr = 0;

u8 lcd_text[10][40];
u8 lcd_mode = 0;

float r37_volt = 0;
float r38_volt = 0;

float r37_stand_up = 2.2;
float r37_stand_down = 1.2;
float r38_stand_up = 3.0;
float r38_stand_down = 1.4;

float r37_pass = 0;
float r38_pass = 0;

u8 param_set = 0;//0:r37up, 1:r37down, 2:r38up, 3:r38down
u8 r37_test_num = 0;
u8 r37_pass_num = 0;
u8 r38_test_num = 0;
u8 r38_pass_num = 0;

u8 led1_flag = 0;
u8 led2_flag = 0;

u32 led1_time = 0;
u32 led2_time = 0;

u8 rx_data = 0;
u8 rx_buff[10];
u8 rx_len = 0;

u8 rec_success = 0;

void led_show(u8 led, u8 mode)
{
	u16 temp = GPIOC->ODR;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	GPIOC->ODR = gpio_odr;
	
	if(mode)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_SET);
	
	gpio_odr = GPIOC->ODR;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	GPIOC->ODR = temp;	
}

void lcd_part0(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"       GOODS        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"     R37:%.2fV      ",r37_volt);
	sprintf((char*)lcd_text[4],"     R38:%.2fV      ",r38_volt);
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");	
}
void lcd_part1(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"      STANDARD      ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"    SR37:%.1f-%.1f"  ,r37_stand_down,r37_stand_up);
	sprintf((char*)lcd_text[4],"    SR38:%.1f-%.1f"  ,r38_stand_down,r38_stand_up);
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");		
}
void lcd_part2(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        PASS        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"     PR37:%.1f%%    ",r37_pass);
	sprintf((char*)lcd_text[4],"     PR38:%.1f%%    ",r38_pass);
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");		
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
	else if(lcd_mode == 2)
	{
		lcd_part2();
	}	
	//sprintf((char*)lcd_text[9],"     %d               ",lcd_mode);
	for(int i = 0; i<10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
}

void adc_get_volt(void)
{
	HAL_ADC_Start(&hadc2);
	u32 temp37 = HAL_ADC_GetValue(&hadc2);
	r37_volt = temp37 * 3.3 / 4096;
	HAL_ADC_Start(&hadc1);
	u32 temp38 = HAL_ADC_GetValue(&hadc1);
	r38_volt = temp38 * 3.3 / 4096;
}

void pass_ctrl(void)
{
	if(r37_test_num > 0)
	{
		r37_pass = ((float)r37_pass_num/r37_test_num)*100;	
	}
	else
		r37_pass = 0;
	
	if(r38_test_num > 0)
	{
		r38_pass = ((float)r38_pass_num/r38_test_num)*100;	
	}
	else
		r38_pass = 0;	
	
}

void led_mode_show(void)
{
	if(led1_flag == 1)
	{
		if(HAL_GetTick() - led1_time > 1000)
		{
			led1_flag = 0;
		}
	}
	if(led2_flag == 1)
	{
		if(HAL_GetTick() - led2_time > 1000)
		{
			led2_flag = 0;
		}
	}	
	if(lcd_mode == 0)
	{
		led_show(3,1);
		led_show(4,0);
		led_show(5,0);
	}
	else if(lcd_mode == 1)
	{
		led_show(3,0);
		led_show(4,1);
		led_show(5,0);
	}
	else if(lcd_mode == 2)
	{
		led_show(3,0);
		led_show(4,0);
		led_show(5,1);
	}
	
	led_show(1,led1_flag);
	led_show(2,led2_flag);	
}
void while_fun(void)
{
	lcd_show();
	pass_ctrl();
	led_mode_show();
	if(rec_success)
	{
		if(rec_success == 1)
		{
			printf("R37:%d,%d,%.1f%%\r\n",r37_test_num,r37_pass_num,r37_pass);
		}
		else
		{
			printf("R38:%d,%d,%.1f%%\r\n",r38_test_num,r38_pass_num,r38_pass);
		}
		rec_success = 0;
	}
}

u32 counter15 = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static u32 counter7 = 0;
	
	if(htim->Instance == TIM7)
	{
		counter7++;	
		if(counter7 % 50 == 0)
		{
			key_scan();
			//adc_get_volt();
		}
		if(counter7 % 300 == 0)
		{
			adc_get_volt();
		}
	}
	else if(htim->Instance == TIM15)
	{
		counter15++;
		if(counter15 % 10 == 0)
		{
			HAL_TIM_Base_Stop_IT(&htim15);
			counter15 = 0;
			
			if(rx_buff[0] == 'R' && rx_buff[1] == '3' && rx_buff[2] == '7' && rx_len == 3)
			{
				rec_success = 1;
			}
			else if(rx_buff[0] == 'R' && rx_buff[1] == '3' && rx_buff[2] == '8'&& rx_len == 3)
			{
				rec_success = 2;
			}
			
			rx_len = 0;
			HAL_UART_Receive_IT(&huart1,&rx_data,1);
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		HAL_TIM_Base_Stop_IT(&htim15);	
		if(rx_len < 10)
		{
			rx_buff[rx_len++] = rx_data;
		}
		counter15 = 0;
		__HAL_TIM_SET_COUNTER(&htim15,0);
		HAL_TIM_Base_Start_IT(&htim15);
		HAL_UART_Receive_IT(&huart1,&rx_data,1);
	}
}



u8 key_state[4] = {0};
u8 key_last_state[4] = {1,1,1,1};
u8 key_mode[4] = {0};
u32 key_press[4] = {0};

void key_scan(void)
{
	key_state[0] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
	key_state[1] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
	key_state[2] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
	key_state[3] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	
	for(int i = 0; i < 4; i++)
	{
		if(!key_state[i])
		{
			if(key_last_state[i] == 1)
			{
				key_press[i] = HAL_GetTick();
			}
		}
		else
		{
			if(!key_last_state[i])
			{
				if(HAL_GetTick() - key_press[i] > 1000)
				{
					key_mode[i] = 2;
				}
				else
				{
					key_mode[i] = 1;					
				}
			}
		}		
	}	
	for(int i = 0; i< 4; i++)
	{
		key_last_state[i] = key_state[i];
	}
//_________________________key1_mode___________________
	if(key_mode[0])
	{
		if(key_mode[0] == 1)
		{
			lcd_mode++;
			lcd_mode %= 3;
		}
	}
//_________________________key2_mode___________________
	if(key_mode[1])
	{
		if(key_mode[1] == 1)
		{
			if(lcd_mode == 0)
			{
				r37_test_num++;
				if(r37_volt >=r37_stand_down &&r37_volt <= r37_stand_up)
				{
					r37_pass_num++;
					led1_time = HAL_GetTick();
					led1_flag = 1;
				}
			}
			else if(lcd_mode == 1)
			{
				param_set++;
				param_set %=4;
			}
		}
	}
//_________________________key3_mode___________________
	if(key_mode[2])
	{
		if(key_mode[2] == 1)
		{
			if(lcd_mode == 0)
			{
				r38_test_num++;
				if(r38_volt >=r38_stand_down &&r38_volt <= r38_stand_up)
				{
					r38_pass_num++;
					led2_time = HAL_GetTick();
					led2_flag = 1;					
				}				
			}
			else if(lcd_mode == 1)//add
			{
				pass_add(param_set);
			}			
		}
	}
//_________________________key4_mode___________________
	if(key_mode[3])
	{
		if(key_mode[3] == 1)
		{
			if(lcd_mode == 1)//sub
			{
				pass_sub(param_set);
			}
			else if(lcd_mode == 2)
			{
				r37_pass_num = 0;
				r37_test_num = 0;	
				r38_pass_num = 0;
				r38_test_num = 0;
			}			
		}

	}
	for(int i = 0; i< 4; i++)
	{
		key_mode[i] = 0;
	}	
}

void pass_add(u8 flag)
{
	if(flag == 0)//r37 up
	{
		r37_stand_up += 0.2f;
		if(r37_stand_up > 3.1f)
		{
			r37_stand_up = 2.2f;
		}
		r37_pass_num = 0;
		r37_test_num = 0;		
	}
	else if(flag == 1)//r37 down
	{
		r37_stand_down += 0.2f;
		if(r37_stand_down > 2.1f)
		{
			r37_stand_down = 1.2f;
		}	
		r37_pass_num = 0;
		r37_test_num = 0;		
	}
	else if(flag == 2)//r38 up
	{
		r38_stand_up += 0.2f;
		if(r38_stand_up > 3.1f)
		{
			r38_stand_up = 2.2f;
		}
		r38_pass_num = 0;
		r38_test_num = 0;		
	}
	else if(flag == 3)//r38 down
	{
		r38_stand_down += 0.2f;
		if(r38_stand_down > 2.1f)
		{
			r38_stand_down = 1.2f;
		}
		r38_pass_num = 0;
		r38_test_num = 0;		
	}		
		
}

void pass_sub(u8 flag)
{
	if(flag == 0)//r37 up
	{
		r37_stand_up -= 0.2f;
		if(r37_stand_up < 2.1f)
		{
			r37_stand_up = 3.0f;
		}
		r37_pass_num = 0;
		r37_test_num = 0;		
	}
	else if(flag == 1)//r37 down
	{
		r37_stand_down -= 0.2f;
		if(r37_stand_down < 1.1f)
		{
			r37_stand_down = 2.0f;
		}	
		r37_pass_num = 0;
		r37_test_num = 0;		
	}
	else if(flag == 2)//r38 up
	{
		r38_stand_up -= 0.2f;
		if(r38_stand_up < 2.1f)
		{
			r38_stand_up = 3.0f;
		}
		r38_pass_num = 0;
		r38_test_num = 0;		
	}
	else if(flag == 3)//r38 down
	{
		r38_stand_down -= 0.2f;
		if(r38_stand_down < 1.1f)
		{
			r38_stand_down = 2.0f;
		}
		r38_pass_num = 0;
		r38_test_num = 0;		
	}		
		
}

