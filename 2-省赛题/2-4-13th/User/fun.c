#include "fun.h"

uint8_t lcd_text[10][40];//
uint8_t part_mode = 0;
uint8_t key1_num = '@';//'@'0x40
uint8_t key2_num = '@';
uint8_t key3_num = '@';
uint8_t pass_str[3]={'1','2','3'};
uint8_t pass_error_cnt = 0;
uint8_t pass_error_flag = 0;

uint32_t fre = 0;
uint8_t led1_mode = 0;
uint8_t led2_mode = 0;

uint8_t rec_data = 0;
uint8_t rx_buff[2][10];
uint8_t rec_lenth = 0;
uint8_t rec_line = 0;
uint8_t rec_col = 0;
uint8_t rec_error = 0;
uint8_t rec_success = 0;

uint16_t gpioc_ostate = 0;


void led_show(uint8_t led, uint8_t mode)
{
	uint16_t temp = GPIOC->ODR;
	GPIOC->ODR = gpioc_ostate;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	if(mode)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led -1), GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led -1), GPIO_PIN_SET);
	}
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	gpioc_ostate = GPIOC->ODR;                         
	GPIOC->ODR = temp;
}

void lcd_part0(void)
{
	sprintf((char *)lcd_text[0],"                       ");
	sprintf((char *)lcd_text[1],"       PSD             ");
	sprintf((char *)lcd_text[2],"                       ");
	sprintf((char *)lcd_text[3],"    B1:%c              ",key1_num);
	sprintf((char *)lcd_text[4],"    B2:%c              ",key2_num);
	sprintf((char *)lcd_text[5],"    B3:%c              ",key3_num);
	sprintf((char *)lcd_text[6],"                       ");
	sprintf((char *)lcd_text[7],"                       ");
	sprintf((char *)lcd_text[8],"                       ");
	sprintf((char *)lcd_text[9],"                       ");
}

void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"                       ");
	sprintf((char *)lcd_text[1],"       STA             ");
	sprintf((char *)lcd_text[2],"                       ");
	sprintf((char *)lcd_text[3],"    F:2000Hz            ");
	sprintf((char *)lcd_text[4],"    D:10%%             ");
	sprintf((char *)lcd_text[5],"                       ");
	sprintf((char *)lcd_text[6],"                       ");
	sprintf((char *)lcd_text[7],"                       ");
	sprintf((char *)lcd_text[8],"                       ");
	sprintf((char *)lcd_text[9],"                       ");
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
	
	for(int i = 0; i < 10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}		
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM17)
	{
		uint32_t temp = HAL_TIM_ReadCapturedValue(&htim17,TIM_CHANNEL_1);
		fre = 80000000/(80*temp);
		__HAL_TIM_SET_COUNTER(&htim17,0);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t counter7 = 0;
	static uint32_t counter7_5s = 0;
	static uint32_t counter15 = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{
			key_scan();
			if(rec_success == 1)
			{
				Rec_Buff_Proc();
				rec_success = 0;
			}
			led_show(1,led1_mode);
			led_show(2,led2_mode);			
		}
		
		if(part_mode == 1)
		{
			led1_mode = 1;
			counter7_5s++;
			if(counter7_5s >= 5000)
			{
				part_mode = 0;
				__HAL_TIM_SET_AUTORELOAD(&htim2,1000);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,1000*0.5);	
				key1_num = '@';
				key2_num = '@';
				key3_num = '@';
				led1_mode = 0;
				
				counter7_5s = 0;
				
			}
		}
		if(pass_error_flag == 1)
		{
			counter7_5s++;
			if(counter7_5s % 100 == 0)
			{
				led2_mode++;
				led2_mode %= 2;
			}
			if(counter7_5s >= 5000)
			{
				led2_mode = 0;
				pass_error_flag = 0;
				
				counter7_5s = 0;
				
			}
		}
	}
	else if(htim->Instance == TIM15)
	{
		counter15++;
		if(counter15 >= 10)
		{
			HAL_TIM_Base_Stop_IT(&htim15);
			HAL_UART_Receive_IT(&huart1,&rec_data,1);
			if(rec_lenth != 7)
			{
				rec_error = 1;
			}
			else
			{
				rec_success = 1;
			}			
			rec_line = 0;
			rec_col = 0;		
			rec_lenth = 0;
			
			counter15 = 0;
		}
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		HAL_TIM_Base_Stop_IT(&htim15);
		__HAL_TIM_SET_COUNTER(&htim15,0);
		
		rec_lenth++;		
		rx_buff[rec_line][rec_col++] = rec_data;
		
		if(rec_data == '-')
		{
			rec_line++;
			rec_col = 0;		
		}
		
		HAL_TIM_Base_Start_IT(&htim15);
		HAL_UART_Receive_IT(&huart1,&rec_data,1);
	}
}

void Rec_Buff_Proc(void)
{
	if(rx_buff[0][0] == pass_str[0] && rx_buff[0][1] == pass_str[1] && rx_buff[0][2] == pass_str[2])
	{
		if(rx_buff[1][0] >= '0' && rx_buff[1][0] <= '9' &&\
			 rx_buff[1][1] >= '0' && rx_buff[1][1] <= '9' &&\
			 rx_buff[1][2] >= '0' && rx_buff[1][2] <= '9')
		{
			pass_str[0] = rx_buff[1][0];
			pass_str[1] = rx_buff[1][1];
			pass_str[2] = rx_buff[1][2];		
		}

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

#define LONG_TIME 1000

void key_scan(void)
{
	key1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
  key2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
  key3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
  key4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
	
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
//____________________________key1_mode___________________//
	if(key1_mode)
	{
		if(key1_mode==1&&part_mode == 0)
		{
			if(key1_num == '@')
			{
				key1_num = '0';
			}
			else
			{
				key1_num++;
				if(key1_num > '9')
				{
					key1_num = '0';
				}
			}
		}
		key1_mode = 0;
	}
//____________________________key2_mode___________________//
	if(key2_mode)
	{
		if(key2_mode==1&&part_mode == 0)
		{
			if(key2_num == '@')
			{
				key2_num = '0';
			}
			else
			{
				key2_num++;
				if(key2_num > '9')
				{
					key2_num = '0';
				}
			}
		}
		key2_mode = 0;
	}
//____________________________key3_mode___________________//
	if(key3_mode)
	{
		if(key3_mode==1&&part_mode == 0)
		{
			if(key3_num == '@')
			{
				key3_num = '0';
			}
			else
			{
				key3_num++;
				if(key3_num > '9')
				{
					key3_num = '0';
				}
			}
		}
		key3_mode = 0;
	}
//____________________________key4_mode___________________//
	if(key4_mode)
	{
		if(key4_mode==1)
		{
			if(pass_str[0] == key1_num && pass_str[1] == key2_num && pass_str[2] == key3_num)
			{
				part_mode = 1;
				__HAL_TIM_SET_AUTORELOAD(&htim2,500);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,500*0.1);
			}
			else
			{
				pass_error_cnt++;
				if(pass_error_cnt > 0)
				{
					key1_num = '@';
					key2_num = '@';
					key3_num = '@';					
				}
				if(pass_error_cnt == 3)
				{
					pass_error_cnt = 0;
					led2_mode = 1;
					
					pass_error_flag = 1;
				}
			}
		}
		key4_mode = 0;
	}
	
	
	key1_last_state = key1_state;
  key2_last_state = key2_state;
  key3_last_state = key3_state;
	key4_last_state = key4_state;
	
	
}	
