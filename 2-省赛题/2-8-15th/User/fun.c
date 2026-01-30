#include "fun.h"

uint16_t gpioc_odr = 0;

uint8_t lcd_text[10][35];
uint8_t part_mode = 0;
uint8_t lcd_update_flag = 0;

float pwma_fre = 0;
float pwmb_fre = 0;
float pwma_fre_real = 0;
float pwmb_fre_real = 0;
float pwma_cycle = 0;
float pwmb_cycle = 0;

uint8_t frecy_flag = 0;
uint8_t pwma_fre_change = 0;
uint8_t pwmb_fre_change = 0;
uint8_t pwma_cycle_change = 0;
uint8_t pwmb_cycle_change = 0;

int16_t pwm_pd_fre = 1000;
int16_t pwm_ph_fre = 5000;
int16_t pwm_px_fre = 0;
uint8_t para_ctrl_flag = 0;

uint8_t nda_count = 0;
uint8_t ndb_count = 0;
uint8_t nha_count = 0;
uint8_t nhb_count = 0;

uint32_t pmwa_cnt = 0;
uint32_t pmwb_cnt = 0;

void led_show(uint8_t led, uint8_t mode)
{
	uint16_t temp = GPIOC->ODR;
	GPIOC->ODR = gpioc_odr;
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	if(mode)
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8 << (led - 1),GPIO_PIN_SET);
	
	gpioc_odr = GPIOC->ODR;
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	GPIOC->ODR = temp;	
}

void lcd_part0_change(void)
{
	if(frecy_flag == 0)
	{
		if(pwma_fre_change == 0)
		{
			sprintf((char *)lcd_text[3],"     A=%.0fHz         ",pwma_fre);
		}
		else
		{
			sprintf((char *)lcd_text[3],"     A=%.2fKHz         ",pwma_fre/1000);
		}
		if(pwma_fre < 0)
		{
			sprintf((char *)lcd_text[3],"     A=NULL            ");
		}
		
		if(pwmb_fre_change == 0)
		{
			sprintf((char *)lcd_text[4],"     B=%.0fHz         ",pwmb_fre);		
		}
		else
		{
			sprintf((char *)lcd_text[4],"     B=%.2fKHz         ",pwmb_fre/1000);
		}			
		if(pwmb_fre < 0)
		{
			sprintf((char *)lcd_text[4],"     B=NULL            ");
		}
	}
	else
	{
		if(pwma_cycle_change == 0)
		{
			sprintf((char *)lcd_text[3],"     A=%.0fuS         ",pwma_cycle);
		}
		else
		{
			sprintf((char *)lcd_text[3],"     A=%.2fmS         ",pwma_cycle/1000);
		}
		if(pwma_fre < 0)
		{
			sprintf((char *)lcd_text[3],"     A=NULL            ");
		}
		
		if(pwmb_cycle_change == 0)
		{
			sprintf((char *)lcd_text[4],"     B=%.0fuS         ",pwmb_cycle);
		}
		else
		{
			sprintf((char *)lcd_text[4],"     B=%.2fmS         ",pwmb_cycle/1000);
		}
		if(pwmb_fre < 0)
		{
			sprintf((char *)lcd_text[4],"     B=NULL            ");
		}
					
	}	
}

void lcd_part0(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"        DATA        ");
	sprintf((char *)lcd_text[2],"                    ");
	
	lcd_part0_change();
	
	sprintf((char *)lcd_text[5],"                    ");
	sprintf((char *)lcd_text[6],"                    ");
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");	
	//sprintf((char *)lcd_text[8],"       a:%.0f       ",pwma_fre_real);
	//sprintf((char *)lcd_text[9],"       b:%.0f       ",pwmb_fre_real);
	
}

void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"        PARA        ");
	sprintf((char *)lcd_text[2],"                    ");
	sprintf((char *)lcd_text[3],"     PD=%dHz        ",pwm_pd_fre);
	sprintf((char *)lcd_text[4],"     PH=%dHz        ",pwm_ph_fre);	
	sprintf((char *)lcd_text[5],"     PX=%dHz        ",pwm_px_fre);
	sprintf((char *)lcd_text[6],"                    ");
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");
		
}
void lcd_part2(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"        RECD        ");
	sprintf((char *)lcd_text[2],"                    ");
	sprintf((char *)lcd_text[3],"     NDA=%d         ",nda_count);
	sprintf((char *)lcd_text[4],"     NDB=%d         ",ndb_count);	
	sprintf((char *)lcd_text[5],"     NHA=%d         ",nha_count);
	sprintf((char *)lcd_text[6],"     NHB=%d         ",nhb_count);
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");
		
}
void lcd_show(void)
{
	if(part_mode == 0)
	{
		lcd_part0();
	}
	else if(part_mode == 1)
	{
		lcd_part1();
	}
	else if(part_mode == 2)
	{
		lcd_part2();
	}
	for(int i = 0; i<10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t counter7 = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{
			key_scan();
			measure_fre_ctrl();
		}
		if(counter7 % 100 == 0)
		{
			lcd_update_flag = 1;
			fre_duty_ctrl();
			led_ctrl();
			
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		pmwa_cnt = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);
		__HAL_TIM_SET_COUNTER(&htim2,0);

	}
	else if(htim->Instance == TIM3)
	{
		pmwb_cnt = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1);
		__HAL_TIM_SET_COUNTER(&htim3,0);
		
	}
}

void fre_duty_ctrl(void)
{
	pwma_fre_real = 80000000.0f/(80 * (pmwa_cnt + 1));
	pwma_fre = pwma_fre_real + pwm_px_fre;
	pwma_cycle = 1000000.0f/pwma_fre;
	
	pwmb_fre_real = 80000000.0f/(80 * (pmwb_cnt + 1));
	pwmb_fre = pwmb_fre_real + pwm_px_fre;
	pwmb_cycle = 1000000.0f/pwmb_fre;			
}

void measure_fre_ctrl(void)
{
	static uint32_t counter_fre = 0;
	static float pwma_fre_last = 0;
	static float pwmb_fre_last = 0;
	
	static float pwma_fre_max = 0;
	static float pwma_fre_min = 30000;
	static float pwmb_fre_max = 0;
	static float pwmb_fre_min = 30000;
	
	static uint8_t no_count_flag = 1;//防止上电时误判频率突变
	
	counter_fre++;
	
	if(pwma_fre > pwm_ph_fre && pwma_fre_last < pwm_ph_fre)
	{
		nha_count++;
	}
	if(pwmb_fre > pwm_ph_fre && pwmb_fre_last < pwm_ph_fre)
	{
		nhb_count++;
	}	
	pwma_fre_last = pwma_fre;
	pwmb_fre_last = pwmb_fre;
	
//	if(counter_fre == 1 && no_count_flag == 0)
//	{
//		pwma_fre_min = pwma_fre;
//		pwmb_fre_min = pwmb_fre;
//	}
//	else if(counter_fre == 61&& no_count_flag == 0)
//	{
//		pwma_fre_max = pwma_fre;
//		pwmb_fre_max = pwmb_fre;
//		if(fabs(pwma_fre_max-pwma_fre_min) > pwm_pd_fre)
//		{
//			nda_count++;
//		}
//		if(fabs(pwmb_fre_max-pwmb_fre_min) > pwm_pd_fre)
//		{
//			ndb_count++;
//		}
//		counter_fre = 0;
//	}
	if(no_count_flag == 0)
	{
		if(counter_fre < 61)
		{
			if(pwma_fre >= 0)
			{
				if(pwma_fre > pwma_fre_max)
				{
					pwma_fre_max = pwma_fre;
				}
				else if(pwma_fre < pwma_fre_min)
				{
					pwma_fre_min = pwma_fre;
				}			
			}

			if(pwmb_fre >= 0)
			{
				if(pwmb_fre > pwmb_fre_max)
				{
					pwmb_fre_max = pwmb_fre;
				}
				else if(pwmb_fre < pwmb_fre_min)
				{
					pwmb_fre_min = pwmb_fre;
				}					
			}		
		}
		else
		{
			counter_fre = 0;
			if((pwma_fre_max - pwma_fre_min) > pwm_pd_fre)
			{
				nda_count++;
			}
			if((pwmb_fre_max - pwmb_fre_min) > pwm_pd_fre)
			{
				ndb_count++;
			}
			pwma_fre_max = 0;
			pwma_fre_min = 30000;
			
			pwmb_fre_max = 0;
			pwmb_fre_min = 30000;		
		}
	}
	else if(counter_fre == 2 && no_count_flag == 1)
	{
		no_count_flag = 0;
		counter_fre = 0;
	}
}

void led_ctrl(void)
{
	if(part_mode == 0)
	{
		led_show(1,1);
	}
	else
	{
		led_show(1,0);
	}
	
	if(pwma_fre > pwm_ph_fre)
	{
		led_show(2,1);
	}
	else
	{
		led_show(2,0);
	}	
	
	if(pwmb_fre > pwm_ph_fre)
	{
		led_show(3,1);
	}
	else
	{
		led_show(3,0);
	}	
	
	if(nda_count >= 3 || ndb_count >= 3)
	{
		led_show(8,1);
	}
	else
	{
		led_show(8,0);
	}
}

void while_fun(void)
{
	if(frecy_flag == 0)
	{
		if(pwma_fre > 1000)
		{
			pwma_fre_change = 1;
		}
		else
		{
			pwma_fre_change = 0;
		}
		
		if(pwmb_fre > 1000)
		{
			pwmb_fre_change = 1;
		}
		else
		{
			pwmb_fre_change = 0;
		}		
	}
	else
	{
		if(pwma_cycle > 1000)
		{
			pwma_cycle_change = 1;
		}
		else
		{
			pwma_cycle_change = 0;
		}
		
		if(pwmb_cycle > 1000)
		{
			pwmb_cycle_change = 1;
		}
		else
		{
			pwmb_cycle_change = 0;
		}		
	}
	if(lcd_update_flag)
	{
		lcd_show();
		lcd_update_flag = 0;
	}
}

uint8_t key_state[4] = {0};
uint8_t key_last_state[4] = {1,1,1,1};
uint8_t key_mode[4] = {0};
uint32_t key_press_time[4] = {0};
#define LONG_TIME 1000
void key_scan(void)
{
	key_state[0] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key_state[1] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key_state[2] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key_state[3] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
	
	for(uint8_t i = 0; i < 4; i++)
	{
		if(key_state[i] == 0)
		{
			if(key_last_state[i])
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
//______________________key1_mode_____________________//
	if(key_mode[0])
	{
		if(key_mode[0] == 1)
		{
			if(para_ctrl_flag == 0)
			{
				pwm_pd_fre += 100;
				if(pwm_pd_fre > 1000)
				{
					pwm_pd_fre = 1000;
				}
			}
			else if(para_ctrl_flag == 1)
			{
				pwm_ph_fre += 100;
				if(pwm_ph_fre > 10000)
				{
					pwm_ph_fre = 10000;
				}				
			}
			else if(para_ctrl_flag == 2)
			{
				pwm_px_fre += 100;
				if(pwm_px_fre > 1000)
				{
					pwm_px_fre = 1000;
				}					
			}
		}
		key_mode[0] = 0;
	}
//______________________key2_mode_____________________//
	if(key_mode[1])
	{
		if(key_mode[1] == 1)
		{
			if(para_ctrl_flag == 0)
			{
				pwm_pd_fre -= 100;
				if(pwm_pd_fre < 100)
				{
					pwm_pd_fre = 100;
				}
			}
			else if(para_ctrl_flag == 1)
			{
				pwm_ph_fre -= 100;
				if(pwm_ph_fre < 1000)
				{
					pwm_ph_fre = 1000;
				}				
			}
			else if(para_ctrl_flag == 2)
			{
				pwm_px_fre -= 100;
				if(pwm_px_fre < -1000)
				{
					pwm_px_fre = -1000;
				}					
			}
		}
		key_mode[1] = 0;
	}
//______________________key3_mode_____________________//
	if(key_mode[2])
	{
		if(key_mode[2] == 1)
		{
			if(part_mode == 0)
			{
				frecy_flag++;
				frecy_flag %= 2;				
			}
			else if(part_mode == 1)
			{
				para_ctrl_flag++;
				para_ctrl_flag %= 3;
			}

		}
		else if(key_mode[2] == 2)
		{
			if(part_mode == 2)
			{
				nda_count = 0;
				ndb_count = 0;
				nha_count = 0;
				nhb_count = 0;
			}
		}
		key_mode[2] = 0;
	}	
//______________________key4_mode_____________________//
	if(key_mode[3])
	{
		if(key_mode[3] == 1)
		{
			if(part_mode == 0)
			{
				frecy_flag = 0;
			}
			else if(part_mode == 1)
			{
				para_ctrl_flag = 0;
			}
			
			part_mode++;
			part_mode %= 3;			
		}
		key_mode[3] = 0;
	}	
	for(uint8_t i = 0; i < 4; i++)
	{
		key_last_state[i] = key_state[i];
	}
	
}

