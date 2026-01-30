#include "fun.h"

u16 gpioc_odr = 0;

u8 lcd_text[10][40];
u8 lcd_mode = 0;

u8 nav_state[4] = {'S','R','L'};
u8 nav_mode = 0;

u16 r37_value = 0;
float r37_volt = 0;

float dis_m = 0;

u8 rec_data = 0; 
u8 rec_len = 0;
u8 rec_buf[20];
u8 rec_nav_flag = 0;
u8 rec_nav_mode = 0;
u8 rec_other_flag = 0;

u8 nav_no_ctrl = 0;

u8 send_buf[4][10] = {"Success\r\n","Warn\r\n","ERROR\r\n","WAIT\r\n"};

u16 counter_nav = 0;
u16 counter_led = 0;

u8 led1_flag = 0;
u8 led2_flag = 0;
u8 led8_flag = 0;

u8 lcd_show_flag = 0;

void led_show(u8 led, u8 mode)
{
	u16 temp = GPIOC->ODR;
	GPIOC->ODR = gpioc_odr;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	
	if(mode)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led-1), GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led-1), GPIO_PIN_SET);
	
	gpioc_odr = GPIOC->ODR;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	GPIOC->ODR = temp;
}

void lcd_part0(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"        DATA        ");
	sprintf((char *)lcd_text[2],"                    ");
	sprintf((char *)lcd_text[3],"       N:%c          ",nav_state[nav_mode]);
	sprintf((char *)lcd_text[4],"       D:%.1f          ",dis_m);
	sprintf((char *)lcd_text[5],"                    ");
	sprintf((char *)lcd_text[6],"                    ");
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");
	//sprintf((char *)lcd_text[9],"volt:%.2f           ",r37_volt);
}
void lcd_part1(void)
{
	sprintf((char *)lcd_text[0],"                    ");
	sprintf((char *)lcd_text[1],"                    ");
	sprintf((char *)lcd_text[2],"                    ");
	sprintf((char *)lcd_text[3],"                    ");
	sprintf((char *)lcd_text[4],"        WARN        ");
	sprintf((char *)lcd_text[5],"                    ");
	sprintf((char *)lcd_text[6],"                    ");
	sprintf((char *)lcd_text[7],"                    ");
	sprintf((char *)lcd_text[8],"                    ");
	sprintf((char *)lcd_text[9],"                    ");
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
	for(int i = 0; i<10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);
	}
}

void adc_cap(void)
{
	HAL_ADC_Start(&hadc2);
	r37_value = HAL_ADC_GetValue(&hadc2);
	r37_volt = r37_value*3.3/4096;
}

void volt_dis_change(void)
{
	adc_cap();
	dis_m = 100.0f*r37_volt > 300.0f ? 300.0f : 100.0f*r37_volt;
}
void nav_ctrl(void)
{
	if(rec_nav_flag == 1)
	{
		if(rec_nav_mode == 'L')
		{
			nav_mode = 2;
		}
		else if(rec_nav_mode == 'R')
		{
			nav_mode = 1;
		}
	}
	
	if(nav_no_ctrl)
	{
		nav_no_ctrl = 0;
		counter_nav = 0;
		rec_nav_flag = 0;
		//nav_mode = 0;
		lcd_mode = 1;
		//HAL_UART_Transmit_IT(&huart1,send_buf[1],10);
		printf("Warn\r\n");
	}
	
	if(lcd_mode == 0&&rec_other_flag)
	{
		//HAL_UART_Transmit_IT(&huart1,send_buf[2],10);
		printf("ERROR\r\n");
		rec_other_flag = 0;
	}
	else if(lcd_mode == 1&&(rec_other_flag == 1 || rec_nav_flag == 1))
	{
		//HAL_UART_Transmit_IT(&huart1,send_buf[3],10);
		printf("WAIT\r\n");
		rec_other_flag = 0;
		rec_nav_flag = 0;
	}
}
	
void while_fun(void)
{
	volt_dis_change();
	nav_ctrl();
	lcd_show();
//	if(lcd_show_flag == 1)
//	{
//		//lcd_show();
//		lcd_show_flag = 0;
//	}
		
	
	if(lcd_mode == 1)
	{
		//led8_flag = 1;
		//led1_flag = 0;
		//led2_flag = 0;
		counter_led = 0;
		
		led_show(1,0);
		led_show(2,0);
		led_show(8,1);
		
	}
	else
	{
		led_show(8,0);
	}
	
	
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static u32 counter7 = 0;
	static u32 counter15 = 0;
	if(htim->Instance == TIM7)
	{
		counter7++;
		if(counter7 % 50 == 0)
		{		
			key_scan();
			if(rec_nav_flag && lcd_mode == 0)
			{
				counter_nav++;
				if(counter_nav >= 100)//5s
				{
					rec_nav_flag = 0;
					counter_nav = 0;
					nav_no_ctrl = 1;
				}
			}
		}
		if(lcd_mode == 0 && nav_mode)
		{
			counter_led++;
			if((counter_led % 100 == 0)&&nav_mode == 2)//l
			{
				led1_flag++;
				led1_flag %= 2;
				
				led_show(1,led1_flag);
				led_show(2,0);				
			}
			else if((counter_led % 100 == 0)&&nav_mode == 1)//r
			{
				led2_flag++;
				led2_flag %= 2;
				
				led_show(1,0);
				led_show(2,led2_flag);				
			}
		}
//		if(counter7 % 100 == 0)
//		{
//			lcd_show_flag = 1;
//		}
	}
	if(htim->Instance == TIM15)
	{
		counter15++;
		if(counter15 % 10 == 0)
		{
			HAL_TIM_Base_Stop_IT(&htim15);
			if(rec_len == 1 && (rec_buf[0] == 'L' || rec_buf[0] == 'R'))
			{
				rec_nav_flag = 1;
				rec_nav_mode = rec_buf[0];
			}
			else if(rec_len>=1)
			{
				rec_other_flag = 1;
			}
			rec_len = 0;
			counter15 = 0;
		}		
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_TIM_Base_Stop_IT(&htim15);
	__HAL_TIM_SET_COUNTER(&htim15,0);
	rec_buf[rec_len] = rec_data;
	rec_len++;
	
	HAL_TIM_Base_Start_IT(&htim15);
	HAL_UART_Receive_IT(&huart1, &rec_data, 1);
}


u8 key_state[4] = {0};
u8 key_last_state[4] = {1,1,1,1};
u8 key_mode[4] = {0};
u32 key_press_time[4] = {0};

void key_scan(void)
{
	key_state[0]=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	key_state[1]=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	key_state[2]=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	key_state[3]=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
	
	for(int i = 0; i < 4; i++)
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
				if(HAL_GetTick() - key_press_time[i] >= 1000)
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
//____________________________key1_mode_______________________//	
	if(key_mode[0])
	{
		if(key_mode[0] == 1)
		{
			if(lcd_mode == 1)
			{
				lcd_mode = 0;
				
				nav_mode = 0;
				//HAL_UART_Transmit_IT(&huart1,send_buf[0],10);	
				printf("Success\r\n");
			}
		}
		key_mode[0] = 0;
	}
//____________________________key2_mode_______________________//	
	if(key_mode[1])
	{
		if(key_mode[1] == 1)
		{
			
		}
		key_mode[1] = 0;
	}
//____________________________key3_mode_______________________//	
	if(key_mode[2])
	{
		if(key_mode[2] == 1)
		{
			if(lcd_mode == 0 && nav_mode == 2)//l
			{
				counter_nav = 0;
				rec_nav_flag = 0;
				nav_mode = 0;
				//lcd_mode = 1;
				//HAL_UART_Transmit_IT(&huart1,send_buf[0],10);
				printf("Success\r\n");				
			}
			else if(lcd_mode == 0 &&nav_mode == 0)
			{
				lcd_mode = 1;
				//HAL_UART_Transmit_IT(&huart1,send_buf[1],10);	
				printf("Warn\r\n");
			}
		}
		key_mode[2] = 0;
	}
//____________________________key4_mode_______________________//	
	if(key_mode[3])
	{
		if(key_mode[3] == 1)
		{
			if(lcd_mode == 0 && nav_mode == 1)//r
			{
				counter_nav = 0;
				rec_nav_flag = 0;
				nav_mode = 0;
				//lcd_mode = 1;
				//HAL_UART_Transmit_IT(&huart1,send_buf[0],10);	
				printf("Success\r\n");
			}
			else if(lcd_mode == 0 &&nav_mode == 0)
			{
				lcd_mode = 1;
				//HAL_UART_Transmit_IT(&huart1,send_buf[1],10);	
				printf("Warn\r\n");
			}			
		}
		key_mode[3] = 0;
	}	
	
	key_last_state[0]=key_state[0];
	key_last_state[1]=key_state[1];
	key_last_state[2]=key_state[2];
	key_last_state[3]=key_state[3];
									 
}



