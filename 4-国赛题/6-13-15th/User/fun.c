#include "fun.h"

u16 gpioc_odr = 0;

u8 lcd_mode = 0;
u8 lcd_text[10][35];

float r37_volt = 0;

float fre = 0,duty = 0;

u8 st_arr[4][5]={"Idle","Busy","Wait"};
u8 st_flag = 0;
float cp_x = 0, cp_y = 0;
u16 tp_x = 0, tp_y = 0;
float car_speed = 1.0f;
u16 rn_count = 0;

float param_r = 1.0f;
u8 param_b = 10;
u8 param_flag = 0;

float recd_ts = 0.0f;
u16 recd_tt = 0;


s16 point_arr[200][2];
u8 point_x_y = 0;
u16 point_arr_cnt = 0;
u8 point_arr_flag = 0;
u16 dest_arr[2] = {0,0};
u8 dest_flag = 0;

u8 work_scene = 1;

u8 second_flag = 0;
u8 point_update_flag = 1;
float pp_distance = 0;
float leave_pp_distance = 0;
float sin_deta = 0,cos_deta = 0;
u16 tp_cnt = 0;

u16 counter_second = 0;

void led_show(u8 led, u8 mode)
{
  u16 temp = GPIOC->ODR;
  GPIOC->ODR =gpioc_odr;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
  if(mode)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_RESET);
  else
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 << (led - 1), GPIO_PIN_SET);
  
  gpioc_odr = GPIOC->ODR;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
  GPIOC->ODR = temp;  
}

void pos_ctrl(void)
{
  if(point_arr[tp_cnt][0]!=-1 && point_arr[tp_cnt][1]!=-1)
  {
    tp_x = point_arr[tp_cnt][0];
    tp_y = point_arr[tp_cnt][1];
    //printf("cnt:%d,d:%d,%d\r\n",tp_cnt,tp_x,tp_y);
    
  }
//  if(point_arr[tp_cnt][0]==-1 && point_arr[tp_cnt][1]==-1)
//  {
//     if(point_arr_cnt == 0)
//     {
//     
//     }
//     else
//     {
//      
//     }
//    //printf("cnt:%d,d:%d,%d\r\n",tp_cnt,tp_x,tp_y);
//    
//  } 
//  else
//  {
//    tp_x = point_arr[tp_cnt][0];
//    tp_y = point_arr[tp_cnt][1];    
//  }
  
  if(st_flag == 1)
  {
    // 更新TP后，需要计算当前路径的方向参数
    if(point_update_flag == 1)
    {
      point_update_flag = 0;
      pp_distance = sqrt((tp_x - cp_x)*(tp_x - cp_x)+(tp_y - cp_y)*(tp_y - cp_y));
      sin_deta = (tp_y - cp_y)/pp_distance;
      cos_deta = (tp_x - cp_x)/pp_distance;
      leave_pp_distance = pp_distance;
    }
    
    if(second_flag == 1 && dest_flag)
    {
      second_flag = 0;
      leave_pp_distance = leave_pp_distance-car_speed;
      recd_tt++;
      recd_ts +=car_speed;
      if(leave_pp_distance <= 0)//到下一个目标点了
      {
        point_update_flag = 1;        
        cp_x = tp_x;
        cp_y = tp_y;
        point_arr[tp_cnt][0] = -1;
        point_arr[tp_cnt][1] = -1;
        rn_count--;
        if(cp_x == dest_arr[0] && cp_y == dest_arr[1])//当前点为最后一个点了
        {
          dest_arr[0] = 0;
          dest_arr[1] = 0;
          tp_cnt = 0;
          st_flag = 0;
          point_arr_cnt = 0;
          memset(point_arr,-1,sizeof(point_arr));
          dest_flag = 0;
        }
        else
        {
          tp_cnt++;
          if(tp_cnt >= 200)
          {
            tp_cnt = 0;
          }
          tp_x = point_arr[tp_cnt][0];
          tp_y = point_arr[tp_cnt][1];          
        }

        
      }
      else
      {
        cp_x +=(car_speed*cos_deta);
        cp_y +=(car_speed*sin_deta);
      }
    }
  }
}

void special_part(void)
{
  if(st_flag ==0)
  {
    sprintf((char*)lcd_text[5],"     TP=NF               ");
    sprintf((char*)lcd_text[7],"     RN=NF                  ");
  }
    
  else
  {
    sprintf((char*)lcd_text[5],"     TP=%d,%d               ",tp_x,tp_y);
    sprintf((char*)lcd_text[7],"     RN=%d                  ",rn_count);
  } 

}
void lcd_part0(void)
{
  sprintf((char*)lcd_text[0],"                       ");
  sprintf((char*)lcd_text[1],"        DATA               ");
  sprintf((char*)lcd_text[2],"                       ");
  sprintf((char*)lcd_text[3],"     ST=%s                  ",st_arr[st_flag]);
  sprintf((char*)lcd_text[4],"     CP=%.0f,%.0f               ",cp_x,cp_y);
  //sprintf((char*)lcd_text[5],"     TP=%d,%d               ",tp_x,tp_y);
  sprintf((char*)lcd_text[6],"     SE=%.1f                ",car_speed);
  //sprintf((char*)lcd_text[7],"     RN=%d                  ",rn_count);
  special_part();
  sprintf((char*)lcd_text[8],"                       ");
  sprintf((char*)lcd_text[9],"                       ");   
//  sprintf((char*)lcd_text[8],"f:%.2f  d:%.2f         ",fre,duty);
//  sprintf((char*)lcd_text[9],"r37:%.2f                 ",r37_volt);  
}
void lcd_part1(void)
{
  sprintf((char*)lcd_text[0],"                       ");
  sprintf((char*)lcd_text[1],"        PARA               ");
  sprintf((char*)lcd_text[2],"                       ");
  sprintf((char*)lcd_text[3],"     R=%.1f                  ",param_r);
  sprintf((char*)lcd_text[4],"     B=%d                  ",param_b);
  sprintf((char*)lcd_text[5],"                       ");
  sprintf((char*)lcd_text[6],"                       ");
  sprintf((char*)lcd_text[7],"                       ");
  sprintf((char*)lcd_text[8],"                       ");
  sprintf((char*)lcd_text[9],"                       ");   
}
void lcd_part2(void)
{
  sprintf((char*)lcd_text[0],"                       ");
  sprintf((char*)lcd_text[1],"        RECD               ");
  sprintf((char*)lcd_text[2],"                       ");
  sprintf((char*)lcd_text[3],"     TS=%.1f               ",recd_ts);
  sprintf((char*)lcd_text[4],"     TT=%d                 ",recd_tt);
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
  else if(lcd_mode == 2)
  {
    lcd_part2();
  }
  for(u8 i =0; i<10;i++)
  {
    LCD_DisplayStringLine(i*24, lcd_text[i]);
  }  
}

void adc_get(void)
{
  HAL_ADC_Start(&hadc2);
  u16 temp = HAL_ADC_GetValue(&hadc2);
  r37_volt = 3.3*temp/4096.0f;
}
float pa6_fre_a = 1000,pa6_duty_a = 0;
float pa7_fre_b = 1000,pa7_duty_b = 0;
void duty_ctrl(void)
{
  
  if(r37_volt < 0.50f)
  {
    pa6_duty_a = 0.1;
    pa7_duty_b = 0.05;
  }
  else if(r37_volt>=0.50f && r37_volt <1.00f)
  {
    pa6_duty_a = 0.30;
    pa7_duty_b = 0.25;    
  }
  else if(r37_volt>=1.00f && r37_volt <1.50f)
  {
    pa6_duty_a = 0.50;
    pa7_duty_b = 0.45;    
  }
  else if(r37_volt>=1.50f && r37_volt <2.00f)
  {
    pa6_duty_a = 0.70;
    pa7_duty_b = 0.65;    
  }
  else if(r37_volt>=2.00f && r37_volt <2.50f)
  {
    pa6_duty_a = 0.90;
    pa7_duty_b = 0.85;    
  }
  else if(r37_volt>=2.50f)
  {
    pa6_duty_a = 0.95;
    pa7_duty_b = 0.90;    
  }  
}
void fre_ctrl(void)
{
  switch(work_scene)
  {
    case 1:
    {
      pa6_fre_a = 1000;
      pa7_fre_b = 1000;
    }break;
    case 2:
    {
      pa6_fre_a = 4000;
      pa7_fre_b = 1000;    
    }break;
    case 3:
    {
      pa6_fre_a = 1000;
      pa7_fre_b = 4000;    
    }break;
    case 4:
    {
      pa6_fre_a = 4000;
      pa7_fre_b = 4000;    
    }break; 
  }      
}

void pwm_ctrl(void)
{
  static u8 work_last = 0;
  if(st_flag == 1)
  {
    duty_ctrl();
    fre_ctrl();    
    __HAL_TIM_SetAutoreload(&htim3,(1000000/pa6_fre_a)-1);
    __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,((1000000/pa6_fre_a)*pa6_duty_a));    
    if(work_last != work_scene)
    {
      __HAL_TIM_SetCounter(&htim3,0);
    }
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    
    __HAL_TIM_SetAutoreload(&htim17,(1000000/pa7_fre_b)-1);
    __HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,((1000000/pa7_fre_b)*pa7_duty_b));   
    if(work_last != work_scene)
    {
      __HAL_TIM_SetCounter(&htim17,0);
    }
    HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
    work_last = work_scene;
  }
  else
  {
    __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,((1000000/pa6_fre_a)*0.00f));
    __HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,((1000000/pa7_fre_b)*0.00f));
  }
 
}

void speed_ctrl(void)
{
  if(st_flag == 1)
    car_speed = 3.14f*param_r*fre/100.0f+param_b;
  else
    car_speed = 0.0f;
}

void other_led_ctrl(void)
{
  switch(work_scene)
  {
    case 1:
    {
      led_show(5,1);
      led_show(6,0);
      led_show(7,0);
      led_show(8,0);
    }break;
    case 2:
    {
      led_show(5,0);
      led_show(6,1);
      led_show(7,0);
      led_show(8,0);      
    }break;
    case 3:
    {
      led_show(5,0);
      led_show(6,0);
      led_show(7,1);
      led_show(8,0);      
    }break;
    case 4:
    {
      led_show(5,0);
      led_show(6,0);
      led_show(7,0);
      led_show(8,1);      
    }break;    
  }
  if(st_flag == 0)
    led_show(1,0);
  else if(st_flag == 1)
    led_show(1,1);
}

void while_fun(void)
{
  adc_get();
  pwm_ctrl();
  pos_ctrl();
  other_led_ctrl();
  speed_ctrl();
  
  lcd_show();
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  static u32 ch1 = 0;
  static u32 ch2 = 0;
  if(htim->Instance ==TIM2)
  {
    ch1 = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_SetCounter(&htim2,0);
    fre = 80000000/(80*(ch1+1));
//    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//    {
//      ch1 = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
//    }
//    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
//    {
//      ch2 = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
//      __HAL_TIM_SetCounter(&htim2,0);
//      fre = 80000000/(80*(ch2+1));
//      duty = (ch1+1)*100.0f/(ch2+1);
//    }
  }
}

void led1_ctrl(void)
{
  static u8 counter = 0;
  static u8 flag = 0;
  if(st_flag == 2)
  {
    counter++;
    if(counter % 4 == 0)
    {
      flag++;
      flag %= 2;
      led_show(1,flag);
      counter = 0;      
    }

  }
}


static u32 counter15 = 0;
u8 rx_data = 0;
u8 rx_buff[100];
u8 rx_len = 0;
u8 rx_flag = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static u32 counter7 = 0;
  
  if(htim->Instance == TIM7)
  {
    counter7++;
    if(counter7 % 50 == 0)
    {
      key_scan();
      led1_ctrl();
    }
    if(st_flag == 1 && second_flag==0)
    {
      counter_second++;
      if(counter_second >= 1000)
      {
        counter_second = 0;
        second_flag = 1;
      }        
    }    
//    if(counter7 % 150 == 0)
//    {
//      pwm_ctrl();
//    }      
  }
  else if(htim->Instance == TIM15)
  {
    counter15++;
    if(counter15 >= 10)
    {
      HAL_TIM_Base_Stop_IT(&htim15);
      counter15 = 0;
      uart_prepare();
      uart_ctrl();
      memset(rx_buff,0,sizeof(rx_buff));
      rx_len = 0;
    }
  }
}

void uart_prepare(void)
{
  //1
  if(rx_buff[0] =='(' && rx_buff[rx_len-1] ==')')//ji shu
  {
    rx_flag = 1;
  }
  else if(rx_buff[0] =='{' && rx_buff[rx_len-1] =='}')//2
  {
    rx_flag = 2;
  }
  else if(rx_buff[0] =='[' && rx_buff[2] ==']' && rx_len==3)//3
  {
    rx_flag = 3;
  }  
  else if(rx_buff[0] =='?' && rx_len == 1)//4
  {
    rx_flag = 4;
  }
  else if(rx_buff[0] =='#' && rx_len == 1)//5
  {
    rx_flag = 5;
  }
  //printf("rx_flag = %d",rx_flag); 
}

void uart_ctrl(void)
{
  u16 douhao_cnt = 0;
  u16 temp = 0;
  switch(rx_flag)
  {
    case 1:
    {
      for(u16 i = 0;i<rx_len;i++)
      {
        if(rx_buff[i] == ',')
          douhao_cnt++;
      }
      if(douhao_cnt % 2 == 1)//有n个坐标点，不存在某个点只有x或只有y(不考虑逗号错误)
      {
        if(st_flag == 0)//空闲状态
        {
          // 设置途径，目标点, 尚未检查中间数据格式不符合的情况
          for(u16 i = 1;i<rx_len;i++)
          {
            if(rx_buff[i] != ',' && rx_buff[i] != ')')//数
            {
              temp = temp*10+rx_buff[i] - '0';
            }
            else if(rx_buff[i] == ',')
            {
              point_arr[point_arr_cnt][point_x_y] = temp;
              point_x_y++;
              point_x_y %= 2;
              if(point_x_y == 0)
              {
                point_arr_cnt++;                
              }
              temp = 0;             
            }
            else if(rx_buff[i] == ')')//最后一个
            {
              point_arr[point_arr_cnt][point_x_y] = temp;
              point_x_y++;
              point_x_y %= 2;
              if(point_x_y == 0)
              {
                point_arr_cnt++;                
              }
              temp = 0;              
            }
          }
          rn_count = point_arr_cnt;
          dest_arr[0] = point_arr[point_arr_cnt-1][0];
          dest_arr[1] = point_arr[point_arr_cnt-1][1];
          printf("Got it");
          dest_flag = 1;
//          printf("d:%d,%d\r\n",dest_arr[0],dest_arr[1]);
//          printf("d:%d,%d\r\n",point_arr[point_arr_cnt-1][0],point_arr[point_arr_cnt-1][1]);
        }
        else
        {
          printf("Busy");
        }
      }
      else
      {
        printf("Error");
      }
    }break;
    case 2:
    {
      u16 point[2] = {0},point_cnt = 0;
      u16 i = 0;
      u16 delete_cnt = 0;
      for(i = 1;i<rx_len;i++)
      {
        if(rx_buff[i] != ',' && rx_buff[i] != '}')//数
        {
          temp = temp*10+rx_buff[i] - '0';
        }
        else if(rx_buff[i] == ',')//第一个数
        {
          point[point_cnt] = temp;
          point_cnt++;
          temp = 0;
        }
        else if(rx_buff[i] == '}')//第二个数
        {
          point[point_cnt] = temp;
          temp = 0;
        }
      }
      for(i = 0;i<point_arr_cnt;i++)
      {
        if(point_arr[i][0] == point[0] && point_arr[i][1] == point[1])
        {
          delete_cnt = i;
          for(;delete_cnt<point_arr_cnt-1;delete_cnt++)
          {
            point_arr[delete_cnt][0] = point_arr[delete_cnt+1][0];
            point_arr[delete_cnt][1] = point_arr[delete_cnt+1][1];
          }
          point_arr_cnt--;       
          break;
        }
      }      
      if(point_arr_cnt != rn_count)
      {
        rn_count = point_arr_cnt;
        dest_arr[0] = point_arr[point_arr_cnt-1][0];
        dest_arr[1] = point_arr[point_arr_cnt-1][1];        
        printf("Got it");
      }      
      else
      {
        printf("Nonexistent");
      }        
    }break;
    case 3:
    {
      if(st_flag == 1)
      {
        u8 temp = 0;
        temp = rx_buff[1] - '0';
        if((temp >=1) &&(temp <= 4) )
        {
          work_scene = temp;
          printf("Got it");
        }
        else
          printf("Device offline");
      }
      else
        printf("Device offline");
    }break;
    case 4:
    {
      switch(st_flag)
      {
        case 0:
          printf("Idle");break;
        case 1:
          printf("Busy");break;
        case 2:
          printf("Wait");break;        
      }
    }break;
    case 5:
    {
      printf("(%.0f,%.0f)",cp_x,cp_y);
    }break;
    default:
    {
      printf("Error");
    }break;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    HAL_TIM_Base_Stop_IT(&htim15);
    counter15 = 0;
    __HAL_TIM_SetCounter(&htim15,0);
    if(rx_len < 100)
    {
      rx_buff[rx_len]=rx_data;
      rx_len++;
    }
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    HAL_TIM_Base_Start_IT(&htim15);    
  }    
}


u8 key_mode[4];
u8 key_state[4] = {0,0,0,0};
u8 key_last_state[4] = {1,1,1,1};
u32 key_time[4] = {0,0,0,0};
u8 key3_flag = 0;
u8 key4_flag = 0;
u32 key34_time = 0;
void key_scan(void)
{
  key_state[0] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
  key_state[1] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
  key_state[2] = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
  key_state[3] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
  
  for(u8 i = 0; i<4;i++)
  {
    if(key_state[i] == 0 && key_last_state[i] == 1)
    {
      key_time[i] = HAL_GetTick();
    }
    else if(key_state[i] == 1 && key_last_state[i] == 0)
    {
      if(HAL_GetTick() - key_time[i] >2000)
      {
        key_mode[i] = 2;
      }
      else
        key_mode[i] = 1;        
    }
  }
  for(u8 i = 0; i<4;i++)
  {
    key_last_state[i] = key_state[i];
  }
  
//_______________key1_mode__________________
  if(key_mode[0])
  {
    if(key_mode[0] == 1)
    {
      if(lcd_mode == 0)
      {
        if(dest_flag == 0 && st_flag == 0)
        {
          st_flag = 0;
        }
        else if(dest_flag == 1 && st_flag == 0)
        {
          st_flag = 1;
          //counter_second = 0;
        }
        else if(st_flag == 1)
        {
          st_flag = 2;
          //counter_second = 0;
        }
        else if(st_flag == 2)
        {
          st_flag = 1;
          //counter_second = 0;
        }        
      }
     
    }
    else if(key_mode[0] == 2)
    {
      
    }
    key_mode[0] = 0;
  }
//_______________key2_mode__________________
  if(key_mode[1])
  {
    if(key_mode[1] == 1)
    {
      lcd_mode++;
      if(lcd_mode == 2)
        param_flag = 0;
      
      lcd_mode %= 3;
    }
    else if(key_mode[1] == 2)
    {
      
    }
    key_mode[1] = 0;
  }
//_______________key3_mode__________________
  if(key_mode[2])
  {
    if(key_mode[2] == 1)
    {
      if(lcd_mode == 1)
      {
        param_flag++;
        param_flag %= 2;          
      }

    }
    else if(key_mode[2] == 2)
    {
      if(lcd_mode == 0)
      {
        key34_time = HAL_GetTick();
        key3_flag++;
      }
        
    }
    key_mode[2] = 0;
  }  
//_______________key4_mode__________________
  if(key_mode[3])
  {
    if(key_mode[3] == 1)
    {
       if(lcd_mode == 1)
      {
        if(param_flag == 0)
        {
          param_r+=0.1f;
          if(param_r > 2.05f)
            param_r = 1.0f;
        }
        else if(param_flag == 1)
        {
          param_b+=10;
          if(param_b > 100)
            param_b = 10;       
        }           
      }

    }
    else if(key_mode[3] == 2)
    {
      if(lcd_mode == 0)
      {
        key34_time = HAL_GetTick();
        key4_flag++;
      }
       
    }
    key_mode[3] = 0;
  }
  if(key3_flag == 1 && key4_flag==1)
  {
    //led_mode(1,1);
    key3_flag = 0;
    key4_flag = 0;
    recd_ts = 0;
    recd_tt = 0;
  }
  if((HAL_GetTick() - key34_time) > 1000 &&(key3_flag==1 || key4_flag==1))
  {
    key3_flag = 0;
    key4_flag = 0;    
  }
  
  
}


