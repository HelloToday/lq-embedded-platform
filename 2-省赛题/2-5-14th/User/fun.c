#include "fun.h"

uint16_t gpioc_state = 0;

uint8_t lcd_text[10][50];
uint8_t lcd_part_mode = 0;
uint8_t pwm_out_mode = 'L';
uint8_t pwm_out_pulse = 10;

uint8_t para_k = 1;
uint8_t para_r = 1;
uint8_t para_k_key = 1;
uint8_t para_r_key = 1;

uint8_t para_change_ctrl = 0;
float pwm_out_v = 0;
uint8_t pwm_change_count = 0;
float pwm_hv_max = 0;
float pwm_lv_max = 0;
float volt_value = 0;
uint32_t fre = 0;
uint8_t pwm_fre_ctrl = 1;//1 up , 2 down
uint8_t pwm_fre_ctrl_flag = 0;
int16_t pwm_fre_change_num = 0;

uint8_t pwm_duty_ctrl_key4 = 0;

uint32_t key1_no_ctrl_tick = 0;

uint8_t  key1_no_ctrl_flag = 1;//只用一次

float test_duty = 0;

void led_show(uint8_t led, uint8_t mode)
{
	uint16_t temp = GPIOC->ODR;
	GPIOC->ODR = gpioc_state;
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	
	if(mode)
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	gpioc_state = GPIOC->ODR;
	GPIOC->ODR = temp;
}

void lcd_part0(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        DATA        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"     M=%c           ",pwm_out_mode);
	sprintf((char*)lcd_text[4],"     P=%d%%         ",pwm_out_pulse);
	sprintf((char*)lcd_text[5],"     V=%.1f         ",pwm_out_v);
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"  dut:%.2f      ",test_duty);
	//sprintf((char*)lcd_text[8],"                    ");
	//sprintf((char*)lcd_text[9],"                    ");
	sprintf((char*)lcd_text[8],"  fre:%d            ",fre);
	sprintf((char*)lcd_text[9],"  vol:%.2f          ",volt_value);
}	
void lcd_part1(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        PARA        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"     R=%d           ",para_r_key);
	sprintf((char*)lcd_text[4],"     K=%d           ",para_k_key);
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");
}
void lcd_part2(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        RECD        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"     N=%d           ",pwm_change_count);
	sprintf((char*)lcd_text[4],"     MH=%.1f         ",pwm_hv_max);
	sprintf((char*)lcd_text[5],"     ML=%.1f         ",pwm_lv_max);
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");
}	
void lcd_show(void)
{
	if(lcd_part_mode == 0)
	{
		lcd_part0();
	}
	else if(lcd_part_mode == 1)
	{
		lcd_part1();
	}
	else if(lcd_part_mode == 2)
	{
		lcd_part2();
	}	
	
	for(int i = 0; i < 10; i++)
	{
		LCD_DisplayStringLine(i*24, lcd_text[i]);
	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint64_t counter7 = 0;
	static uint32_t pwm_counter = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{
			key_scan();
			volt_value = get_volt();
			vmax_ctrl();
		}
		
		if(counter7 % 10 == 0)
		{
			
			if(pwm_fre_ctrl_flag == 1)//pwm_fre_ctrl
			{
				pwm_counter++;
				pwm_fre_change_num ++;
				
				if(pwm_fre_ctrl == 1)
				{
					float temp_period = 1000000/(4000+(pwm_fre_change_num+1)*8);
					
					__HAL_TIM_SET_AUTORELOAD(&htim2,temp_period-1);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(temp_period*pwm_out_pulse/100) - 1);
					__HAL_TIM_SetCounter(&htim2,0);
				}
				else if(pwm_fre_ctrl == 2)
				{
					float temp_period = 1000000/(8000-(pwm_fre_change_num+1)*8);
					
					__HAL_TIM_SET_AUTORELOAD(&htim2,temp_period-1);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(temp_period*pwm_out_pulse/100) - 1);
					__HAL_TIM_SetCounter(&htim2,0);					
				}
				if(pwm_counter == 499)
				{
					pwm_counter = 0;
					pwm_fre_ctrl_flag = 0;
					pwm_fre_change_num = 0;
					pwm_out_mode = 'H';
					
					pwm_fre_ctrl++;
					pwm_change_count++;
					
					if(pwm_fre_ctrl > 2)
					{
						pwm_fre_ctrl = 1;
						pwm_out_mode = 'L';
					}
					
				}
			}
			else if(pwm_fre_ctrl_flag == 0)//容易忽略点：当不进行高低频转换时，adc采集的电压也可以对信号占空比的影响
			{
				if(pwm_fre_ctrl == 1)
				{
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(250*pwm_out_pulse/100) - 1);
				}
				else if(pwm_fre_ctrl == 2)
				{
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,(125*pwm_out_pulse/100) - 1);
				}
			}
		}
		if(counter7 % 100 == 0)
		{
			
			//led
			if(lcd_part_mode == 0 && pwm_fre_ctrl_flag == 0)
			{
				led_show(1,1);
				led_show(2,0);				
			}
			else if(pwm_fre_ctrl_flag == 1)
			{
				static uint8_t led2_flag = 0;
				//led_show(1,0);
				led2_flag++;
				led2_flag %= 2;
				led_show(2,led2_flag);
			}
			else
			{
				led_show(1,0);
				led_show(2,0);
			}
			
			if(pwm_duty_ctrl_key4 == 1)
			{
				led_show(3,1);
			}
			else
			{
				led_show(3,0);
			}					
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t tim3_ch1 = 0;
	static uint32_t tim3_ch2 = 0;
	
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			tim3_ch1 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1);		
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			tim3_ch2 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_2);
			__HAL_TIM_SetCounter(&htim3,0);
			fre = 80000000/(80*(tim3_ch2+1));
			pwm_out_v = (fre * 2* 3.14 * para_r)/(100 * para_k);
			test_duty = (tim3_ch1+1)*100/(float)(tim3_ch2+1);
		}

	}
}

double get_volt(void)
{
	HAL_ADC_Start(&hadc2);
	uint32_t temp = HAL_ADC_GetValue(&hadc2);
	return (double)temp/4096*3.3;
}

void while_fun(void)
{	
	if(pwm_fre_ctrl_flag == 0 && pwm_duty_ctrl_key4 == 0)
	{
		if(volt_value <= 1.0f)
		{
			pwm_out_pulse = 10;
		}
		else if(volt_value > 1.0f && volt_value <= 3.0f)
		{
			
			pwm_out_pulse = (75/2)*(volt_value - 1)+10;//k = (85-10)/(3-1),y-y1 = k(x-x1)
		}
		else
		{
			pwm_out_pulse = 85;
		}
	}	
}

void vmax_ctrl(void)
{
	static float last_v = 0;
	static uint32_t counter = 0;
	
	if(fabs(pwm_out_v - last_v) < (para_r/para_k)*20)
	{
		counter++;
		if(counter >= 40)
		{
			counter = 0;
			if(pwm_fre_ctrl == 1)
			{
				pwm_lv_max = pwm_out_v;
			}
			else if(pwm_fre_ctrl == 2)
			{
				pwm_hv_max = pwm_out_v;
			}
		}
	}
	else
	{
		counter = 0;
	}
	
	last_v = pwm_out_v;
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

#define LONG_TIME 2000

void key_scan(void)
{
	key1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
//	
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
//	
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
//	
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
//	
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

	
	//____________key1_mode_________________//
	if(key1_mode)
	{
		if(key1_mode == 1)
		{
			if(lcd_part_mode == 1)//界面更换后，参数才生效
			{
				para_r = para_r_key;
				para_k = para_k_key;
			}
			
			lcd_part_mode++;
			lcd_part_mode %= 3;
			
			if(lcd_part_mode == 1)//参数界面时，默认选择调整参数R
			{
				para_change_ctrl = 0;
			}			
			
			//pwm_out_pulse++;
		}
		else if(key1_mode == 2)
		{
			//pwm_out_pulse = 2;
		}
		key1_mode= 0;
	}
	//____________key2_mode_________________//
	if(key2_mode)
	{
		if(key2_mode == 1)
		{
//			if(pwm_fre_ctrl_flag == 0 && lcd_part_mode == 0)
//			{ 
//				pwm_fre_ctrl_flag = 1;	
//			}
			if(lcd_part_mode == 1)
			{
				para_change_ctrl++;
				if(para_change_ctrl > 1)
				{
					para_change_ctrl = 0;
				}
			}
			
			else if((lcd_part_mode == 0 && ((HAL_GetTick() - key1_no_ctrl_tick)>=5000) && pwm_fre_ctrl_flag == 0) || (key1_no_ctrl_flag == 1 && lcd_part_mode == 0))
			{
				key1_no_ctrl_tick = HAL_GetTick();
				key1_no_ctrl_flag = 0;
				pwm_fre_ctrl_flag = 1;	
			}
			

		}
		else if(key2_mode == 2)
		{
			//pwm_out_pulse = 2;
		}
		key2_mode= 0;
	}	
	//____________key3_mode_________________//
	if(key3_mode)
	{
		if(key3_mode == 1)
		{
			if(lcd_part_mode == 1)
			{
				if(para_change_ctrl == 0)
				{
					para_r_key++;
					if(para_r_key > 10)
					{
						para_r_key = 1;
					}					
				}
				else
				{
					para_k_key++;
					if(para_k_key > 10)
					{
						para_k_key = 1;
					}							
				}

			}
				
		}
		else if(key3_mode == 2)
		{

		}
		key3_mode= 0;
	}
	//____________key4_mode_________________//
	if(key4_mode)
	{
		if(key4_mode == 1)
		{
			if(lcd_part_mode == 1)
			{
				if(lcd_part_mode == 1)
				{
					if(para_change_ctrl == 0)
					{
						para_r_key--;
						if(para_r_key < 1)
						{
							para_r_key = 10;
						}					
					}
					else
					{
						para_k_key--;
						if(para_k_key < 1)
						{
							para_k_key = 10;
						}							
					}

				}
			}
			else if(lcd_part_mode == 0)
			{
				pwm_duty_ctrl_key4 = 0;
			}
		}
		else if(key4_mode == 2)
		{
			if(lcd_part_mode == 0)
			{
				pwm_duty_ctrl_key4 = 1;
			}
		}
		key4_mode= 0;
	}	
	
	
	key1_last_state = key1_state;
  key2_last_state = key2_state;
  key3_last_state = key3_state;
	key4_last_state = key4_state;
	
}


