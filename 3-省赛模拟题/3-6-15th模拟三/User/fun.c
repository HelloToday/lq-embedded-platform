#include "fun.h"

u16 gpioc_odr = 0;

u8 lcd_text[10][40];
u8 lcd_mode = 0;

u8 pri = 0;
u8 pri_ctrl = 0;
u16 tot = 0;
float r38_volt = 0;
float change_volt = 0;

u8 price_arr1 [7] = {5,10,15,20,30,35,40};
u8 price_arr2 [7] = {40,35,25,20,15,10,5};

s8 price_cnt1 = 0;
s8 price_cnt2 = 0;

u8 pri_change = 0;


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
	sprintf((char *)lcd_text[0],"                     ");
	sprintf((char *)lcd_text[1],"        DATA         ");
	sprintf((char *)lcd_text[2],"                     ");
	sprintf((char *)lcd_text[3],"       Pri:%d        ",pri);
	sprintf((char *)lcd_text[4],"       Wei:%.1f      ",change_volt);
	sprintf((char *)lcd_text[5],"       Tot:%d        ",tot);
	sprintf((char *)lcd_text[6],"                     ");
	sprintf((char *)lcd_text[7],"                     ");
	sprintf((char *)lcd_text[8],"                     ");
	sprintf((char *)lcd_text[9],"                     ");
}
void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"                     ");
	sprintf((char *)lcd_text[1],"       RRICE         ");
	sprintf((char *)lcd_text[2],"                     ");
	sprintf((char *)lcd_text[3],"        %d           ",pri_ctrl);
	sprintf((char *)lcd_text[4],"                     ");
	sprintf((char *)lcd_text[5],"                     ");
	sprintf((char *)lcd_text[6],"                     ");
	sprintf((char *)lcd_text[7],"                     ");
	sprintf((char *)lcd_text[8],"                     ");
	sprintf((char *)lcd_text[9],"                     ");
}
void lcd_show(void)
{
	if(lcd_mode == 0)
	{
		lcd_part0();
	}
	else
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
	HAL_ADC_Start(&hadc1);
	u16 temp = HAL_ADC_GetValue(&hadc1);
	r38_volt = temp * 3.3 /4096;
	
	change_volt = (int)(r38_volt*10)/10.0;
	tot = pri * change_volt;
}

void while_fun(void)
{
	lcd_show();
	if(pri_change)
	{
		i2c_write(0,pri_ctrl);
		pri_change = 0;
	}
	if(lcd_mode == 0)
	{
		led_show(1,1);
		led_show(2,0);
	}
	else if(lcd_mode == 1)
	{
		led_show(1,0);
		led_show(2,1);
	}
}

void my_init(void)
{
	pri_ctrl = i2c_read(0);
	pri = pri_ctrl;
	for(int i = 0; i<7; i++)
	{
		if(price_arr2[i] == pri_ctrl)
		{
			price_cnt2 = i;
			break;
		}
	}
	for(int i = 0; i<7; i++)
	{
		if(price_arr1[i] == pri_ctrl)
		{
			price_cnt1 = i;
			break;
		}
	}	
	
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
			
		}
		if(counter7 % 100 == 0)
		{
			adc_volt_get();
		}
	}
}


u8 key_state[4]= {0};
u8 key_last_state[4]= {1,1,1,1};
u32 key_press[4]= {0};
u8 key_mode[4]= {0};

void key_scan(void)
{
	key_state[0] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
	key_state[1] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
	key_state[2] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
	key_state[3] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	
	for(int i = 0; i<4; i++)
	{
		if(key_state[i] == 0)
		{
			if(key_last_state[i] == 1)
			{
				key_press[i] = HAL_GetTick();
			}
		}
		else
		{
			if(key_last_state[i] == 0)
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
	key_last_state[0]=key_state[0];
	key_last_state[1]=key_state[1];
	key_last_state[2]=key_state[2];
	key_last_state[3]=key_state[3];
	
//_____________key1_mode__________________//
	if(key_mode[0])
	{
		if(key_mode[0] == 1)
		{
			lcd_mode++;
			lcd_mode %= 2; 
			if(lcd_mode == 0)
			{
				if(pri_ctrl != pri)
				{
					pri = pri_ctrl;
					pri_change = 1;
				}
			}
		}
		key_mode[0] = 0;
	}
//_____________key2_mode__________________//
	if(key_mode[1])
	{
		if(key_mode[1] == 1)
		{
			if(lcd_mode == 1)
			{
				price_cnt1++;
				if(price_cnt1 > 6)
				{
					price_cnt1 = 0;
				}
				pri_ctrl = price_arr1[price_cnt1];
				for(int i = 0; i<7; i++)
				{
					if(price_arr2[i] == pri_ctrl)
					{
						price_cnt2 = i;
						break;
					}
				}
			}
		}
		key_mode[1] = 0;
	}	
//_____________key3_mode__________________//
	if(key_mode[2])
	{
		if(key_mode[2] == 1)
		{
			if(lcd_mode == 1)
			{
				price_cnt2++;
				if(price_cnt2 > 6)
				{
					price_cnt2 = 0;
				}
				pri_ctrl = price_arr2[price_cnt2];
				for(int i = 0; i<7; i++)
				{
					if(price_arr1[i] == pri_ctrl)
					{
						price_cnt1 = i;
						break;
					}
				}				
			}			
		}
		key_mode[2] = 0;
	}	
//_____________key4_mode__________________//
	if(key_mode[3])
	{
		if(key_mode[3] == 1)
		{
			
		}
		key_mode[3] = 0;
	}

}

