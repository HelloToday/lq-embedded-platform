#include "fun.h"

u16 gpioc_odr = 0;

u8 lcd_text[10][35];
u8 lcd_mode = 0;

u16 fre_pa15 = 0;
u16 fre_pb4 = 0;
u16 fre_threshold = 2500;
u16 fre_threshold_ctrl = 2500;

u8 uart_fre_flag = 0;
u8 uart_fre_init_flag = 1;

u8 rx_data = 0;
u8 rx_buff[10];
u8 rx_len = 0;

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

void fre_ctrl(void)
{
  if(uart_fre_init_flag)
  {
    sprintf((char *)lcd_text[5],"      RX:N/A          ");
  }
  
  else
  {
    switch(uart_fre_flag)
    {
      case 0:
      {
        sprintf((char *)lcd_text[5],"      RX:F1,ERROR      ");
      }break;
      
      case 1:
      {
        sprintf((char *)lcd_text[5],"      RX:F1,OK         ");
      }break;
      case 2:
      {
        sprintf((char *)lcd_text[5],"      RX:F2,ERROR      ");
      }break;
      case 3:
      {
        sprintf((char *)lcd_text[5],"      RX:F2,OK         ");
      }break;      
    }
  }
}

void lcd_part0(void)
{
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"        DATA        ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"      F1:%dHZ       ",fre_pb4);
  sprintf((char *)lcd_text[4],"      F2:%dHZ       ",fre_pa15);
  fre_ctrl();
  sprintf((char *)lcd_text[6],"                    ");
  sprintf((char *)lcd_text[7],"                    ");
  sprintf((char *)lcd_text[8],"                    ");
  sprintf((char *)lcd_text[9],"                    ");
}

void lcd_part1(void)
{
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"        PARA        ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"     BF:%dHZ        ",fre_threshold);
  sprintf((char *)lcd_text[4],"                    ");
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
  
  for(int i = 0; i< 10; i++)
  {
    LCD_DisplayStringLine(i*24,lcd_text[i]);
  }
}


void while_fun(void)
{
//  if(uart_fre_flag == 1 || uart_fre_flag ==3)
//  {
//    led_show(1,1);
//  }
//  else
//    led_show(1,0);
  
  lcd_show();
}
u32 counter15 = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static u32 counter7 = 0;
  if(htim->Instance == TIM7)
  {
    counter7++;
    if(counter7 % 50 == 0)
    {
      key_scan();
    }
  }
  if(htim->Instance == TIM15)
  { 
    counter15++;
    if(counter15 % 10 == 0)
    {
      HAL_TIM_Base_Stop_IT(&htim15);
      if(rx_len == 2)
      {
        if(rx_buff[0] == 'F' && rx_buff[1] == '1')
        {
          uart_fre_init_flag = 0;
          if(fre_pb4 > fre_threshold_ctrl)
          {
            uart_fre_flag = 1;
            printf("F1,OK");
            led_show(1,1);
          }
          else
          {
            uart_fre_flag = 0;
            printf("F1,ERROR");
            led_show(1,0);
          }
          
        }
        
        else if(rx_buff[0] == 'F' && rx_buff[1] == '2')
        {
          uart_fre_init_flag = 0;
          if(fre_pa15 > fre_threshold_ctrl)
          {
            uart_fre_flag = 3;
            printf("F2,OK");
            led_show(1,1);
          }
          else
          {
            uart_fre_flag = 2;
            printf("F2,ERROR");
            led_show(1,0);
          }
        }      
      }
      
      rx_len = 0;
      counter15 = 0;
    }

    
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  u32 temp = 0;
  if(htim->Instance == TIM8)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      temp = HAL_TIM_ReadCapturedValue(&htim8,TIM_CHANNEL_1);
      __HAL_TIM_SET_COUNTER(&htim8,0);
      fre_pa15 = 80000000/(80*(temp+1));
    }
  }
  if(htim->Instance == TIM16)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      temp = HAL_TIM_ReadCapturedValue(&htim16,TIM_CHANNEL_1);
      __HAL_TIM_SET_COUNTER(&htim16,0);
      fre_pb4 = 80000000/(80*(temp+1));
    }
  }  
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    HAL_TIM_Base_Stop_IT(&htim15);
    
    if(rx_len < 10)
    {
      rx_buff[rx_len++] = rx_data;
    }
    
    __HAL_TIM_SET_COUNTER(&htim15,0);
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
  
  for(int i = 0; i<4; i++)
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
  
  for(int i = 0; i<4; i++)
  {
     key_last_state[i] = key_state[i];
  }
  
//___________________________________KEY1_MODE__________________________//
  if(key_mode[0])
  {
    if(key_mode[0] == 1)
    {
      lcd_mode++;
      
      if(lcd_mode == 2)
        lcd_mode = 0;
      
      if(lcd_mode == 0)
        fre_threshold = fre_threshold_ctrl;
    }
  }
//___________________________________KEY2_MODE__________________________//
  if(key_mode[1])
  {
    if(key_mode[1] == 1)
    {
      if(lcd_mode == 1)
      {
        fre_threshold+= 500;
        if(fre_threshold > 5000)
          fre_threshold = 1000;
      }
    }
  }
//___________________________________KEY3_MODE__________________________//
  if(key_mode[2])
  {
    if(key_mode[2] == 1)
    {
      if(lcd_mode == 1)
      {
        fre_threshold-= 500;
        if(fre_threshold < 1000)
          fre_threshold = 5000;
      }
    }
  }
//___________________________________KEY4_MODE__________________________//
  if(key_mode[3])
  {
    if(key_mode[3] == 1)
    {
      if(lcd_mode == 1)
      {
        fre_threshold_ctrl = fre_threshold;
      }
    }
  }
  
  
  for(int i = 0; i<4; i++)
  {
     key_mode[i] = 0;
  }
  
}



