#include "fun.h"

u16 gpioc_odr = 0;

u8 lcd_mode = 0;
u8 lcd_text[10][40];

u16 n_value = 0;
s16 nh_value = 150;
u16 duty = 0;
float fre = 0;
float r37_volt = 0;

float param_k = 0;

u32 counter15 = 0;

u8 rx_data = 0;
u8 rx_len = 0;
u8 rx_buff[10];

u8 led3_flag = 0;


void led_show(u8 led, u8 mode)
{
  u16 temp = GPIOC->ODR;
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
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"        DATA        ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"       N:%dr/s      ",n_value);
  sprintf((char *)lcd_text[4],"       D:%d%%       ",duty);
  sprintf((char *)lcd_text[5],"       U:%.1fV      ",r37_volt);
  sprintf((char *)lcd_text[6],"                    ");
  sprintf((char *)lcd_text[7],"                    ");
  sprintf((char *)lcd_text[8],"                    ");
  sprintf((char *)lcd_text[9],"                 P:%d",lcd_mode+1);
}
void lcd_part1(void)
{
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"        PARA        ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"      NH:%d         ",nh_value);
  sprintf((char *)lcd_text[4],"                    ");
  sprintf((char *)lcd_text[5],"                    ");
  sprintf((char *)lcd_text[6],"                    ");
  sprintf((char *)lcd_text[7],"                    ");
  sprintf((char *)lcd_text[8],"                    ");
  sprintf((char *)lcd_text[9],"                 P:%d",lcd_mode+1);
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
  for(u8 i = 0; i<10; i++)
  {
    LCD_DisplayStringLine(i*24,lcd_text[i]);
  }
}

void volt_get(void)
{
  HAL_ADC_Start(&hadc2);
  u32 temp = HAL_ADC_GetValue(&hadc2);
  r37_volt = temp*3.3/4096.0f;
}

void n_ctrl(void)
{
  static u16 last_value = 0;
  
  static u16 last_nh_value = 0;
  
  if(duty < 10)
    param_k = 1.0f;
  else if(duty >= 10 && duty <= 50)
  {
    param_k = -0.025f*(duty-50);
  }
  else if(duty > 50 && duty <= 90)
  {
    param_k = 0.025f*(duty-50);
  }
  else if(duty > 90)
    param_k = 1.0f;
  
  n_value = param_k*r37_volt*1000.0f/5.0f;
  
  
  if(n_value > nh_value && last_value < nh_value)
  {
    printf("warn");
  }
    
  if(last_nh_value > nh_value && n_value > nh_value)
  {
    printf("warn");
  }
  last_nh_value = nh_value;
  last_value = n_value;
  
}

void led_ctrl(void)
{
  if(lcd_mode == 0)
    led_show(1,1);
  else
    led_show(1,0);
  
  if(lcd_mode == 1)
    led_show(2,1);
  else
    led_show(2,0);
  
  if(n_value > nh_value)
  {
    led_show(3,1);
  }   
  else
  {
    led3_flag++;
    if(led3_flag == 2)
      led3_flag = 0; 
    led_show(3,led3_flag);
  }
  
}



void while_fun(void)
{
  volt_get();
  lcd_show();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  static u32 ch1 = 0;
  static u32 ch2 = 0;
  if(htim->Instance == TIM3)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      ch1 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      ch2 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim3,0);
      
      fre = 80000000.0f/(80*(ch2+1));
      duty = (ch1+1)*100.0f/(ch2+1);
      
    }
  }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static u32 counter7 = 0;
  if(htim->Instance == TIM7)
  {
    counter7++;
    if(counter7 % 50 == 0)
    {
      n_ctrl();
      key_scan();
    }
    if(counter7 % 100 == 0)
    {
      led_ctrl();
    }   
  }
  if(htim->Instance == TIM15)
  {
    counter15++;
    if(counter15 % 10 == 0)
    {
      counter15 = 0;
      
      HAL_TIM_Base_Stop_IT(&htim15);
      
      if(rx_len == 1 && rx_buff[0] == '?')
      {
        printf("N=%dr/s",n_value);
      }
      else
      {
        printf("error");
      }
      
      rx_len = 0;
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    HAL_TIM_Base_Stop_IT(&htim15);
    
    if(rx_len <10)
    {
      rx_buff[rx_len++] = rx_data;
    }
    
    __HAL_TIM_SetCounter(&htim15,0);
    counter15 = 0;
    HAL_UART_Receive_IT(&huart1,&rx_data,1);
    HAL_TIM_Base_Start_IT(&htim15);
    
  }
}

u8 key_state[4]={0,0,0,0};
u8 key_last_state[4]={1,1,1,1};
u32 key_press_time[4]={0,0,0,0};
u8 key_mode[4]={0,0,0,0};
void key_scan(void)
{
  key_state[0] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
  key_state[1] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
  key_state[2] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
  key_state[3] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
  for(u8 i = 0; i < 4; i++)
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
        if(HAL_GetTick() - key_press_time[i] > 1000)
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
  
  for(u8 i = 0; i < 4; i++)
  {
    key_last_state[i] = key_state[i];
  }
  
  
  
//__________________________________Key1_mode_______________________________________//
  
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
      
    }
  }
  
//__________________________________Key2_mode_______________________________________//
  
  if(key_mode[1])
  {
    if(key_mode[1] == 1)
    {
      if(lcd_mode == 1)
      {
        nh_value+=50;
        if(nh_value > 1000)
          nh_value = 0;
      }
    }
    else if(key_mode[1] == 2)
    {
      
    }
  } 

//__________________________________Key3_mode_______________________________________//
  
  if(key_mode[2])
  {
    if(key_mode[2] == 1)
    {
      if(lcd_mode == 1)
      {
        nh_value-=50;
        if(nh_value < 0)
          nh_value = 1000;
      }
    }
    else if(key_mode[2] == 2)
    {
      
    }
  }
//__________________________________Key4_mode_______________________________________//
  
  if(key_mode[3])
  {
    if(key_mode[3] == 1)
    {
      
    }
    else if(key_mode[3] == 2)
    {
      
    }
  }

  
  
  for(u8 i = 0; i<4; i++)
  {
    key_mode[i] = 0;
  }  
}
