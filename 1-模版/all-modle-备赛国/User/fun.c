#include "fun.h"
u16 gpioc_odr = 0;

u8 lcd_text[10][35];
u8 lcd_mode = 0;

u16 adc1_buff[4];
u16 adc2_buff[2];

float r37_volt,r38_volt,mcp_volt;

float pa15_fre,pa15_duty;
float pb4_fre;

float pa7_fre,pa7_duty;

u8 write_u8 = 20;
u8 read_u8 = 1;
u8 write_arr[4] = {3,4,5};
u8 read_arr[4] = {0,0,0};
double w_double = 1.6587;
double r_double = 0;
uint8_t mcp_data = 0;
u16 u16_data = 1568;
u16 u16_data_read = 0;

RTC_TimeTypeDef my_sTime = {0};
RTC_DateTypeDef my_sDate = {0};

void eep_init(void)
{
//  eep_write_u8(0x00,write_u8);
//  read_u8 = eep_read_u8(0x00);
  
//  eep_write_arr(0x00,write_arr,strlen((char*)write_arr));
//  eep_read_arr(0x00,read_arr,3);
  
//  eep_write_double(0x00,&w_double);
//  eep_read_double(0x00,&r_double);
  
  //mcp_write(63);
  //mcp_data = mcp_read();
  
  eep_write_u16(0x00,&u16_data,2);
  eep_read_u16(0x00,&u16_data_read,2);  
}

void led_show(u8 led,u8 mode)
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
u8 test = 0;
void lcd_part0(void)
{
  sprintf((char*)lcd_text[0],"%d         1              ",test);
  sprintf((char*)lcd_text[1],"volt:%.2f %.2f %.2f            ",r37_volt,r38_volt,mcp_volt);
  sprintf((char*)lcd_text[2],"a15:f:%.1f d:%.1f       ",pa15_fre,pa15_duty);
  sprintf((char*)lcd_text[3],"b4: f:%.1f:              ",pb4_fre);
  sprintf((char*)lcd_text[4],"a7: f:%.1f d:%.1f       ",pa7_fre,pa7_duty);  
  sprintf((char*)lcd_text[5],"ep_u8:%d                ",read_u8);
  sprintf((char*)lcd_text[6],"ep_mul:%d %d %d         ",read_arr[0],read_arr[1],read_arr[2]);
  sprintf((char*)lcd_text[7],"ep_double:%.3f           ",r_double);
  sprintf((char*)lcd_text[8],"mcp:%d                    ",mcp_data);
  sprintf((char*)lcd_text[9],"ep_u16:%d                  ",u16_data_read);
}
void lcd_part1(void)
{
  HAL_RTC_GetTime(&hrtc, &my_sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &my_sDate, RTC_FORMAT_BIN);
  sprintf((char*)lcd_text[0],"Date:%02d-%02d-%02d       ",my_sDate.Year,my_sDate.Month,my_sDate.Date);
  sprintf((char*)lcd_text[1],"Time:%02d-%02d-%02d       ",my_sTime.Hours,my_sTime.Minutes,my_sTime.Seconds);
  sprintf((char*)lcd_text[2],"                       ");
  sprintf((char*)lcd_text[3],"                       ");
  sprintf((char*)lcd_text[4],"                       ");  
  sprintf((char*)lcd_text[5],"                       ");
  sprintf((char*)lcd_text[6],"                       ");
  sprintf((char*)lcd_text[7],"                       ");
  sprintf((char*)lcd_text[8],"                       ");
  sprintf((char*)lcd_text[9],"                       ");
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
    
  for(u8 i= 0; i<10; i++)
  {
    LCD_DisplayStringLine(i*24, lcd_text[i]);
  }
}


void adc_get(void)
{
//  //normal
//  u16 temp = 0;
//  HAL_ADC_Start(&hadc1);
//  temp = HAL_ADC_GetValue(&hadc1);
//  r38_volt = temp*3.3f/4096.0f;
//  
//  HAL_ADC_Start(&hadc1);
//  temp = HAL_ADC_GetValue(&hadc1);
//  mcp_volt = temp*3.3f/4096.0f; 
//  
//  HAL_ADC_Start(&hadc2);
//  temp = HAL_ADC_GetValue(&hadc2);
//  r37_volt = temp*3.3f/4096.0f; 
  
  //dma
  r37_volt = (adc2_buff[0]+adc2_buff[1])*3.3f/2.0f/4096.0f;
  r38_volt = (adc1_buff[0]+adc1_buff[2])*3.3f/2.0f/4096.0f;
  mcp_volt = (adc1_buff[1]+adc1_buff[3])*3.3f/2.0f/4096.0f;
  
}

void while_fun(void)
{
  
  lcd_show();
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  static u8 flag = 0;
  flag++;
  flag %= 2;
  led_show(1,flag);
}
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
  static u8 flag = 0;
  flag++;
  flag %= 2;
  led_show(2,flag);  
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  static u32 tim2_ch1,tim2_ch2;
  static u32 tim16_ch1;
  static u32 tim3_ch1,tim3_ch2;
  if(htim->Instance == TIM2)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      tim2_ch1 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      tim2_ch2 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim2,0);
      pa15_fre = 80000000.0f/(80*(tim2_ch2+1));
      pa15_duty = (tim2_ch1+1)*100.0f/(tim2_ch2+1);
    }
  }
  else if(htim->Instance == TIM16)
  {
    tim16_ch1 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
    __HAL_TIM_SetCounter(&htim16,0);
    pb4_fre = 80000000.0f/(80*(tim16_ch1+1));
  }
  else if(htim->Instance == TIM3)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      tim3_ch1 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      tim3_ch2 = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2);
      __HAL_TIM_SetCounter(&htim3,0);
      pa7_fre = 80000000.0f/(80*(tim3_ch2+1));
      pa7_duty = (tim3_ch1+1)*100.0f/(tim3_ch2+1);
    }
  }
}

static u8 counter15 = 0;
u8 rx_data = 0;
u8 rx_buff[10];
u8 rx_len = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static u32 counter7 = 0;
  if(htim->Instance == TIM7)
  {
    counter7++;
    if(counter7 % 50 == 0)
    {
      //key_scan();
      adc_get();
      key_scan_two();
    }
  }
  else if(htim->Instance == TIM15)
  {
    counter15++;
    if(counter15 >= 10)
    {
      counter15 = 0;
      printf("rx_lent=%d\r\n",rx_len);
      rx_len = 0;
      HAL_TIM_Base_Stop_IT(&htim15);
    }
  }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance  == USART1)
  {
    HAL_TIM_Base_Stop_IT(&htim15);
    counter15 = 0;
    __HAL_TIM_SetCounter(&htim15,0);
    if(rx_len < 10)
    {
      rx_buff[rx_len++] = rx_data;
    }
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);   
    HAL_TIM_Base_Start_IT(&htim15);
  }
}





u8 key_mode[4] = {0,0,0,0};
u8 key_state[4] = {0,0,0,0};
u8 key_last_state[4] = {1,1,1,1};
u32 key_time[4] = {0,0,0,0};
u8 key_flag[4] = {0,0,0,0};
void key_scan_two(void)
{
  key_state[0]=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
  key_state[1]=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
  key_state[2]=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
  key_state[3]=HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
  for(u8 i = 0;i<4;i++)
  {
    switch(key_flag[i])
    {
      case 0:
      {
        if(key_state[i] == 0 && key_last_state[i] == 1)
        {
          key_time[i] = HAL_GetTick();
          key_flag[i]++;
        }
      }break;
      case 1:
      {
        if(key_state[i] == 1 && key_last_state[i] == 1 && (HAL_GetTick() - key_time[i] > 1000))
        {
          key_mode[i] = 2;
          key_flag[i] = 0;
        }
        else if(key_state[i] == 0 && key_last_state[i] == 1 && (HAL_GetTick() - key_time[i] < 300) )
        {
          key_mode[i] = 3;
          key_flag[i] = 0;
        }
        else if(key_state[i] == 1 && key_last_state[i] == 1 && (HAL_GetTick() - key_time[i] > 300) && (HAL_GetTick() - key_time[i] < 1000))
        {
          key_mode[i] = 1;
          key_flag[i] = 0;
        }
      }break;       
    }
  }
  for(u8 i = 0;i<4;i++)
  {
    key_last_state[i] = key_state[i];
  }
//______________key1_mode_________//
  if(key_mode[0])
  {
    if(key_mode[0] == 1)
    {
      lcd_mode++;
      lcd_mode %= 2;      
      test = 1;
    }
    else if(key_mode[0] == 2)
    {
      test = 2;
    }
    else if(key_mode[0] == 3)
    {
      test = 3;
    }    
    key_mode[0] = 0;
  }
//______________key2_mode_________//
  if(key_mode[1])
  {
    if(key_mode[1] == 1)
    {
    
    }
    else if(key_mode[1] == 2)
    {
      
    }
    else if(key_mode[1] == 3)
    {
      
    }    
    key_mode[1] = 0;
  } 
//______________key3_mode_________//
  if(key_mode[2])
  {
    if(key_mode[2] == 1)
    {
    
    }
    else if(key_mode[2] == 2)
    {
      
    }
    else if(key_mode[2] == 3)
    {
      
    }    
    key_mode[2] = 0;
  }
//______________key4_mode_________//
  if(key_mode[3])
  {
    if(key_mode[3] == 1)
    {
    
    }
    else if(key_mode[3] == 2)
    {
      
    }
    else if(key_mode[3] == 3)
    {
      
    }    
    key_mode[3] = 0;
  }    
  
}

void key_scan(void)
{
  key_state[0]=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
  key_state[1]=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
  key_state[2]=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
  key_state[3]=HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
  
  for(u8 i = 0;i<4;i++)
  {
    if(key_state[i] == 0 && key_last_state[i] == 1)
    {
      key_time[i] = HAL_GetTick();
    }
    else if(key_state[i] == 1 && key_last_state[i] == 0)
    {
      if(HAL_GetTick() - key_time[i] > 1000)
      {
        key_mode[i] = 2;
      }
      else
      {
        key_mode[i] = 1;
      }
    }
  }
  for(u8 i = 0;i<4;i++)
  {
    key_last_state[i] = key_state[i];
  }
  
//______________key1_mode_________//
  if(key_mode[0])
  {
    if(key_mode[0] == 1)
    {

    }
    else if(key_mode[0] == 2)
    {
      
    }
    key_mode[0] = 0;
  }
//______________key2_mode_________//
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
//______________key3_mode_________//
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
//______________key4_mode_________//
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
}
