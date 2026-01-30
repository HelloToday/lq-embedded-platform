#include "fun.h"

u16 gpioc_odr = 0;
u8 lcd_mode = 0;
u8 lcd_text[10][40];
u8 test = 0;

float r37_volt = 0, r38_volt = 0, mp_volt = 0;

u16 adc1_cap[4];
u16 adc2_cap[2];

void led_show(uint8_t led, uint8_t mode)
{
	uint16_t temp = GPIOC->ODR;
	GPIOC->ODR = gpioc_odr;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	
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
	sprintf((char*)lcd_text[0],"test:%d",test);
	sprintf((char*)lcd_text[1],"r37:%.2f     ",r37_volt);
	sprintf((char*)lcd_text[2],"r38:%.2f     ",r38_volt);
	sprintf((char*)lcd_text[3],"mov:%.2f     ",mp_volt);
	sprintf((char*)lcd_text[4],"                    ");
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"9                   ");
	
}

void lcd_show(void)
{
	if(lcd_mode == 0)
	{
		lcd_part0();
	}
	
	for(int i = 0; i < 10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);	
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static u32 counter7 = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)//50ms
		{
			key_scan();
			volt_adc_get();
		}
	}
}

void volt_adc_get(void)
{
	u16 temp1 = 0,temp2 = 0,temp3 = 0;
	
//	HAL_ADC_Start(&hadc1);
//	temp1 = HAL_ADC_GetValue(&hadc1);
//	mp_volt = temp1 * 3.3 / 4096;
//	
//	HAL_ADC_Start(&hadc1);
//	temp2 = HAL_ADC_GetValue(&hadc1);
//	r38_volt = temp2 * 3.3 / 4096;
//	
//	HAL_ADC_Start(&hadc2);
//	temp3 = HAL_ADC_GetValue(&hadc2);
//	r37_volt = temp3 * 3.3 / 4096;
	mp_volt = ((adc1_cap[0]+adc1_cap[2])/2.0)*3.3/4096;
	r38_volt = ((adc1_cap[1]+adc1_cap[3])/2.0)*3.3/4096;
	r37_volt = ((adc2_cap[0]+adc2_cap[1])/2.0)*3.3/4096;
}

void while_fun(void)
{
	lcd_show();
}

u8 key_state[4] = {0};
u8 key_last_state[4] = {1,1,1,1};
u32 key_press_time[4] = {0};
u8 key_mode[4] = {0};
#define LONG_TIME 1000
void key_scan(void)
{
	key_state[0] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
	key_state[1] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
	key_state[2] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
	key_state[3] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	
	for(int i = 0; i < 4; i++)
	{
		if(key_state[i] == 0)
		{
			if(key_last_state[i] == 1)
			{
				key_press_time[i] = HAL_GetTick();
			}
		}
		else
		{
			if(key_last_state[i] == 0)
			{
				if(HAL_GetTick() - key_press_time[i] > LONG_TIME)
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
//__________________________key1_mode____________________//
	if(key_mode[0])
	{
		if(key_mode[0] == 1)
		{
			
		}
		else if(key_mode[0] == 2)
		{
			//test++;
		}
		key_mode[0] = 0;
	}
//__________________________key2_mode____________________//
	if(key_mode[1])
	{
		if(key_mode[1] == 1)
		{
			
		}
		else if(key_mode[1] == 2)
		{
			
		}
		key_mode[1] = 0;
	}	
//__________________________key3_mode____________________//
	if(key_mode[2])
	{
		if(key_mode[2] == 1)
		{
			
		}
		else if(key_mode[2] == 2)
		{
			
		}
		key_mode[2] = 0;
	}
//__________________________key4_mode____________________//
	if(key_mode[3])
	{
		if(key_mode[3] == 1)
		{
			
		}
		else if(key_mode[3] == 2)
		{
			
		}
		key_mode[3] = 0;
	}
	
	key_last_state[0] = key_state[0];
  key_last_state[1] = key_state[1];
  key_last_state[2] = key_state[2];
  key_last_state[3] = key_state[3];
	
	
}

