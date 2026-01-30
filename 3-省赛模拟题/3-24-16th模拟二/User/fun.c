#include "fun.h"

u16 gpioc_odr = 0;
u8 lcd_mode = 0;
u8 lcd_text[10][40];
u8 test = 0;

u8 rx_data = 0;

u16 fre_pb4 = 0;
u16 fre_pa15 = 0;
u16 fre_para = 1000;
u8 ctrl_mode[2][6] = {"KEY","USART"}; 
u8 ctrl_mode_flag = 0;

u8 fre_pb4_ctrl = 1;
u8 fre_pa15_ctrl = 1;

u8 led3_flag = 0;

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
	sprintf((char*)lcd_text[1],"        DATA        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"     F1=%dHZ        ",fre_pb4);
	sprintf((char*)lcd_text[4],"     F2=%dHZ        ",fre_pa15);
	sprintf((char*)lcd_text[5],"     MODE=%s        ",ctrl_mode[ctrl_mode_flag]);
	sprintf((char*)lcd_text[6],"                    ");
	sprintf((char*)lcd_text[7],"                    ");
	sprintf((char*)lcd_text[8],"                    ");
	sprintf((char*)lcd_text[9],"                    ");	
}

void lcd_part1(void)
{
	sprintf((char*)lcd_text[0],"                    ");
	sprintf((char*)lcd_text[1],"        PARA        ");
	sprintf((char*)lcd_text[2],"                    ");
	sprintf((char*)lcd_text[3],"      PF=%dHZ       ",fre_para);
	sprintf((char*)lcd_text[4],"                    ");
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
	for(int i = 0; i < 10; i++)
	{
		LCD_DisplayStringLine(i*24,lcd_text[i]);	
	}
}

void while_fun(void)
{
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

	if(ctrl_mode_flag == 1)
	{
		led_show(8,1);
	}
	else
	{
		led_show(8,0);
	}
	
	if(fre_pb4>=fre_para || fre_pa15>=fre_para)
	{
		led_show(3,0);
	}
	
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM16)
	{
		if(fre_pb4_ctrl)
		{
			u32 temp = HAL_TIM_ReadCapturedValue(&htim16,TIM_CHANNEL_1);
			fre_pb4 = 80000000/(80*(temp+1));			
		}
		__HAL_TIM_SetCounter(&htim16,0);

		
	}
	else if(htim->Instance == TIM8)
	{
		if(fre_pa15_ctrl)
		{
			u32 temp = HAL_TIM_ReadCapturedValue(&htim8,TIM_CHANNEL_1);
			fre_pa15 = 80000000/(80*(temp+1));				
		}
		__HAL_TIM_SetCounter(&htim8,0);
	
	}

}

u8 rx_buff[10];
u8 rx_lenth = 0;
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
		if(counter7%100 == 0)
		{
			if(fre_pb4<fre_para && fre_pa15<fre_para)
			{
				led3_flag ++;
				if(led3_flag == 2)
					led3_flag = 0;
				led_show(3,led3_flag);
			}			
		}
	}
	else if(htim->Instance == TIM15)
	{
		HAL_TIM_Base_Stop_IT(&htim15);
		if(rx_lenth == 3&&ctrl_mode_flag == 1)
		{
			if(rx_buff[0] =='T'&&rx_buff[1] =='F'&&rx_buff[2] =='1')
			{
				fre_pb4_ctrl = 1;
			}
			else if(rx_buff[0] =='T'&&rx_buff[1] =='F'&&rx_buff[2] =='2')
			{
				fre_pa15_ctrl = 1;
			}
			else if(rx_buff[0] =='P'&&rx_buff[1] =='F'&&rx_buff[2] =='1')
			{
				fre_pb4_ctrl = 0;
			}
			else if(rx_buff[0] =='P'&&rx_buff[1] =='F'&&rx_buff[2] =='2')
			{
				fre_pa15_ctrl = 0;
			}
			else
			{
				printf("NULL");
			}
		}
		else if(rx_lenth == 2&&ctrl_mode_flag == 1)
		{
			if(rx_buff[0] =='F'&&rx_buff[1] =='1')
			{
				printf("F1:%dHZ",fre_pb4);
			}
			else if(rx_buff[0] =='F'&&rx_buff[1] =='2')
			{
				printf("F2:%dHZ",fre_pa15);
			}
			else
			{
				printf("NULL");
			}			
		}
		else
		{
			printf("NULL");
		}
		
		rx_lenth = 0;
		HAL_UART_Receive_IT(&huart1,&rx_data,1);
		
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		HAL_TIM_Base_Stop_IT(&htim15);
		__HAL_TIM_SetCounter(&htim15,0);
		//HAL_UART_Transmit_IT(&huart1,&rx_data,1);
		
		if(rx_lenth < 10)
		{
			rx_buff[rx_lenth++]= rx_data;			
		}
		
		HAL_TIM_Base_Start_IT(&htim15);
		HAL_UART_Receive_IT(&huart1,&rx_data,1);		
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
			lcd_mode++;
			if(lcd_mode == 2)
			{
				lcd_mode = 0;
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
			ctrl_mode_flag++;
			if(ctrl_mode_flag == 2)
			{
				ctrl_mode_flag = 0;
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
			if(lcd_mode == 0&&ctrl_mode_flag == 0)
			{
				fre_pb4_ctrl++;
				if(fre_pb4_ctrl == 2)
					fre_pb4_ctrl = 0;
			}
			else if(lcd_mode == 1)
			{
				fre_para+=1000;
				if(fre_para > 10000)
					fre_para = 1000;				
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
			if(lcd_mode == 0&& ctrl_mode_flag == 0)
			{
				fre_pa15_ctrl++;
				if(fre_pa15_ctrl == 2)
					fre_pa15_ctrl = 0;
			}
			else if(lcd_mode == 1)
			{
				fre_para-=1000;
				if(fre_para < 1000)
					fre_para = 10000;
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

