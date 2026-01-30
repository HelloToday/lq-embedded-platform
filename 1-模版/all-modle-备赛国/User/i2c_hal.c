/**
 * CT117E-M4 / GPIO - I2C
*/

#include "i2c_hal.h"

#define DELAY_TIME	20

//
void SDA_Input_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//
void SDA_Output_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//
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

//
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

//
uint8_t SDA_Input(void)
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET){
		return 1;
	}else{
		return 0;
	}
}

//
static void delay1(volatile unsigned int n)
{
    volatile uint32_t i;
    for ( i = 0; i < n; ++i);
}

//
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

//
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

//
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
    SCL_Output(0);
    SDA_Output_Mode();
    delay1(DELAY_TIME);
    return SUCCESS;
}

//
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

//
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

//
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

//
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


void eep_write_u8(uint8_t adr, uint8_t data)
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

uint8_t eep_read_u8(uint8_t adr)
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

void eep_write_arr(uint8_t adr, uint8_t* w_arr, uint8_t w_len)
{
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  while(w_len)
  {
    I2CSendByte(*w_arr);
    I2CWaitAck();
    w_len--;
    w_arr++;
  }
  I2CStop();
  HAL_Delay(10);   
}

void eep_read_arr(uint8_t adr, uint8_t* r_arr, uint8_t r_len)
{
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  I2CStart();
  I2CSendByte(0xA1);
  I2CWaitAck();
  while(r_len)
  {
    *r_arr = I2CReceiveByte();   
    if(r_len == 1)
    {
      r_len--;
      I2CSendNotAck();      
    }
    else
    {
      r_len--;
      I2CSendAck();
      r_arr++;
    }
  }
  I2CStop();
}


void eep_write_double(uint8_t adr, double* w_double)
{
  uint8_t w_len = 8;
  uint8_t* double_arr = (uint8_t*)w_double;
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  while(w_len)
  {
    I2CSendByte(*double_arr);
    I2CWaitAck();
    w_len--;
    double_arr++;
  }
  I2CStop();
  HAL_Delay(10);   
}

void eep_read_double(uint8_t adr, double* r_double)
{
  uint8_t r_len = 8;
  uint8_t * double_str = (uint8_t*)r_double;
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  I2CStart();
  I2CSendByte(0xA1);
  I2CWaitAck();
  while(r_len)
  {
    *double_str = I2CReceiveByte();   
    if(r_len == 1)
    {
      r_len--;
      I2CSendNotAck();      
    }
    else
    {
      r_len--;
      I2CSendAck();
      double_str++;
    }
  }
  I2CStop();
}

void mcp_write(uint8_t data)
{
  I2CStart();
  I2CSendByte(0x5E);
  I2CWaitAck();
  I2CSendByte(data);
  I2CWaitAck();
  I2CStop();
  HAL_Delay(10);  
}
uint8_t mcp_read(void)
{
  I2CStart();
  I2CSendByte(0x5F);
  I2CWaitAck();
  uint8_t temp = I2CReceiveByte(); 
  I2CSendNotAck();
  I2CStop();
  return temp;
} 


void eep_write_u16(uint8_t adr, uint16_t* data_u16, uint8_t w_len)
{
  uint8_t* w_arr = (uint8_t*)data_u16;
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  while(w_len)
  {
    I2CSendByte(*w_arr);
    I2CWaitAck();
    w_len--;
    w_arr++;
  }
  I2CStop();
  HAL_Delay(10);   
}

void eep_read_u16(uint8_t adr, uint16_t* data_u16, uint8_t r_len)
{
  uint8_t* r_arr = (uint8_t*)data_u16;
  I2CStart();
  I2CSendByte(0xA0);
  I2CWaitAck();
  I2CSendByte(adr);
  I2CWaitAck();
  
  I2CStart();
  I2CSendByte(0xA1);
  I2CWaitAck();
  while(r_len)
  {
    *r_arr = I2CReceiveByte();   
    if(r_len == 1)
    {
      r_len--;
      I2CSendNotAck();      
    }
    else
    {
      r_len--;
      I2CSendAck();
      r_arr++;
    }
  }
  I2CStop();
}


