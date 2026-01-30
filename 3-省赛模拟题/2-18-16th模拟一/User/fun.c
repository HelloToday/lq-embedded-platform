#include "fun.h"

u16 gpioc_odr = 0;
u8 lcd_mode = 0;
u8 lcd_text[10][40];
u8 test = 0;
u8 word_input[5]="***";
u8 word_ctrl[5]="***";
u8 pass_word[5];
u8 word_cnt = 0;
u8 word_success = 0;

u32 time_word = 0;

u8 lcd_flag = 0;
float volt_adc = 0;
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

void password_part0(void)
{
	sprintf((char*)lcd_text[4],"        %c %c %c      ",word_input[0],word_input[1],word_input[2]);
}

void lcd_part0(void)
{	
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        Lock          ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"      Pass Word       ");
	//sprintf((char*)lcd_text[4],"                    ");
	password_part0();
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");
	//sprintf((char*)lcd_text[8]," %c %c %c           ",pass_word[0],pass_word[1],pass_word[2]);
	//sprintf((char*)lcd_text[9],"%.2f                   ",volt_adc);
	
}
void password_part1(void)
{
	sprintf((char*)lcd_text[4],"        %c %c %c      ",word_ctrl[0],word_ctrl[1],word_ctrl[2]);
}
void lcd_part1(void)
{	
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        Set          ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"       Change       ");
	//sprintf((char*)lcd_text[4],"                    ");
	password_part1();
	sprintf((char*)lcd_text[5],"                    ");
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");
	//sprintf((char*)lcd_text[9],"%.2f                   ",volt_adc);
	
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
			volt_get();
			word_get();
		}
		if(counter7 % 100 == 0)
		{
			lcd_flag = 1;
		}
	}
}

void volt_get(void)
{
	HAL_ADC_Start(&hadc2);
	uint32_t temp = HAL_ADC_GetValue(&hadc2);
	volt_adc = temp * 3.3 / 4096;
}

void word_get(void)
{
	if(lcd_mode == 0)
	{
		led_show(1,1);
		led_show(2,0);
		if(volt_adc < 1.50f)
		{
			word_input[word_cnt] = 0x30;
		}
		else if(volt_adc >= 1.50f && volt_adc <= 2.50f)
		{
			word_input[word_cnt] = 0x31;
		}
		else if(volt_adc > 2.50f)
		{
			word_input[word_cnt] = 0x32;
		}		
	}
	else if(lcd_mode == 1)
	{	
		led_show(1,0);
		led_show(2,1);		
		if(volt_adc < 1.50f)
		{
			word_ctrl[word_cnt] = 0x30;
		}
		else if(volt_adc >= 1.50f && volt_adc <= 2.50f)
		{
			word_ctrl[word_cnt] = 0x31;
		}
		else if(volt_adc > 2.50f)
		{
			word_ctrl[word_cnt] = 0x32;
		}					
	}

	
}

void while_fun(void)
{
	
	if(lcd_flag == 1)
	{
		lcd_show();
		lcd_flag = 0;
	}
	//lcd_show();
	if(word_success == 1)
	{
		eprom_write(0,(pass_word[0] - 0x30));
		eprom_write(1,(pass_word[1]- 0x30));
		eprom_write(2,(pass_word[2]- 0x30));
		
//		eprom_write(0,(pass_word[0]));
//		eprom_write(1,(pass_word[1]));
//		eprom_write(2,(pass_word[2]));			
		word_success = 0;
	}
	if((HAL_GetTick() - time_word > 3000) && word_cnt == 0 && lcd_mode == 1)
	{
		lcd_mode = 0;
		strcpy((char *)word_ctrl, "***");
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
			if(lcd_mode == 0)
			{
				word_cnt++;
				if(word_cnt == 3)
				{
					word_cnt = 0;
					//if(strcmp((char *)word_input, (char *)pass_word) == 0)
					if(word_input[0] == pass_word[0] && word_input[1] == pass_word[1] && word_input[2] == pass_word[2])
					{
						strcpy((char *)word_input, "***");
						lcd_mode = 1;
						time_word = HAL_GetTick();
					}
					else
					{
						strcpy((char *)word_input, "***");
					}
				}					
			}
			else if(lcd_mode == 1)
			{
				word_cnt++;
				if(word_cnt == 3)
				{
					word_cnt = 0;
					lcd_mode = 0;
					word_success = 1;
					strcpy((char *)pass_word,(char *)word_ctrl);
					strcpy((char *)word_ctrl, "***");
				}					
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

