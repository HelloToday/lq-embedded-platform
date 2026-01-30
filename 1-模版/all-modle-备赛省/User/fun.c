#include "fun.h"
u16 gpioc_odr = 0;

u8 lcd_mode = 0;
u8 lcd_text[10][30];

u8 key_test = 0;

u32 counter15 = 0;
u8 rx_data = 0;
u8 rx_buff[10];
u8 rx_len = 0;


u16 r37_value = 0;
float r37_volt = 0;
float r38_volt = 0;
float mcp_volt = 0;

float fre_r39 = 0;
float fre_r40 = 0;
float duty_r39 = 0;
float duty_r40 = 0;

float fre_pa6 = 0;
float duty_pa6 = 0;

uint8_t eep_u8_data = 0;
u16 eep_u16_data = 0;
uint8_t mcp_data = 0;

RTC_TimeTypeDef my_sTime = {0};
RTC_DateTypeDef my_sDate = {0};
RTC_AlarmTypeDef my_sAlarm = {0};
RTC_AlarmTypeDef my_sAlarmb = {0};

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
  sprintf((char *)lcd_text[0],"        Part0        ");
  sprintf((char *)lcd_text[1],"        key:%d       ",key_test);
  sprintf((char *)lcd_text[2],"        r37v:%.2f    ",r37_volt);
  sprintf((char *)lcd_text[3],"        r38v:%.2f    ",r38_volt);
  sprintf((char *)lcd_text[4],"        mcpv:%.2f    ",mcp_volt);
  sprintf((char *)lcd_text[5],"        r39f:%.2f    ",fre_r39);
  sprintf((char *)lcd_text[6],"        r40f:%.2f    ",fre_r40);
  sprintf((char *)lcd_text[7],"        r39d:%.2f    ",duty_r39);
  sprintf((char *)lcd_text[8],"        r40d:%.2f    ",duty_r40);
  sprintf((char *)lcd_text[9],"                     ");
}

void lcd_part1(void)
{
  sprintf((char *)lcd_text[0],"        Part1        ");
  sprintf((char *)lcd_text[1],"        pa6f:%.2f    ",fre_pa6);
  sprintf((char *)lcd_text[2],"        pa6d:%.2f    ",duty_pa6);
  sprintf((char *)lcd_text[3],"        eep_u8:%d    ",eep_u8_data);
  sprintf((char *)lcd_text[4],"        eep_u16:%d    ",eep_u16_data);
  sprintf((char *)lcd_text[5],"        mcp:%d        ",mcp_data);
  //sprintf((char *)lcd_text[6],"                    ");
  sprintf((char *)lcd_text[6],"Alara:%d:%d:%d        ",my_sAlarm.AlarmTime.Hours,my_sAlarm.AlarmTime.Minutes,my_sAlarm.AlarmTime.Seconds);
  sprintf((char *)lcd_text[7],"Alarb:%d:%d:%d        ",my_sAlarmb.AlarmTime.Hours,my_sAlarmb.AlarmTime.Minutes,my_sAlarmb.AlarmTime.Seconds);
  sprintf((char *)lcd_text[8],"Date:%d-%d-%d         ",my_sDate.Year,my_sDate.Month,my_sDate.Date);
  sprintf((char *)lcd_text[9],"Time:%d:%d:%d         ",my_sTime.Hours,my_sTime.Minutes,my_sTime.Seconds);
  
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

void get_volt(void)
{
  u32 temp = 0;
  HAL_ADC_Start(&hadc1);
  temp = HAL_ADC_GetValue(&hadc1);
  r38_volt = temp*3.3/4096.0f;
  
  HAL_ADC_Start(&hadc1);
  temp = HAL_ADC_GetValue(&hadc1);  
  mcp_volt = temp*3.3/4096.0f;
//  if(HAL_DMA_GetState(&hdma_adc2))
//  {
//    r37_volt = r37_value*3.3f/4096.0f;
//  }
  r37_volt = r37_value*3.3f/4096.0f;
    
}

void eepom_ctrl(void)
{
  eep_write(0x00,0x0B);
  eep_u8_data = eep_read(0x00);
  
  u8 temp1 = 5678>>8;
  u8 temp2 = (u8)5678;
  eep_write(0x01,temp1);
  eep_write(0x02,temp2);
  eep_u16_data = (eep_read(0x01)<<8) + eep_read(0x02);
  
  mcp_write(63);
  mcp_data = mcp_read();  
}

void rtc_ctrl(void)
{
  HAL_RTC_GetTime(&hrtc,&my_sTime,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,&my_sDate,RTC_FORMAT_BIN);
  
  HAL_RTC_GetAlarm(&hrtc,&my_sAlarm,RTC_ALARM_A,RTC_FORMAT_BIN);
  HAL_RTC_GetAlarm(&hrtc,&my_sAlarmb,RTC_ALARM_B,RTC_FORMAT_BIN);
}

void while_fun(void)
{
  rtc_ctrl();
  get_volt();
  lcd_show();
}

u8 led4_flag = 0;
u8 led3_flag = 0;
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  led3_flag++;
  if(led3_flag == 2)
    led3_flag = 0;  
  led_show(3,led3_flag);
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
  led4_flag++;
  if(led4_flag == 2)
    led4_flag = 0;
  led_show(4,led4_flag);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  static u32 r39_ch1 = 0;
  static u32 r39_ch2 = 0;
  static u32 r40_ch1 = 0;
  static u32 r40_ch2 = 0;
  
  static u32 pa1_ch1 = 0;
  static u32 pa1_ch2 = 0;
  if(htim->Instance == TIM3)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      r39_ch1 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      r39_ch2 = HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim3,0);
      
      fre_r39 = 80000000/(80*(r39_ch2+1));
      duty_r39 = (r39_ch1+1)*1.0f/(r39_ch2+1);
    }
  }
  
  else if(htim->Instance == TIM8)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      r40_ch1 = HAL_TIM_ReadCapturedValue(&htim8,TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      r40_ch2 = HAL_TIM_ReadCapturedValue(&htim8,TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim8,0);
      
      fre_r40 = 80000000/(80*(r40_ch2+1));
      duty_r40 = (r40_ch1+1)*1.0f/(r40_ch2+1);
    }
  }
  else if(htim->Instance == TIM2)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      pa1_ch1 = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      pa1_ch2 = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim2,0);
      fre_pa6 = 80000000/(80*(pa1_ch2+1));
      duty_pa6 = (pa1_ch1+1)*1.0f/(pa1_ch2+1);
      
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
      //key_scan();
      key_scan_2();
    }
  }
  if(htim->Instance == TIM15)
  {
    counter15++;
    if(counter15 % 10 == 0)
    {
      HAL_TIM_Base_Stop_IT(&htim15);
      HAL_UART_Receive_IT(&huart1,&rx_data,1);
      
      if(rx_len == 2)
      {
        if(rx_buff[0] == 'A' && rx_buff[1] == '1')
        {
          key_test++;
          led_show(2,1);
          
        }
        else
        {
          
        }
      }
      else
      {
        
      }
      
      counter15 = 0;
      rx_len = 0;

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
    
    counter15 = 0;
    __HAL_TIM_SetCounter(&htim15,0);
    HAL_UART_Receive_IT(&huart1,&rx_data,1);
    HAL_TIM_Base_Start_IT(&htim15);
  }
}

u8 key_state[4]={0,0,0,0};
u8 key_last_state[4]={1,1,1,1};
u32 key_press_time[4]={0,0,0,0};
u8 key_mode[4]={0,0,0,0};
u8 case_state[4] = {0};
void key_scan_2(void)
{
  key_state[0] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
  key_state[1] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
  key_state[2] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
  key_state[3] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
  
  for(u8 i = 0; i < 4; i++)
  {
    switch(case_state[i])
    {
      case 0:
      {
        if(key_state[i] == 0 && key_last_state[i] == 1)
        {
          key_press_time[i] = HAL_GetTick();
          case_state[i]++;
        }
      }break;
      case 1:
      {
        if(key_state[i] == 1 && key_last_state[i] == 1 && (HAL_GetTick() - key_press_time[i] > 1000))//长按
        {
          key_mode[i] = 2;
          case_state[i] = 0;
        }
        else if(key_state[i] == 1 && key_last_state[i] == 1 && (HAL_GetTick() - key_press_time[i] > 400)&&(HAL_GetTick() - key_press_time[i] < 1000))//单击
        {
          key_mode[i] = 1;
          case_state[i] = 0;
        }
        else if(key_state[i] == 0 && key_last_state[i] == 1&& (HAL_GetTick() - key_press_time[i] < 400))//双击
        {
          key_mode[i] = 3;
          case_state[i] = 0; 
        }
      }break;
    }
  }
  
  for(u8 i = 0; i < 4; i++)
  {
    key_last_state[i] = key_state[i];
  }
  
//_________________key1_mode_________________//
  if(key_mode[0])
  {
    if(key_mode[0] == 1)
    {
      key_test = 1;
    }
    else if(key_mode[0] == 2)
    {
      key_test = 2;
    }
    else if(key_mode[0] == 3)
    {
      key_test = 3;
    }
  }
  
  
  
  for(u8 i = 0; i<4; i++)
  {
    key_mode[i] = 0;
  }   
  
}

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
      key_test = 0;
    }
  }
  
//__________________________________Key2_mode_______________________________________//
  
  if(key_mode[1])
  {
    if(key_mode[1] == 1)
    {
      key_test++;
    }
    else if(key_mode[1] == 2)
    {
      key_test = 0;
    }
  } 

//__________________________________Key3_mode_______________________________________//
  
  if(key_mode[2])
  {
    if(key_mode[2] == 1)
    {
      key_test++;
    }
    else if(key_mode[2] == 2)
    {
      key_test = 0;
    }
  }
//__________________________________Key4_mode_______________________________________//
  
  if(key_mode[3])
  {
    if(key_mode[3] == 1)
    {
      key_test++;
    }
    else if(key_mode[3] == 2)
    {
      key_test = 0;
    }
  }

  
  
  for(u8 i = 0; i<4; i++)
  {
    key_mode[i] = 0;
  }  
}


