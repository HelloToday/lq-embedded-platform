/*
  程序说明: CT117E-M4嵌入式竞赛板GPIO模拟I2C总线驱动程序
  软件环境: MDK-ARM HAL库
  硬件环境: CT117E-M4嵌入式竞赛板
  日    期: 2020-3-1
*/

#include "i2c_hal.h"

#define DELAY_TIME	20

/**
  * @brief SDA线输入模式配置
  * @param None
  * @retval None
  */
void SDA_Input_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief SDA线输出模式配置
  * @param None
  * @retval None
  */
void SDA_Output_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief SDA线输出一个位
  * @param val 输出的数据
  * @retval None
  */
void SDA_Output( uint16_t val )
{
    if ( val )
    {
        GPIOB->BSRR |= GPIO_PIN_7;
    }
    else
    {
        GPIOB->BRR |= GPIO_PIN_7;
    }
}

/**
  * @brief SCL线输出一个位
  * @param val 输出的数据
  * @retval None
  */
void SCL_Output( uint16_t val )
{
    if ( val )
    {
        GPIOB->BSRR |= GPIO_PIN_6;
    }
    else
    {
        GPIOB->BRR |= GPIO_PIN_6;
    }
}

/**
  * @brief SDA输入一位
  * @param None
  * @retval GPIO读入一位
  */
uint8_t SDA_Input(void)
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET){
		return 1;
	}else{
		return 0;
	}
}


/**
  * @brief I2C的短暂延时
  * @param None
  * @retval None
  */
static void delay1(unsigned int n)
{
    uint32_t i;
    for ( i = 0; i < n; ++i);
}

/**
  * @brief I2C起始信号
  * @param None
  * @retval None
  */
void I2CStart(void)
{
    SDA_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SDA_Output(0);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);
}

/**
  * @brief I2C结束信号
  * @param None
  * @retval None
  */
void I2CStop(void)
{
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output(0);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SDA_Output(1);
    delay1(DELAY_TIME);

}

/**
  * @brief I2C等待确认信号
  * @param None
  * @retval None
  */
unsigned char I2CWaitAck(void)
{
    unsigned short cErrTime = 5;
    SDA_Input_Mode();
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    while(SDA_Input())
    {
        cErrTime--;
        delay1(DELAY_TIME);
        if (0 == cErrTime)
        {
            SDA_Output_Mode();
            I2CStop();
            return ERROR;
        }
    }
    SDA_Output_Mode();
    SCL_Output(0);
    delay1(DELAY_TIME);
    return SUCCESS;
}

/**
  * @brief I2C发送确认信号
  * @param None
  * @retval None
  */
void I2CSendAck(void)
{
    SDA_Output(0);
    delay1(DELAY_TIME);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);

}

/**
  * @brief I2C发送非确认信号
  * @param None
  * @retval None
  */
void I2CSendNotAck(void)
{
    SDA_Output(1);
    delay1(DELAY_TIME);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);

}

/**
  * @brief I2C发送一个字节
  * @param cSendByte 需要发送的字节
  * @retval None
  */
void I2CSendByte(unsigned char cSendByte)
{
    unsigned char  i = 8;
    while (i--)
    {
        SCL_Output(0);
        delay1(DELAY_TIME);
        SDA_Output(cSendByte & 0x80);
        delay1(DELAY_TIME);
        cSendByte += cSendByte;
        delay1(DELAY_TIME);
        SCL_Output(1);
        delay1(DELAY_TIME);
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
}

/**
  * @brief I2C接收一个字节
  * @param None
  * @retval 接收到的字节
  */
unsigned char I2CReceiveByte(void)
{
    unsigned char i = 8;
    unsigned char cR_Byte = 0;
    SDA_Input_Mode();
    while (i--)
    {
        cR_Byte += cR_Byte;
        SCL_Output(0);
        delay1(DELAY_TIME);
        delay1(DELAY_TIME);
        SCL_Output(1);
        delay1(DELAY_TIME);
        cR_Byte |=  SDA_Input();
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output_Mode();
    return cR_Byte;
}

//
void I2CInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}



void eeprom_write_buff(uint8_t adr, uint8_t* w_buff, uint8_t lenth)
{
  I2CStart();
  I2CSendByte(0xa0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  while(lenth)
  {
    I2CSendByte(*w_buff);
    I2CWaitAck();
    w_buff++;
    lenth--;
  }
  I2CStop();
  HAL_Delay(10);
}

void eeprom_read_buff(uint8_t adr, uint8_t* r_buff, uint8_t lenth)
{
  I2CStart();
  I2CSendByte(0xa0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  I2CStart();
  I2CSendByte(0xa1);
  I2CWaitAck();
  while(lenth)
  {
    *r_buff = I2CReceiveByte();
    if(lenth == 1)
    {
      I2CSendNotAck();
      lenth--;
     
    }
    else
    {
      I2CSendAck();      
      r_buff++;
      lenth--;
    }

  }
  
  I2CStop();
}



//________________________float______________________________

void eeprom_write_buff_float(uint8_t adr,double* w_double)
{
  uint8_t lenth = 8;
  uint8_t* double_adr = (uint8_t *)w_double;
  
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  while(lenth)
  {
    I2CSendByte(*double_adr);
    I2CWaitAck();
    lenth--;
    double_adr++;
  }
  
  I2CStop();
  HAL_Delay(10);
  
}


void eep_write_double(uint8_t adr,double* w_double)
{
  uint8_t lenth = 8;
  uint8_t* double_str = (uint8_t*)w_double;
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  while(lenth)
  {
    I2CSendByte(*double_str);
    I2CWaitAck();
    double_str++;
    lenth--;   
  }
  
  I2CStop();
  HAL_Delay(10);
}


void eeprom_read_buff_float(uint8_t adr,double* r_double)
{
  uint8_t lenth = 8;
  uint8_t *double_str = (uint8_t *)r_double;
  
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  I2CStart();
  I2CSendByte(0xA1);
  I2CWaitAck();
  
  while(lenth)
  {
    *double_str = I2CReceiveByte();
    if(lenth == 1)
    {
      lenth--;
      I2CSendNotAck();
    }
    else
    {
      
      I2CSendAck();
      double_str++;
      lenth--;
    }
  }
  I2CStop();  
  
}

void eep_read_double(uint8_t adr,double * r_double)
{
  uint8_t len = 8;
  uint8_t* double_str = (uint8_t*)r_double;
  
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  I2CStart();
  I2CSendByte(0xA1);
  I2CWaitAck();
  while(len)
  {
    *double_str = I2CReceiveByte();
    len--;
    double_str++;
    if(len == 0)
    {
      I2CSendNotAck();
    }
    else
    {
      I2CSendAck();
    }      
  }
  I2CStop();
}




void eep_write(uint8_t adr,uint8_t data)
{
  I2CStart();

  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  I2CSendByte(data);
  I2CWaitAck();
  I2CStop();
  HAL_Delay(10);
}

uint8_t eep_read(uint8_t adr)
{
  uint8_t temp = 0;
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  I2CStart();
  I2CSendByte(0xA1);
  I2CWaitAck();
  temp = I2CReceiveByte();
  I2CSendNotAck();
  I2CStop();
  return temp;
}
