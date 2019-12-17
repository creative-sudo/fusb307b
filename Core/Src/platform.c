/*******************************************************************************
 * @file     platform.c
 * @author   USB PD Firmware Team
 *
 * Copyright 2018 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 ******************************************************************************/
/*
 * platform.c
 *
 * Implements HW interfaces on the embedded processor for the port manager.
 */

#include "FSCTypes.h"
#include "local_platform.h"

#ifdef FSC_HAVE_6295
#include "fan6295.h"
#endif /* FSC_HAVE_6295 */

/* HAL Includes */
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"

#ifdef FSC_HAVE_UART
#include "stm32f0xx_hal_usart.h"
#endif /* FSC_HAVE_UART */
#include "timer.h"

/* Pin selections: */
#define PIN_I2C_SCL         GPIO_PIN_10 /* PB_10 */
#define PIN_I2C_SDA         GPIO_PIN_11 /* PB_11 */
#define PIN_DBG_USART_RX    GPIO_PIN_3  /* PA_3  */
#define PIN_DBG_USART_TX    GPIO_PIN_2  /* PA_2  */
#define PIN_USB_HID_pl      GPIO_PIN_12 /* PA_12 */
#define PIN_USB_HID_DM      GPIO_PIN_11 /* PA_11 */

#define PIN_ALERT_1         GPIO_PIN_6  /* PA_8  */
#define PIN_ALERT_2         GPIO_PIN_9  /* PA_9  */
#define PIN_ALERT_3         GPIO_PIN_10 /* PA_10 */

#define PIN_HV_ENABLE       GPIO_PIN_12 /* PB_12 */

#define PIN_DEBUG           GPIO_PIN_1  /* PA_1  */

#define UART_BUFFER_SIZE    1024

/* File Variables */

#ifdef FSC_HAVE_UART
/* UART Transmit PingPong Buffers */
FSC_U8 *UARTXmitBuffer;
FSC_U16 UARTXmitIndex;
FSC_U8 UARTXmitBuffer_Ping[UART_BUFFER_SIZE];
FSC_U8 UARTXmitBuffer_Pong[UART_BUFFER_SIZE];
FSC_BOOL UARTXmitBuffer_IsPing;
#endif /* FSC_HAVE_UART */

extern volatile FSC_BOOL g_timer_int_active;

void SystemClockConfig(void);
void InitializePeripheralClocks(void);
void InitializeI2C(void);
void InitializeGPIO(void);
void InitializeTickTimer(void);
void InitializeTSTimer(void);

#ifdef FSC_HAVE_UART
void InitializeUART(void);
#endif /* FSC_HAVE_UART */

void PlatformInitialize(void)
{
#ifdef FSC_HAVE_UART
  UARTXmitBuffer = UARTXmitBuffer_Ping;
  UARTXmitIndex = 0;
  UARTXmitBuffer_IsPing = TRUE;
#endif /* FSC_HAVE_UART */

  //SystemClockConfig();

  //InitializePeripheralClocks();

  //HAL_Init();
  //HAL_InitTick(1);

  //InitializeI2C();
  //InitializeGPIO();
  InitializeTickTimer();
  InitializeTSTimer();
#ifdef FSC_HAVE_UART
  InitializeUART();
#endif /* FSC_HAVE_UART */
#ifdef FSC_HAVE_6295
  FAN6295_Initialize();
#endif /* FSC_HAVE_6295 */
}

FSC_BOOL platform_i2c_read(FSC_U8 slaveaddress, FSC_U8 regaddr,
                           FSC_U8 length, FSC_U8 *data)
{
  I2C_HandleTypeDef i2chandle = {};
  HAL_StatusTypeDef result = HAL_OK;

  /* To prevent storing a global, re-initialize the basic items here. */
  i2chandle.Instance = I2C3;
  i2chandle.State    = HAL_I2C_STATE_READY;

  result = HAL_I2C_Mem_Read(&i2chandle, slaveaddress,
                            regaddr, 1, data, length, 0x10);

  return ((result == HAL_OK) ? TRUE : FALSE);
}

FSC_BOOL platform_i2c_write(FSC_U8 slaveaddress, FSC_U8 regaddr,
                            FSC_U8 length, FSC_U8 *data)
{
  I2C_HandleTypeDef i2chandle = {};
  HAL_StatusTypeDef result = HAL_OK;

  /* To prevent storing a global, re-initialize the basic items here. */
  i2chandle.Instance = I2C3;
  i2chandle.State    = HAL_I2C_STATE_READY;

  result = HAL_I2C_Mem_Write(&i2chandle, slaveaddress,
                             regaddr, 1, data, length, 0x10);

  return ((result == HAL_OK) ? TRUE : FALSE);
}

FSC_BOOL platform_get_device_irq_state(FSC_U8 port)
{
  GPIO_PinState state = GPIO_PIN_RESET;

  switch(port) {
  case 1:
    state = HAL_GPIO_ReadPin(GPIOA, PIN_ALERT_1);
    break;
  case 2:
    state = HAL_GPIO_ReadPin(GPIOA, PIN_ALERT_2);
    break;
  case 3:
    state = HAL_GPIO_ReadPin(GPIOA, PIN_ALERT_3);
    break;
  default:
    break;
  }

  /* ALERT signals are active low, so this looks backwards! */
  return (state == GPIO_PIN_SET) ? FALSE : TRUE;
}

void platform_SetDebugPin(FSC_BOOL enable)
{
  HAL_GPIO_WritePin(GPIOA, PIN_DEBUG,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void platform_setHVSwitch(FSC_BOOL enable)
{
  HAL_GPIO_WritePin(GPIOB, PIN_HV_ENABLE,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

FSC_BOOL platform_getHVSwitch(void)
{
  GPIO_PinState state = HAL_GPIO_ReadPin(GPIOB, PIN_HV_ENABLE);
  return (state == GPIO_PIN_SET) ? TRUE : FALSE;
}

void platform_setPPSVoltage(FSC_U8 port, FSC_U32 mv)
{
  if (mv == 0) {
#ifdef FSC_HAVE_6295
    FAN6295_SetVoltage(250);
    FAN6295_SetEnable(FALSE);
#endif /* FSC_HAVE_6295 */
  }
  else {
#ifdef FSC_HAVE_6295
    FAN6295_SetVoltage(mv);
    FAN6295_SetEnable(TRUE);
#endif /* FSC_HAVE_6295 */
  }
}

FSC_U16 platform_getPPSVoltage(FSC_U8 port)
{
#ifdef FSC_HAVE_6295
  return FAN6295_GetVoltage();
#else
  return 0;
#endif /* FSC_HAVE_6295 */
}

void platform_setPPSCurrent(FSC_U8 port, FSC_U32 ma)
{
#ifdef FSC_HAVE_6295
    FAN6295_SetILimit(ma);
#endif /* FSC_HAVE_6295 */
}

FSC_U16 platform_getPPSCurrent(FSC_U8 port)
{
#ifdef FSC_HAVE_6295
  return FAN6295_GetCurrent();
#else
  return 0;
#endif /* FSC_HAVE_6295 */
}

void InitializeTickTimer(void)
{
  /* Disable */
  TIM2->CR1 = 0x00000000;

  /* SystemCoreClock is 48MHz. */
  /* Prescaler - 48 gives 1us resolution */
  TIM2->PSC = 48;

  /* Start the count at the end.  Certain registers (PSC,ARR,...) are */
  /* "shadowed" and only get updated on a rollover or other event. */
  TIM2->CNT = 0xFFFFFFFF;

  /* Period/Reload - a long period give a free-running time-stamp-mode timer */
  TIM2->ARR = 0xFFFFFFFF;

  /* Enable! */
  EnableTickTimer(TRUE);

  /* Enable and set TIM2 Interrupt */
  TIM2->SR = 0;
  HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void EnableTickTimer(FSC_BOOL enable)
{
  if (enable) {
    TIM2->CR1 |= TIM_CR1_CEN;
  }
  else {
    TIM2->CR1 &= ~TIM_CR1_CEN;
  }
}

FSC_U32 platform_current_time(void)
{
  return TIM2->CNT;
}

void platform_delay(FSC_U32 microseconds)
{
  FSC_U32 currentTime = TIM2->CNT;
  while((FSC_U32)(TIM2->CNT - currentTime) < microseconds);
}

void SetTimeInterrupt(FSC_U32 microseconds)
{
  /* Set the offset for the next interrupt request */
  TIM2->CCR1 = TIM2->CNT + microseconds;
  /* Clear the previous and enable the next CC1 Interrupt */
  TIM2->SR = 0;
  TIM2->DIER |= TIM_DIER_CC1IE;
  g_timer_int_active = TRUE;
}

void ClearTimeInterrupt()
{
  TIM2->DIER &= ~TIM_DIER_CC1IE;
}

void InitializeTSTimer(void)
{
  /* Use Timer1 to count at 0.1ms resolution up to one second. */
  /* Use Timer3, triggered by Timer16, to count seconds. */

  /* SystemCoreClock is 48MHz. */
  TIM1->PSC = 4800;               /* A prescaler of 4800 gives 0.1ms res */
  TIM1->ARR = 10000;              /* A period of 10000 gives 1sec intervals */
  TIM1->CR2 |= TIM_CR2_MMS_1;     /* MMS = 0b010 generates TRGO */

  /* Start the count at the end.  Certain registers (PSC,ARR,...) are */
  /* "shadowed" and only get updated on a rollover or other event. */
  TIM1->CNT = 0xFFFFFFFF;

  /* Set Timer3 up as a slave to Timer1.  Timer3 will count in seconds. */
  TIM3->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;
  TIM3->CNT = 0;

  /* Enable both. */
  TIM1->CR1 |= TIM_CR1_CEN;
  TIM3->CR1 |= TIM_CR1_CEN;
}

void EnableTSTimer(FSC_BOOL enable)
{
  if (enable) {
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM3->CR1 |= TIM_CR1_CEN;
  }
  else {
    TIM1->CR1 &= ~TIM_CR1_CEN;
    TIM3->CR1 &= ~TIM_CR1_CEN;
  }
}

void platform_enable_timer(FSC_BOOL enable)
{
  EnableTickTimer(enable);
  EnableTSTimer(enable);
}

FSC_U32 platform_timestamp(void)
{
  /* This packs seconds and tenths of milliseconds into one 32-bit value. */
  return ((FSC_U32)(TIM3->CNT) << 16) +
          (FSC_U32)(TIM1->CNT);
}

#ifdef FSC_HAVE_UART
void InitializeUART(void)
{
  /* Initialize pins/clocks */
  GPIO_InitTypeDef  GPIO_InitStruct = {};

  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_USART2;

  GPIO_InitStruct.Pin       = PIN_DBG_USART_RX;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin       = PIN_DBG_USART_TX;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Initialize USART2 peripheral */
  /* Set the Rx/Tx enable bits and the Rx interrupt enable bit */
  USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

  /* Set the baud rate generator value. */
  /* Other rates -> ? */
  /* 115200 -> 0x000001A0 */
  USART2->BRR = 0x000001A0;

  /* Enable DMA Transmit Mode */
  USART2->CR3 |= USART_CR3_DMAT;

  /* Enable! */
  USART2->CR1 += USART_CR1_UE;

  /* Set up DMA Ch 4 to transfer to the USART TDR register */
  DMA1_Channel4->CCR |= DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;
  DMA1_Channel4->CPAR = (FSC_U32)&(USART2->TDR);

  /* Enable and set DMA Ch 4 IRQ to clear completion flag */
  HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

  /* Enable and set UART Rec Char Match IRQ */
  HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}
#endif /* FSC_HAVE_UART */

#ifdef FSC_HAVE_UART
void HexToAscii(FSC_U32 value, FSC_U8 bytes, FSC_U8 *data)
{
  FSC_U32 i = 0, j = 0;

  /* The old hex->ascii by hand trick */
  for (i = 0, j = 28; (i < 8) && (i < bytes*2); ++i, j-=4) {
    if (((value >> j) & 0xF) < 0x0A)
      data[i] = 0x30 + ((value >> j) & 0xF);
    else
      data[i] = 0x37 + ((value >> j) & 0xF);
  }

  return;
}
#endif /* FSC_HAVE_UART */

void platform_printf(FSC_U8 port, const char *msg, FSC_S32 value)
{
#ifdef FSC_HAVE_UART
  char *str = (char *)msg;
  FSC_U8 data[8];
  FSC_U32 timeval = 0;
  FSC_U32 timestamp = platform_timestamp();
  FSC_U32 i = 0;

  /* Print time stamp seconds */
  timeval = (timestamp & 0xFFFF0000) >> 16;
  for (i = 1000; i != 0;) {
    FSC_U8 digit = timeval / i;
    WriteUART(digit + 0x30);
    timeval = timeval - (digit * i);
    i = i / 10;
  }

  WriteUART('.');

  /* Print time stamp tenths of milliseconds */
  timeval = timestamp & 0x0000FFFF;
  for (i = 1000; i != 0;) {
    FSC_U8 digit = timeval / i;
    WriteUART(digit + 0x30);
    timeval = timeval - (digit * i);
    i = i / 10;
  }

  WriteUART(' ');

  /* Port Number */
  WriteUART('P');

  switch(port) {
  case 1:
    WriteUART('1');
    break;
  case 2:
    WriteUART('2');
    break;
  case 3:
    WriteUART('3');
    break;
  default:
    WriteUART('X');
    break;
  }

  WriteUART(' ');

  /* String */
  while (*str != '\n' && *str != '\r' && *str != 0)
    WriteUART(*str++);

  /* Optional Value */
  if (value >= 0) {
    WriteUART(' ');

    HexToAscii(value, 4, data);

    for (i = 0; i < 8; ++i)
      WriteUART(data[i]);
  }

  WriteUART('\r');
  WriteUART('\n');

  return;
#endif /* FSC_HAVE_UART */
}

void WriteUART(FSC_S8 c)
{
#ifdef FSC_HAVE_UART
  if (UARTXmitBuffer == NULL) return;

  if (UARTXmitIndex < UART_BUFFER_SIZE) {
    UARTXmitBuffer[UARTXmitIndex++] = c;
  }
#endif /* FSC_HAVE_UART */
}

#ifdef FSC_HAVE_UART
void ProcessUART(void)
{
  /* Try again later if transfer already in progress */
  if (DMA1_Channel4->CCR & DMA_CCR_EN) {
    return;
  }

  if (UARTXmitIndex > 0) {
    /* Handle the Ping/Pong buffering */
    if (UARTXmitBuffer_IsPing) {
      /* Set the memory buffer address */
      DMA1_Channel4->CMAR = (FSC_U32)UARTXmitBuffer_Ping;

      UARTXmitBuffer = UARTXmitBuffer_Pong;
      UARTXmitBuffer_IsPing = FALSE;
    }
    else {
      /* Set the memory buffer address */
      DMA1_Channel4->CMAR = (FSC_U32)UARTXmitBuffer_Pong;

      UARTXmitBuffer = UARTXmitBuffer_Ping;
      UARTXmitBuffer_IsPing = TRUE;
    }

    /* Set the transfer size */
    DMA1_Channel4->CNDTR = UARTXmitIndex;

    UARTXmitIndex = 0;

    /* Enable the transfer */
    DMA1_Channel4->CCR |= DMA_CCR_EN;
  }
}
#endif /* FSC_HAVE_UART */


#ifdef FSC_HAVE_DP
FSC_BOOL platform_dp_enable_pins(FSC_BOOL enable, FSC_U32 config)
{
  return TRUE;
}

void platform_dp_status_update(FSC_U32 status)
{
}
#endif /* FSC_HAVE_DP */
