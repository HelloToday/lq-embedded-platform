#include "fun.h"

u16 gpioc_odr = 0;
u8 lcd_mode = 0;
u8 lcd_text[10][40];
u8 test = 0;

float r37_volt = 0;
u8 my_count = 0;
float volt_up_set = 3.3f;
float volt_down_set = 0;

u8 volt_ctrl_mode = 0;

float r37_volt_get;
u8 led8_flag = 0;

float fre = 0;
float duty = 0;

u8 lcd_update_flag = 0;


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
	sprintf((char*)lcd_text[3],"      VR37:%.1fV    ",r37_volt);
	sprintf((char*)lcd_text[4],"     COUNT:%d       ",my_count);
	sprintf((char*)lcd_text[5],"                    ");
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
	sprintf((char*)lcd_text[3],"       UP:%.1f      ",volt_up_set);
	sprintf((char*)lcd_text[4],"     DOWN:%.1f      ",volt_down_set);
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

void adc_volt_get(void)
{
  HAL_ADC_Start(&hadc2);
  u32 temp = HAL_ADC_GetValue(&hadc2);
  r37_volt_get = 3.3f*temp/4096.0f;
  r37_volt = (u8)r37_volt_get+(float)(((u8)((r37_volt_get - (u8)r37_volt_get)*10))/10.0f);
  
}

void led_ctrl(void)
{
  if(lcd_mode == 0)
  {
    led_show(1,1);
  }
  else
    led_show(1,0);
  
  if(lcd_mode == 1)
  {
    led_show(2,1);
  }
  else
    led_show(2,0); 

  if(volt_up_set >= r37_volt && volt_down_set <=r37_volt)
  {
    led_show(8,1);

  }
  else
  {
    led8_flag++;
    if(led8_flag == 2)
      led8_flag = 0;
    led_show(8,led8_flag);
  }
}

void count_ctrl(void)
{
  static u8 count_flag = 1;
  if(volt_up_set >= r37_volt && volt_down_set <=r37_volt)
  {
    __HAL_TIM_SetAutoreload(&htim2,1000-1);
    __HAL_TIM_SetCounter(&htim2,0);
    __HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,(1000*0.7)-1);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
    count_flag = 1;
  }
  else
  {
    if(count_flag)
      my_count++;
    
    __HAL_TIM_SetAutoreload(&htim2,1000-1);
    __HAL_TIM_SetCounter(&htim2,0);
    __HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,(1000*0.3)-1);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
    
    count_flag = 0;//确保电压一直大于或小于限制参数时，只进行一次计数加1
  }
  
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  static u32 temp1 = 0;
  static u32 temp2 = 0;
  if(htim->Instance == TIM3)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      temp1 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      temp2 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim3,0);
      fre = (80000000/(80*(temp2+1)));
      duty = (temp1+1)*100/(float)(temp2+1);
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
    if(counter7%100 == 0)
    {
      count_ctrl();

      led_ctrl();
    }
	}
}

void while_fun(void)
{
  
  adc_volt_get();
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
			lcd_mode++;
      if(lcd_mode == 2)
        lcd_mode = 0;
      if(lcd_mode == 1)
        volt_ctrl_mode = 0;
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
      if(lcd_mode == 1)
      {
        if(volt_ctrl_mode == 0)
        {
          volt_up_set+=0.3f;
//          if(volt_up_set > 3.3000001f)
//          {
//            volt_up_set = volt_down_set;
//          }
          if(volt_up_set > 3.3f)
          {
            volt_up_set = 0;
          }          
        }
        else if(volt_ctrl_mode == 1)
        {
          volt_down_set+=0.3f;
//          if(volt_down_set > volt_up_set)
//          {
//            volt_down_set = volt_up_set;
//          }
          if(volt_down_set > 3.3f)
          {
            volt_down_set = 0;
          }          
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
      if(lcd_mode == 1)
      {
        if(volt_ctrl_mode == 0)
        {
          volt_up_set-=0.3f;
//          if(volt_up_set < volt_down_set)
//          {
//            volt_up_set = volt_down_set;
//          }
          if(volt_up_set < 0)
          {
            volt_up_set = 3.3f;
          }          
        }
        else if(volt_ctrl_mode == 1)
        {
          volt_down_set-=0.3f;
//          if(volt_down_set < 0)
//          {
//            volt_down_set = 0;
//          }
          if(volt_down_set < 0)
          {
            volt_down_set = 3.3f;
          }           
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
			if(lcd_mode == 1)
      {
        volt_ctrl_mode++;
        if(volt_ctrl_mode == 2)
          volt_ctrl_mode = 0;
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

