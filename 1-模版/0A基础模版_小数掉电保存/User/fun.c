#include "fun.h"

u16 gpioc_odr = 0;
u8 lcd_mode = 0;
u8 lcd_text[10][40];
u8 test = 0;

double float_num = 24.12;
u8 ep_num1 = 0;
u8 ep_num2 = 0;

u16 u16_num = 5421;
u16 u16_num_disp = 0;


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
	sprintf((char*)lcd_text[1]," %d                   ",ep_num1);
	sprintf((char*)lcd_text[2]," %d                    ",ep_num2);
	sprintf((char*)lcd_text[3]," %d                   ",u16_num_disp);
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
		}
	}
}

void while_fun(void)
{
	lcd_show();
}

void u162eeprom(u16 num, u8 adr)
{
  u8 temp1 = num/256;
  u8 temp2 = num%256;
  i2c_write(adr,temp1);
  i2c_write(adr+1,temp2);
	ep_num1 = i2c_read(adr);
	ep_num2 = i2c_read(adr+1);
}

void my_init(void)
{
//	double test;
//	i2c_write(0,(u8)float_num);
//	
//	test = float_num - (int)float_num;
//	
//	i2c_write(1,(u8)(test*100));
//	ep_num1 = i2c_read(0);
//	ep_num2 = i2c_read(1);
	
//		i2c_write(0,(u8)(u16_num>>8));
//		i2c_write(1,(u8)(u16_num));
//	ep_num1 = i2c_read(0);
//	ep_num2 = i2c_read(1);
  u162eeprom(u16_num,0);
	u16_num_disp = (ep_num1<<8) + ep_num2;
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

