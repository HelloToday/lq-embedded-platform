#include "fun.h"

u16 gpioc_odr = 0;
u8 lcd_mode = 0;
u8 lcd_text[10][40];
u8 test = 0;

u16 my_rate = 0;
u16 my_con = 0;
u16 my_rate_max = 0;
u16 my_rate_min = 0;
u16 my_rate_warn_high = 100;
u16 my_rate_warn_low = 60;
u16 my_rate_warn_high_set = 100;
u16 my_rate_warn_low_set = 60;
u32 pb4_fre = 0;

u8 warn_mode = 0;//0:high,1:low
u8 warn_update = 0;//0:退出，1：进入
u8 warn_update_flag = 0;

float r37_volt = 0;

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
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"       HEART        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"      Rate:%d       ",my_rate);
	sprintf((char*)lcd_text[4],"       Con:%d       ",my_con);
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"        %.2f        ",r37_volt);
	sprintf((char*)lcd_text[9],"        %d          ",pb4_fre);
	
}
void lcd_part1(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"       RECORD       ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"       Max:%d       ",my_rate_max);
	sprintf((char*)lcd_text[4],"       Min:%d       ",my_rate_min);
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");
	
}
void lcd_part2(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        PARA        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"      High:%d       ",my_rate_warn_high);
	sprintf((char*)lcd_text[4],"       Low:%d       ",my_rate_warn_low);
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
	
	for(int i = 0; i < 10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);	
	}
}

void volt_get(void)
{
	HAL_ADC_Start(&hadc2);
	u32 temp = HAL_ADC_GetValue(&hadc2);
	r37_volt = 3.3f*temp/4096;
	
}

void warn_set_ctrl(void)
{
	if(warn_update == 1)
	{
		u16 temp = 45.0f*r37_volt+15;
		if(r37_volt > 3.0f)
		{
			temp = 45.0f*3.0f+15;
		}
		else if(r37_volt<1.0f)
		{
			temp = 45.0f*1.0f+15;
		}
		
		if(warn_mode == 0)//high
		{
			my_rate_warn_high = temp;
		}
		else if(warn_mode == 1)
		{
			my_rate_warn_low = temp;
		}
	}
}

void get_M_value_ctrl(void)
{
	static u16 last_max = 0;
	static u16 last_min = 1000;
	if(last_min>my_rate || last_min < 30)//初始化上电会进入该函数，但此时my_rate仍为0，就会导致my_rate_min一直为0，所以添加 || last_min < 30
	{
		last_min = my_rate;
		my_rate_min = my_rate;
	}	
	
	if(my_rate > last_max)
	{
		last_max = my_rate;
		my_rate_max = my_rate;
	}
}

u32 warn_led_time_click = 0;
u8 led6_flag = 0;


void get_warn_sum(void)
{
	static u16 last_rate = 0;
	if((my_rate > my_rate_warn_high_set) && last_rate < my_rate_warn_high_set)
	{
		my_con++;
		warn_led_time_click = HAL_GetTick();
		led6_flag = 1;
	}
	else if((my_rate < my_rate_warn_low_set)&&last_rate > my_rate_warn_low_set)
	{
		my_con++;
		warn_led_time_click = HAL_GetTick();
		led6_flag = 1;
	}
	last_rate = my_rate;
	
}

void while_fun(void)
{
	warn_set_ctrl();
	get_M_value_ctrl();
	get_warn_sum();
	lcd_show();
	
	if(lcd_mode == 0)
	{
		led_show(1,1);
	}
	else
	{
		led_show(1,0);
	}
	
	if(lcd_mode == 1)
	{
		led_show(2,1);
	}
	else
	{
		led_show(2,0);
	}	
	
	if(lcd_mode == 2)
	{
		led_show(3,1);
	}
	else
	{
		led_show(3,0);
	}

	if(lcd_mode == 2&&warn_update==1&&warn_mode ==0)
	{
		led_show(4,1);
	}
	else
	{
		led_show(4,0);
	}		
	
	if(lcd_mode == 2&&warn_update==1&&warn_mode ==1)
	{
		led_show(5,1);
	}
	else
	{
		led_show(5,0);
	}

	if(HAL_GetTick() - warn_led_time_click > 5000)
	{
		led6_flag = 0;
	}
	led_show(6,led6_flag);
	
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM16)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			u32 temp = HAL_TIM_ReadCapturedValue(&htim16,TIM_CHANNEL_1);
			__HAL_TIM_SetCounter(&htim16,0);
			temp = 80000000/(80*(temp+1));
			pb4_fre = temp;
			if(temp <= 1000)
			{
				my_rate = 30;
			}
			else if(temp >= 2000)
			{
				my_rate = 200;
			}
			else 
				my_rate = (u16)(0.17f*temp-140);
		}
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
		if(counter7 % 100 == 0)
		{
			volt_get();
		}
	}
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
			if(lcd_mode == 2)//从参数界面切换到数值界面
			{
				my_rate_warn_high = my_rate_warn_high_set;
				my_rate_warn_low = my_rate_warn_low_set;
				
				warn_update = 0;
			}
			
			lcd_mode++;
			if(lcd_mode == 3)
			{
				lcd_mode = 0;
			}
			else if(lcd_mode == 2)
			{
				warn_mode = 0;
			}
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
			if(lcd_mode == 2)
			{
				warn_mode++;
				if(warn_mode == 2)
				{
					warn_mode = 0;
				}
			}
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
			if(lcd_mode == 2)
			{
				warn_update++;
				if(warn_update == 2)
				{
					warn_update = 0;
				}
				if(warn_update == 0)//从进入修改变成退出状态
				{
					my_rate_warn_high_set = my_rate_warn_high;
					my_rate_warn_low_set = my_rate_warn_low;
				}
			}
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
			if(lcd_mode == 0)
			{
				my_con = 0;
			}
			
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

