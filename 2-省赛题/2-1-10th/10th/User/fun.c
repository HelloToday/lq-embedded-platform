#include "fun.h"
u8 lcd_text[10][21];
float cap_volt=0;

u8 test= 0;
u8 part_mode = 0;
float set_maxvolt=2.4;
float set_minvolt=1.2;
u8 up_led = 1;
u8 low_led = 2;
u8 highline = 3;
u8 volt_state [3][8]={"Upper","Lower","Normal"};
u8 volt_mode = 0;
u8 led_tw = 0;

void led_show(u8 led, u8 mode)
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	if(mode)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

double adc_cap(void)
{
	HAL_ADC_Start(&hadc2);
	uint32_t temp = HAL_ADC_GetValue(&hadc2);
	return temp *3.3 / 4096;
}

void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"        Main        ");
	sprintf((char *)lcd_text[2],"                    ");
	sprintf((char *)lcd_text[3],"   Volt: %.2fV         ",cap_volt);
	sprintf((char *)lcd_text[4],"                    ");
	sprintf((char *)lcd_text[5],"   Status: %s          ",volt_state[volt_mode]);
	sprintf((char *)lcd_text[6],"                    ");
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");
	
}

void lcd_highline(u8 line)
{
	LCD_SetBackColor(Black);
	for(int i = 2;i<line;i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
	LCD_SetBackColor(Green);
	
	LCD_DisplayStringLine(line*24,lcd_text[line]);
	
	LCD_SetBackColor(Black);
	for(int i = line+1;i<7;i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}	
	
}
void lcd_part2(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"       Setting        ");
	sprintf((char *)lcd_text[2],"                    ");
	sprintf((char *)lcd_text[3],"   Max Volt: %.1fV         ",set_maxvolt);
	sprintf((char *)lcd_text[4],"   Min Volt: %.1fV         ",set_minvolt);
	sprintf((char *)lcd_text[5],"   Upper: LD%d             ",up_led);
	sprintf((char *)lcd_text[6],"   Lower: LD%d             ",low_led);
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");
}

void lcd_show(void)
{
	if(part_mode == 0)
	{
		lcd_part1();
		for(int i = 0; i<10; i++)
		{
			LCD_DisplayStringLine(i*24,lcd_text[i]);
		}
	}
	else if(part_mode == 1)
	{
		lcd_part2();
		lcd_highline(highline);
		for(int i = 0; i<10; i++)
		{
			if(i != 2 && i != 3 && i != 4 && i != 5 && i != 6)
				LCD_DisplayStringLine(i*24,lcd_text[i]);
		}		
	}
	
	if(cap_volt > set_maxvolt)
	{
		volt_mode = 0;
		for(int i = 1; i<9;i++)
		{
			if(i != up_led && i != low_led) 
				led_show(i,0);
		}
		led_show(up_led,led_tw);
		led_show(low_led,0);
	}
	else if(cap_volt < set_minvolt)
	{
		volt_mode = 1;
		for(int i = 1; i<9;i++)
		{
			if(i != up_led && i != low_led) 
				led_show(i,0);
			led_show(i,0);
		}		
		led_show(up_led,0);
		led_show(low_led,led_tw);
	}
	else if(cap_volt <= set_maxvolt && cap_volt >= set_minvolt)
	{
		volt_mode = 2;
		for(int i = 1; i<9;i++)
		{
			if(i != up_led && i != low_led) 
				led_show(i,0);
			led_show(i,0);
		}		
		led_show(up_led,0);
		led_show(low_led,0);		
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t counter7 = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 5 == 0)
		{
			cap_volt = adc_cap();
		}
		if(counter7 % 200 == 0)
		{
			counter7 = 0;
			led_tw++;
			led_tw %= 2;
		}
	}
}


u8 key_state[4]={0};
u8 key_last_state[4] = {1,1,1,1};
uint32_t key_press_time[4] = {0};
u8 key_mode[4]={0};

void key_scan(void)
{
	
	key_state[0] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key_state[1] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key_state[2] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key_state[3] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
//_________________Key1_state____________________//
	if(key_state[0] == 0)
	{
		if(key_last_state[0])
		{
			key_press_time[0] = HAL_GetTick();
		}
	}
	else
	{
		if(key_last_state[0] == 0)
		{
			if(HAL_GetTick() - key_press_time[0] >= LONG_TIME)
			{
				key_mode[0] = 2;
			}
			else
			{
				key_mode[0] = 1;
			}
		}
	}
	
//_________________Key2_state____________________//
	if(key_state[1] == 0)
	{
		if(key_last_state[1])
		{
			key_press_time[1] = HAL_GetTick();
		}
	}
	else
	{
		if(key_last_state[1] == 0)
		{
			if(HAL_GetTick() - key_press_time[1] >= LONG_TIME)
			{
				key_mode[1] = 2;
			}
			else
			{
				key_mode[1] = 1;
			}
		}
	}	
//_________________Key3_state____________________//
	if(key_state[2] == 0)
	{
		if(key_last_state[2])
		{
			key_press_time[2] = HAL_GetTick();
		}
	}
	else
	{
		if(key_last_state[2] == 0)
		{
			if(HAL_GetTick() - key_press_time[2] >= LONG_TIME)
			{
				key_mode[2] = 2;
			}
			else
			{
				key_mode[2] = 1;
			}
		}
	}	
//_________________Key4_state____________________//
	if(key_state[3] == 0)
	{
		if(key_last_state[3])
		{
			key_press_time[3] = HAL_GetTick();
		}
	}
	else
	{
		if(key_last_state[3] == 0)
		{
			if(HAL_GetTick() - key_press_time[3] >= LONG_TIME)
			{
				key_mode[3] = 2;
			}
			else
			{
				key_mode[3] = 1;
			}
		}
	}
	
//____________________________key1_mode___________________//
	if(key_mode[0])
	{
		if(key_mode[0]== 1)
		{
			//test = 1;
			part_mode++;
			part_mode = part_mode % 2;
			highline = 3;
		}
		key_mode[0] = 0;
	}
//____________________________key2_mode___________________//
	if(key_mode[1])
	{
		if(key_mode[1]== 1&& part_mode)
		{
			//test = 2;
			highline++;
			if(highline > 6)
				highline = 3;
		}
		key_mode[1] = 0;
	}	
//____________________________key3_mode___________________//
	if(key_mode[2])
	{
		if(key_mode[2]== 1 && part_mode)
		{
			//test = 3;
			if(highline == 3)
			{
				set_maxvolt += 0.3f;
				set_maxvolt = set_maxvolt < 3.3f ? set_maxvolt : 3.3f;
			}
			else if(highline == 4)
			{
				set_minvolt += 0.3f;
				set_minvolt = set_minvolt < set_maxvolt ? set_minvolt : set_maxvolt;
			}
			else if(highline == 5)
			{
				up_led++;
				if(up_led == low_led)
				{
					up_led++;
				}
				if(up_led > 8)
				{
					up_led = 1;
				}
				if(up_led == low_led)
				{
					up_led++;
				}
			}
			else if(highline == 6)
			{
				low_led++;
				if(up_led == low_led)
				{
					low_led++;
				}
				if(low_led > 8)
				{
					low_led = 1;
				}	
				if(up_led == low_led)
				{
					low_led++;
				}				
			}
		}
		key_mode[2] = 0;
	}
//____________________________key4_mode___________________//
	if(key_mode[3])
	{
		if(key_mode[3]== 1&& part_mode)
		{
			//test = 4;
			if(highline == 3)
			{
				set_maxvolt -= 0.3f;
				set_maxvolt = set_maxvolt > set_minvolt ? set_maxvolt :set_minvolt;
				if(set_maxvolt < 0)
				{
					set_maxvolt = 0;
				}				
			}
			else if(highline == 4)
			{
				set_minvolt -= 0.3f;
				set_minvolt = set_minvolt < set_maxvolt ? set_minvolt : set_maxvolt;
				if(set_minvolt < 0)
				{
					set_minvolt = 0;
				}
			}
			else if(highline == 5)
			{
				up_led--;
				if(up_led == low_led)
				{
					up_led--;
				}
				if(up_led < 1)
				{
					up_led = 8;
				}
				if(up_led == low_led)
				{
					up_led--;
				}
			}
			else if(highline == 6)
			{
				low_led--;
				if(up_led == low_led)
				{
					low_led--;
				}
				if(low_led < 1)
				{
					low_led = 8;
				}
				if(up_led == low_led)
				{
					low_led--;
				}				
			}
		}
		key_mode[3] = 0;
	}
	
	
	
	
	key_last_state[0]=key_state[0];
	key_last_state[1]=key_state[1];
	key_last_state[2]=key_state[2];
	key_last_state[3]=key_state[3];
	
}
