#include "fun.h"

u8 part_mode = 0;
u8 lcd_text[10][20];
float cap_volt = 0.0f;
u8 mea_state[2][8]={"AUTO","MANU"};
u8 mea_mode = 0;
u32 fre = 0;
u8 pwm6_set = 10;
u8 pwm7_set = 10;

void led_show(u8 led, u8 mode)
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	if(mode)
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"      Data              ");
	sprintf((char *)lcd_text[1],"                        ");
	sprintf((char *)lcd_text[2],"    V:%.2fV             ",cap_volt);
	sprintf((char *)lcd_text[3],"                        ");
	sprintf((char *)lcd_text[4],"    Mode:%s             ",mea_state[mea_mode]);
	sprintf((char *)lcd_text[5],"                        ");
	sprintf((char *)lcd_text[6],"                        ");
	sprintf((char *)lcd_text[7],"                        ");
	sprintf((char *)lcd_text[8],"                        ");
	sprintf((char *)lcd_text[9],"                        ");
}
void lcd_part2(void)
{
	sprintf((char *)lcd_text[0],"      Para              ");
	sprintf((char *)lcd_text[1],"                        ");
	sprintf((char *)lcd_text[2],"    PA6:%d%%             ",pwm6_set);
	sprintf((char *)lcd_text[3],"                        ");
	sprintf((char *)lcd_text[4],"    PA7:%d%%             ",pwm7_set);
	sprintf((char *)lcd_text[5],"                        ");
	sprintf((char *)lcd_text[6],"                        ");
	sprintf((char *)lcd_text[7],"                        ");
	sprintf((char *)lcd_text[8],"                        ");
	sprintf((char *)lcd_text[9],"                        ");	
}

void lcd_show(void)
{
	if(part_mode == 0)
	{
		lcd_part1();
		led_show(2,1);
	}
	else if(part_mode == 1)
	{
		lcd_part2();
		led_show(2,0);
	}
	
	for(int i = 0;i<10;i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
	
	if(mea_mode == 0)
	{
		led_show(1,1);
	}
	else
	{
		led_show(1,0);
	}	
}

double get_adc(void)
{
	HAL_ADC_Start_IT(&hadc2);
	u32 cap_value = HAL_ADC_GetValue(&hadc2);
	return cap_value*3.3/4096;

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static u32 counter7 = 0; 
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{
			key_scan();
			
			cap_volt = get_adc();
			if(part_mode == 0)
			{
				__HAL_TIM_SetCompare(&htim16,TIM_CHANNEL_1,100*cap_volt/3.3f);
				__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,200*cap_volt/3.3f);				
			}
			else if(part_mode == 1)
			{
				__HAL_TIM_SetCompare(&htim16,TIM_CHANNEL_1,100*(float)pwm6_set/100);
				__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,200*(float)pwm7_set/100);	
			}
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		u32 temp = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);
		TIM2->CNT = 0;
		fre = 80000000/(80*temp);
	}
}


u8 key1_state = 0;
u8 key2_state = 0;
u8 key3_state = 0;
u8 key4_state = 0;

u8 key1_last_state = 1;
u8 key2_last_state = 1;
u8 key3_last_state = 1;
u8 key4_last_state = 1;

u32 key1_press_time = 0;
u32 key2_press_time = 0;
u32 key3_press_time = 0;
u32 key4_press_time = 0;

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
//______________________key1_state___________________//	
	if(key1_state == 0)
	{
		if(key1_last_state == 1)
		{
			key1_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key1_last_state == 0)
		{
			if(HAL_GetTick()-key1_press_time >=LONG_TIME)
			{
				key1_mode = 2;
			}
			else
			{
				key1_mode = 1;
			}
		}
	}
//______________________key2_state___________________//	
	if(key2_state == 0)
	{
		if(key2_last_state == 1)
		{
			key2_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key2_last_state == 0)
		{
			if(HAL_GetTick()-key2_press_time >=LONG_TIME)
			{
				key2_mode = 2;
			}
			else
			{
				key2_mode = 1;
			}
		}
	}	
//______________________key3_state___________________//	
	if(key3_state == 0)
	{
		if(key3_last_state == 1)
		{
			key3_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key3_last_state == 0)
		{
			if(HAL_GetTick()-key3_press_time >=LONG_TIME)
			{
				key3_mode = 2;
			}
			else
			{
				key3_mode = 1;
			}
		}
	}
//______________________key4_state___________________//	
	if(key4_state == 0)
	{
		if(key4_last_state == 1)
		{
			key4_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(key4_last_state == 0)
		{
			if(HAL_GetTick()-key4_press_time >=LONG_TIME)
			{
				key4_mode = 2;
			}
			else
			{
				key4_mode = 1;
			}
		}
	}	
	
//________________________________key_mode_____________________________//
	if(key1_mode)
	{
		if(key1_mode == 1)
		{
			part_mode++;
			part_mode %= 2;		
		}
		key1_mode = 0;
	}
	if(key2_mode)
	{
		if(key2_mode == 1 && part_mode == 1)
		{
			pwm6_set+=10;
			if(pwm6_set >90)
			{
				pwm6_set = 10;
			}
		}
		key2_mode = 0;
	}
	if(key3_mode)
	{
		if(key3_mode == 1 && part_mode == 1)
		{
			pwm7_set+=10;
			if(pwm7_set >90)
			{
				pwm7_set = 10;
			}
		}
		key3_mode = 0;
	}
	if(key4_mode)
	{
		if(key4_mode == 1)
		{
			//cap_volt = 4;
			mea_mode++;
			mea_mode %= 2;
		}
		key4_mode = 0;
	}	

	
	key1_last_state = key1_state;
	key2_last_state = key2_state;
  key3_last_state = key3_state;
  key4_last_state = key4_state;
}
