#include "fun.h"

u16 gpio_odr = 0;

RTC_TimeTypeDef my_sTime = {0};
RTC_DateTypeDef my_sDate = {0};

u8 lcd_mode = 0;
u8 lcd_text[10][40];

float pa7_fre = 0;
float pa7_duty = 0;
float pa15_fre = 0;
u8 st_mode[2][7] = {"UNLOCK","LOCK"};
u8 st_mode_flag = 0;

float pa7_fre_rec = 0;
float pa7_duty_rec = 0;
u32 pa15_fre_rec = 0;
u8 st_mode_flag_rec = 0;
float fre_xf = 0;
u8 hour_rec = 0;
u8 minu_rec = 0;
u8 sec_rec = 0;

s16 param_ds = 1;
s16 param_dr = 80;
s16 param_fs = 100;
s16 param_fr = 2000;
s16 param_ds_set = 1;
s16 param_dr_set = 80;
s16 param_fs_set = 100;
s16 param_fr_set = 2000;
u8 param_ctrl_flag = 0;

float r37_volt = 0;
float r38_volt = 0;

u8 time_str[10]={'0','0','H','0','0','M','0','0','S'};
u8 time_str_rec[10]={'0','0','H','0','0','M','0','0','S'};


u8 param_update = 0;

u8 error_update_flag = 0;

void my_init(void)
{
  //my_sTime.Hours
}


void led_show(u8 led, u8 mode)
{
  u16 temp = GPIOC->ODR;
  GPIOC->ODR = gpio_odr;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
  if(mode)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_RESET);
  else
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_SET);
  
  gpio_odr = GPIOC->ODR;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET); 
  GPIOC->ODR = temp;
}

//void time_show_ctrl(void)
//{
//  for(u8 i = 0; i < 3;i++)
//  {
//    LCD_DisplayChar(Line7,320-i*16,lcd_text[7][i]);
//  }
//  
//  for(u8 i = 3; i < 3;i++)
//  {
//    LCD_DisplayChar(Line7,320-i*16,lcd_text[7][i]);
//  }  
//  
//}

void lcd_part0(void)
{
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"       PWM          ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"   CF=%.0fHz          ",pa7_fre);
  sprintf((char *)lcd_text[4],"   CD=%.0f%%        ",pa7_duty);
  sprintf((char *)lcd_text[5],"   DF=%.0fHz          ",pa15_fre);
  sprintf((char *)lcd_text[6],"   ST=%s            ",st_mode[st_mode_flag]);
  sprintf((char *)lcd_text[7],"   %s               ",time_str);
  sprintf((char *)lcd_text[8],"                    ");
  sprintf((char *)lcd_text[9],"                    ");
}
void lcd_part1(void)
{
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"       RECD         ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"   CF=%.0fHz          ",pa7_fre_rec);
  sprintf((char *)lcd_text[4],"   CD=%.0f%%        ",pa7_duty_rec);
  sprintf((char *)lcd_text[5],"   DF=%dHz          ",pa15_fre_rec);
  sprintf((char *)lcd_text[6],"   XF=%.0fHz          ",fre_xf);
  sprintf((char *)lcd_text[7],"   %s               ",time_str_rec);
  sprintf((char *)lcd_text[8],"                    ");
  sprintf((char *)lcd_text[9],"                    ");
}
void lcd_part2(void)
{
  sprintf((char *)lcd_text[0],"                    ");
  sprintf((char *)lcd_text[1],"       PARA         ");
  sprintf((char *)lcd_text[2],"                    ");
  sprintf((char *)lcd_text[3],"   DS=%d%%          ",param_ds_set);
  sprintf((char *)lcd_text[4],"   DR=%d%%          ",param_dr_set);
  sprintf((char *)lcd_text[5],"   FS=%dHz          ",param_fs_set);
  sprintf((char *)lcd_text[6],"   FR=%dHz          ",param_fr_set);
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
  else if(lcd_mode == 2)
  {
    lcd_part2();
  }
  for(u8 i = 0; i< 10; i++)
  {
    LCD_DisplayStringLine(i*24,lcd_text[i]);
  }
}

void volt_get(void)
{
  u32 temp;
  HAL_ADC_Start(&hadc2);
  temp = HAL_ADC_GetValue(&hadc2);
  r37_volt = 3.3f*temp/4096.0f;
  HAL_ADC_Start(&hadc1);
  temp = HAL_ADC_GetValue(&hadc1); 
  r38_volt = 3.3f*temp/4096.0f;
}

void fre_ctrl(void)
{
  u32 fre_fanwei = 0;
  float fre_ctrl_cnt = 0;
  float volt_ctrl_single2 = 0;
  u32 volt_cnt2 = 0;
  u32 fre_set = 0;
  
  u8 duty_fanwei = 0;
  float duty_ctrl_cnt = 0;
  float volt_ctrl_single = 0;
  u8 volt_cnt = 0;
  float duty_set = 0;   
//  fre_fanwei = 2000 - 1000;
//  fre_ctrl_cnt = 10+1;
//  volt_ctrl_single2 = 3.3f/fre_ctrl_cnt;
//  volt_cnt2 = r38_volt/volt_ctrl_single2;
//  fre_set = 1000+1*100;  

  if(param_update)
  {
      param_ds=param_ds_set;
     param_dr=param_dr_set;
     param_fs=param_fs_set;
     param_fr=param_fr_set;
    param_update = 0;
  }
  
  fre_fanwei = param_fr - 1000;
  fre_ctrl_cnt = fre_fanwei/param_fs+1;
  volt_ctrl_single2 = 3.3f/fre_ctrl_cnt;
  volt_cnt2 = r38_volt/volt_ctrl_single2;
  fre_set = 1000+volt_cnt2*param_fs;
//  duty_fanwei = 80 - 10;
//  duty_ctrl_cnt = 70/7 + 1;
//  volt_ctrl_single = 3.3f/(duty_ctrl_cnt - 1);
//  volt_cnt = r37_volt/(volt_ctrl_single);
//  duty_set = (10+volt_cnt*7)/100.0f; 
  duty_fanwei = param_dr - 10;
  duty_ctrl_cnt = duty_fanwei/param_ds+1;
  volt_ctrl_single = 3.3f/duty_ctrl_cnt;
  volt_cnt = r37_volt/volt_ctrl_single;
  duty_set = (10+volt_cnt*param_ds)/100.0f;

  
  __HAL_TIM_SetAutoreload(&htim3,(80000000.0f/(80*fre_set))-1);
  __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,((80000000.0f/(80*fre_set))-1)*duty_set);
  __HAL_TIM_SetCounter(&htim3,0);  
  
  pa7_fre = fre_set;
  pa7_duty = duty_set*100.0f;
  
}

void error_check(void)
{
  if(fabs(pa15_fre -pa7_fre)>1000 && error_update_flag == 0)
  {
    pa7_fre_rec = pa7_fre;
    pa7_duty_rec = pa7_duty;
    pa15_fre_rec = pa15_fre;
    if(pa15_fre - pa7_fre > 1000)
      fre_xf = pa15_fre - pa7_fre;
    else
      fre_xf =pa7_fre-pa15_fre;
    
    strcpy((char *)time_str_rec,(char *)time_str);
//    hour_rec = my_sTime.Hours;
//    minu_rec = my_sTime.Minutes;
//    sec_rec = my_sTime.Seconds;
    error_update_flag = 1;
    led_show(3,1);
  }
  else if(fabs(pa15_fre -pa7_fre)<1000)
  {
    error_update_flag = 0;
    led_show(3,0);
  }
}

void led_ctrl(void)
{
  if(lcd_mode == 0)
  {
    led_show(1,1);
  }
  else
  {
    led_show(1,0);
  }
  
  if(st_mode_flag)
  {
    led_show(2,1);
  }
  else
  {
    led_show(2,0);
  }
  
}


void while_fun(void)
{ 
  HAL_RTC_GetTime(&hrtc,&my_sTime,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,&my_sDate,RTC_FORMAT_BIN);
  error_check();
  led_ctrl();
  volt_get();
  lcd_show();
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static u32 counter7 = 0;
  static u8 time_update = 1;
  if(htim->Instance == TIM7)
  {
    counter7++;
    if(counter7 % 50 == 0)
    {
      key_scan();
    }
    if(st_mode_flag==0 && counter7 % 200 == 0)
    {
       fre_ctrl();
      if(time_update != my_sTime.Seconds)
      {
        if(my_sTime.Seconds < 10)
        {
          time_str[7] = my_sTime.Seconds+'0';
        }
        else
        {
          time_str[6] = my_sTime.Seconds/10+'0';
          time_str[7] = my_sTime.Seconds%10+'0';
        } 
        
        if(my_sTime.Minutes < 10)
        {
          time_str[4] = my_sTime.Minutes+'0';
        }
        else
        {
          time_str[3] = my_sTime.Minutes/10+'0';
          time_str[4] = my_sTime.Minutes%10+'0';
        } 
        
        if(my_sTime.Hours < 10)
        {
          time_str[1] = my_sTime.Hours+'0';
        }
        else
        {
          time_str[0] = my_sTime.Hours/10+'0';
          time_str[1] = my_sTime.Hours%10+'0';
        }         
      }

      
      time_update = my_sTime.Seconds;
      
    }
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM8)
  {
    u32 temp = HAL_TIM_ReadCapturedValue(&htim8,TIM_CHANNEL_1);
    __HAL_TIM_SetCounter(&htim8,0);
    pa15_fre = 80000000.0f/(80*(temp+1));
  }
}


void param_check_ctrl(void)
{
  if(((param_ds_set*2) <= param_dr_set) && ((param_fs_set*2) <= param_fr_set) 
    && param_ds_set > 0 && param_dr_set > 0 && param_fs_set > 0 &&param_fr_set > 0 && param_dr_set < 100)
  {
    param_update = 1;  
  }

}

u8 key_state[4] = {0};
u8 key_last_state[4] = {1,1,1,1};
u8 key_mode[4] = {0};
u32 key_press_time[4] = {0};

void key_scan(void)
{
  key_state[0]= HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
  key_state[1]= HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
  key_state[2]= HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
  key_state[3]= HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
  
  for(u8 i =0; i<4; i++)
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
        if(HAL_GetTick() - key_press_time[i] > 2000)
        {
          key_mode[i] = 2;
        }
        else
        {
          key_mode[i] = 1;
        }
      }
    }
    key_last_state[i] = key_state[i];
  }  
//  for(u8 i =0; i<4; i++)
//  {
//    
//  }
  
//_____________________key1_mode_________________________//

  if(key_mode[0])
  {
    if(key_mode[0] == 1)
    {
      lcd_mode++;
      if(lcd_mode == 3)
      {
        lcd_mode = 0;
        param_ctrl_flag = 0;
        param_check_ctrl();
      }
        
      
    }
    else
    {
      
    }
  }
//_____________________key2_mode_________________________//

  if(key_mode[1])
  {
    if(key_mode[1] == 1)
    {
      if(lcd_mode == 2)
      {
        param_ctrl_flag++;
        if(param_ctrl_flag == 4)
        {
          param_ctrl_flag = 0;
        }
      }
      if(lcd_mode == 0)
      {
        st_mode_flag++;
        if(st_mode_flag == 2)
          st_mode_flag = 0;
      }
    }
    else
    {
      if(lcd_mode == 0)
      {
        //Çå¿ÕÊ±¼ä
        RTC_TimeTypeDef clear_sTime = {0};
        u8 time_str_clear[10]={'0','0','H','0','0','M','0','0','S'};
        HAL_RTC_SetTime(&hrtc,&clear_sTime,RTC_FORMAT_BIN);
        strcpy((char*)time_str,(char*)time_str_clear);
        
      }
    }
  }  
//_____________________key3_mode_________________________//

  if(key_mode[2])
  {
    if(key_mode[2] == 1)
    {
      if(lcd_mode == 2)
      {
        switch(param_ctrl_flag)
        {
          case 0:
          {
            param_ds_set++;
          }break;
          case 1:
          {
            param_dr_set+=10;
          }break;
          case 2:
          {
            param_fs_set+=100;
          }break;
          case 3:
          {
            param_fr_set+=1000;
          }break;           
        }
      }
    }
    else
    {
      
    }
  }
//_____________________key4_mode_________________________//

  if(key_mode[3])
  {
    if(key_mode[3] == 1)
    {
      if(lcd_mode == 2)
      {
        switch(param_ctrl_flag)
        {
          case 0:
          {
            param_ds_set--;
          }break;
          case 1:
          {
            param_dr_set-=10;
          }break;
          case 2:
          {
            param_fs_set-=100;
          }break;
          case 3:
          {
            param_fr_set-=1000;
          }break;           
        }
      }      
    }
    else
    {
      
    }
  }

  
  for(u8 i =0; i<4; i++)
  {
    key_mode[i] = 0;
  }  
}






