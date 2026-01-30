#include "headfile.h"

uint8_t high_left= 7, high_right = 9;
uint8_t high_site = 1;
int8_t rom_time[5][6]={0};
uint8_t work_mode = 0;
uint8_t work_state[4][8] ={"Standby","Setting","Running","Pause"};
uint8_t led_flag = 0;
uint32_t fre,cap_value;


void led_show(uint8_t led, uint8_t mode)
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	if(mode)
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_SET);
	}
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

uint8_t lcd_text[10][21];
uint8_t rom_mode = 0;

void lcd_show(void)
{
	for(int i = 0; i < 10; i++)
	{
		if(i != 2)
			LCD_DisplayStringLine(i*24, lcd_text[i]);
	}
	led_show(1,led_flag);
}

void highlight_show(uint8_t line, uint8_t begin, uint8_t end,uint8_t* str)
{
	LCD_SetBackColor(Black);
	for(int i = 0; i< begin; i++)
	{
		LCD_DisplayChar(line,320 - 16*i,str[i]);
	}
	LCD_SetBackColor(Blue);
	for(int i = begin; i< end; i++)
	{
		LCD_DisplayChar(line,320 - 16*i,str[i]);
	}	
	LCD_SetBackColor(Black);
	for(int i = end; i< 20; i++)
	{
		LCD_DisplayChar(line,320 - 16*i,str[i]);
	}	
}


void lcd_sub(void)
{
	sprintf((char *)lcd_text[0], "       NO : %d       ",rom_mode+1);
	sprintf((char *)lcd_text[2], "       %d%d: %d%d: %d%d         ",
																rom_time[rom_mode][0],rom_time[rom_mode][1],\
																rom_time[rom_mode][2],rom_time[rom_mode][3],\
																rom_time[rom_mode][4],rom_time[rom_mode][5]);
	highlight_show(Line2,high_left,high_right,lcd_text[2]);
	
	sprintf((char *)lcd_text[4], "        %s       ",work_state[work_mode]);
	
	sprintf((char *)lcd_text[6], "       fre:%d       ",fre);
		
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t couter7 = 0;
	if(htim->Instance == TIM7)
	{
		couter7++;
		
		if(work_mode == 2)
		{
			HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
		}
		else
		{
			HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_2);
			fre = 0;
		}
		
		if(couter7 % 500 == 0)
		{
			if(work_mode == 2)
			{
				led_flag++;
				led_flag %= 2;				
			}
			else
			{
				led_flag = 0;
			}

		}
		if(couter7 >= 1000)
		{
			if(work_mode == 2)
			{
				rom_time[rom_mode][5]--;
			}
			couter7 = 0;
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		cap_value = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);
		TIM2->CNT = 0;
		fre = 80000000/(10 * cap_value);
	}
}

uint8_t key1_state = 0;
uint8_t key2_state = 0;
uint8_t key3_state = 0;
uint8_t key4_state = 0;

uint8_t key1_last_state = 1;
uint8_t key2_last_state = 1;
uint8_t key3_last_state = 1;
uint8_t key4_last_state = 1;

uint32_t key1_press_time = 0;
uint32_t key2_press_time = 0;
uint32_t key3_press_time = 0;
uint32_t key4_press_time = 0;

uint8_t key1_mode = 0;
uint8_t key2_mode = 0;
uint8_t key3_mode = 0;
uint8_t key4_mode = 0;

#define LONG_TIME 800

void key_scan(void)
{
	static uint8_t counter = 0;
	key1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
	
	//key1_state
	if(0 == key1_state)
	{
		if(key1_last_state)//首次按下
		{
			key1_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(0 == key1_last_state)//松手
		{
			if(HAL_GetTick() - key1_press_time >= LONG_TIME)
			{
				key1_mode = 2;
			}
			else
			{
				key1_mode = 1;
			}
		}
	}
	//key2_state
	if(0 == key2_state)
	{
		if(key2_last_state)//首次按下
		{
			key2_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(0 == key2_last_state)//松手
		{
			if(HAL_GetTick() - key2_press_time >= LONG_TIME)
			{
				key2_mode = 2;
			}
			else
			{
				key2_mode = 1;
			}
		}
	}	
	//key3_state
	if(0 == key3_state)
	{
		if(key3_last_state)//首次按下
		{
			key3_press_time = HAL_GetTick();
		}
		else if(HAL_GetTick() - key3_press_time >= LONG_TIME)
		{
			key3_mode = 3;
		}
	}
	else
	{
		if(0 == key3_last_state)//松手
		{
			if(HAL_GetTick() - key3_press_time >= LONG_TIME)
			{
				key3_mode = 2;
			}
			else
			{
				key3_mode = 1;
			}
		}
	}	
	//key4_state
	if(0 == key4_state)
	{
		if(key4_last_state)//首次按下
		{
			key4_press_time = HAL_GetTick();
		}
	}
	else
	{
		if(0 == key4_last_state)//松手
		{
			if(HAL_GetTick() - key4_press_time >= LONG_TIME)
			{
				key4_mode = 2;
			}
			else
			{
				key4_mode = 1;
			}
		}
	}	

//_______________key_mode___________________________//
	//key1_mode
	if(key1_mode)
	{
		if(1 == key1_mode)
		{
			rom_mode++;
			if(rom_mode > 4)
			{
				rom_mode = 0;
			}
			for(int i = 0; i < 6; i++)
			{
				rom_time[rom_mode][i] = rom_read(rom_mode*6 + i);
			}						
		}
		else if(2 == key1_mode)
		{
			//rom_mode = 2;
		}
		key1_mode = 0;
	}
	//key2_mode
	if(key2_mode)
	{
		work_mode = 1;
		
		if(1 == key2_mode)
		{
			high_site++;
			if(high_site > 3)
			{
				high_site = 1;
			}
			
			if(1 == high_site)
			{
				high_left = 7;
				high_right = 9;
			}
			else if(2 == high_site)
			{
				high_left = 11;
				high_right = 13;				
			}
			else if(3 == high_site)
			{
				high_left = 15;
				high_right = 17;				
			}				
		}
		else if(2 == key2_mode)
		{		
			for(int i = 0; i<6; i++)
			{
				rom_write(6*rom_mode+i,rom_time[rom_mode][i]);
			}
		}
		key2_mode = 0;
	}
	//key3_mode
	if(key3_mode)
	{
		work_mode = 1;
		if(1 == key3_mode)
		{
			if(1 == high_site)
			{
				rom_time[rom_mode][1]++;
			}
			else if(2 == high_site)
			{
				rom_time[rom_mode][3]++;
			}
			else if(3 == high_site)
			{
				rom_time[rom_mode][5]++;
			}			
		}
		else if(2 == key3_mode)
		{
			//rom_mode = 6;
		}
		else if(3 == key3_mode)
		{
			counter++;
			if(counter >= 5)
			{
				counter = 0;
				if(1 == high_site)
				{
					rom_time[rom_mode][1]++;
				}
				else if(2 == high_site)
				{
					rom_time[rom_mode][3]++;
				}
				else if(3 == high_site)
				{
					rom_time[rom_mode][5]++;
				}					
			}
		}
		key3_mode = 0;
	}
	//key4_mode
	if(key4_mode)
	{
		if(1 == key4_mode)
		{
			if(work_mode == 0)
			{
				work_mode = 2;
			}
			else if(work_mode == 2)
			{
				work_mode = 3;
			}
			else if(work_mode != 2)
			{
				work_mode = 2;
			}			
			//rom_mode = 7;
		}
		else if(2 == key4_mode)
		{
			work_mode = 0;
		}
		key4_mode = 0;
	}
	
	//计时进位
	if(rom_time[rom_mode][0]>1 && rom_time[rom_mode][1]>3)
	{
		rom_time[rom_mode][0] = 0;
		rom_time[rom_mode][1] = 0;
	}
	else if(rom_time[rom_mode][1]>9)
	{
		rom_time[rom_mode][1] = 0;
		rom_time[rom_mode][0]++;
	}	
	else if(rom_time[rom_mode][2]>5)
	{
		rom_time[rom_mode][2] = 0;
		rom_time[rom_mode][1]++;
	}		
	else if(rom_time[rom_mode][3]>9)
	{
		rom_time[rom_mode][3] = 0;
		rom_time[rom_mode][2]++;
	}	
	else if(rom_time[rom_mode][4]>5)
	{
		rom_time[rom_mode][4] = 0;
		rom_time[rom_mode][3]++;
	}	
	else if(rom_time[rom_mode][5]>9)
	{
		rom_time[rom_mode][5] = 0;
		rom_time[rom_mode][4]++;
	}
	//计时借位
	if(rom_time[rom_mode][5] < 0)
	{
		rom_time[rom_mode][5] = 9;
		rom_time[rom_mode][4] --;
	}	
	if(rom_time[rom_mode][4] < 0)
	{
		rom_time[rom_mode][4] = 5;
		rom_time[rom_mode][3] --;
	}	
	if(rom_time[rom_mode][3] < 0)
	{
		rom_time[rom_mode][4] = 9;
		rom_time[rom_mode][2] --;
	}	
	if(rom_time[rom_mode][2] < 0)
	{
		rom_time[rom_mode][2] = 5;
		rom_time[rom_mode][1] --;
	}
	if(rom_time[rom_mode][1] < 0)
	{
		rom_time[rom_mode][1] = 9;
		rom_time[rom_mode][0] --;
	}		
	if(rom_time[rom_mode][0] < 0 && rom_time[rom_mode][1] == 0 &&\
		 rom_time[rom_mode][2] == 0 && rom_time[rom_mode][3] == 0 &&\
		 rom_time[rom_mode][4] == 0 && rom_time[rom_mode][5] == 0)
	{
		 
	}	
	key1_last_state =	key1_state;
	key2_last_state =	key2_state;
  key3_last_state =	key3_state;
  key4_last_state =	key4_state;
	
}	
