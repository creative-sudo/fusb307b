/*******************************************************************************
 * @file     platform.h
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
/* platform.h
 *
 * THIS FILE DEFINES EVERY PLATFORM-DEPENDENT ELEMENT THAT THE CORE REQUIRES.
 *
 * INSTRUCTIONS FOR THIS FILE:
 * 1. Modify this file with a definition of Generic Type Definitions
 * (FSC_S8, FSC_U32, etc) Either by include or putting directly in this file.
 * 2. Include this as a header file for your platform.c and implement the
 * function headers as defined below.
 *
 * It is the driver-writer's responsibility to implement each function
 * stub and to allocate/initialize/reserve sufficient system resources.
 *
 */
#ifndef _FSC_PLATFORM_H_
#define _FSC_PLATFORM_H_

/* PLATFORM_ARM
 *
 * This platform is for the ARM M0.
 */
#ifdef PLATFORM_ARM
#include "FSCTypes.h"

#define kMSTimeFactor   1000  /* ARM platform timer set for 1us */
#endif // PLATFORM_ARM

/* FSC_PLATFORM_LINUX
 *
 * This platform is for the Linux kernel driver.
 */
#ifdef FSC_PLATFORM_LINUX
#include "../Platform_Linux/FSCTypes.h"

#define kMSTimeFactor   1     /* Linux platform converts sched_clock to ms */
#endif // FSC_PLATFORM_LINUX

#ifdef PLATFORM_NONE
#include "../Platform_None/FSCTypes.h"
#define kMSTimeFactor   1  /* 1ms timer */
#endif /* PLATFORM_NONE */

typedef enum {
  VBUS_LVL_0V,
  VBUS_LVL_5V,
  VBUS_LVL_9V,
  VBUS_LVL_12V,
  VBUS_LVL_15V,
  VBUS_LVL_20V,
  VBUS_LVL_COUNT,
  VBUS_LVL_ALL = 99
} VBUS_LVL;

void platform_printf(FSC_U8 port, const char *str, FSC_S32 value);

void platform_setHVSwitch(FSC_BOOL enable);
FSC_BOOL platform_getHVSwitch(void);

void platform_setPPSVoltage(FSC_U8 port, FSC_U32 mv);
FSC_U16 platform_getPPSVoltage(FSC_U8 port);
void platform_setPPSCurrent(FSC_U8 port, FSC_U32 ma);
FSC_U16 platform_getPPSCurrent(FSC_U8 port);

/*******************************************************************************
 * Function:        platform_get_device_irq_state
 * Input:           Port ID - 0 if one port system
 * Return:          Boolean.  TRUE = Interrupt Active
 * Description:     Get the state of the INT_N pin.  INT_N is active low.  This
 *                  function handles that by returning TRUE if the pin is
 *                  pulled low indicating an active interrupt signal.
 ******************************************************************************/
FSC_BOOL platform_get_device_irq_state(FSC_U8 port);

/*******************************************************************************
 * Function:        platform_i2c_write
 * Input:           SlaveAddress - Slave device bus address
 *                  RegisterAddress - Internal register address
 *                  DataLength - Length of data to transmit
 *                  Data - Buffer of char data to transmit
 * Return:          Error state
 * Description:     Write a char buffer to the I2C peripheral.
 ******************************************************************************/
FSC_BOOL platform_i2c_write(FSC_U8 SlaveAddress,
                            FSC_U8 RegisterAddress,
                            FSC_U8 DataLength,
                            FSC_U8* Data);

/*******************************************************************************
 * Function:        platform_i2c_read
 * Input:           SlaveAddress - Slave device bus address
 *                  RegisterAddress - Internal register address
 *                  DataLength - Length of data to attempt to read
 *                  Data - Buffer for received char data
 * Return:          Error state.
 * Description:     Read char data from the I2C peripheral.
 ******************************************************************************/
FSC_BOOL platform_i2c_read( FSC_U8 SlaveAddress,
                            FSC_U8 RegisterAddress,
                            FSC_U8 DataLength,
                            FSC_U8* Data);

/*****************************************************************************
* Function:        platform_enable_timer
* Input:           enable - TRUE to enable platform timer, FALSE to disable
* Return:          None
* Description:     Enables or disables platform timer
******************************************************************************/
void platform_enable_timer(FSC_BOOL enable);

/******************************************************************************
 * Function:        platform_delay
 * Input:           delayCount - Number of microseconds to wait
 * Return:          None
 * Description:     Perform a blocking software delay in intervals of 1us
 ******************************************************************************/
void platform_delay(FSC_U32 microseconds);

/******************************************************************************
 * Function:        platform_current_time
 * Input:           None
 * Return:          Current system time value in microseconds
 * Description:     Provide a running system clock for timer implementations
 ******************************************************************************/
FSC_U32 platform_current_time(void);

/******************************************************************************
 * Function:        platform_timestamp
 * Input:           None
 * Return:          Encoded timestamp: 0xSSSSMMMM
 * Description:     Provide a timestamp encoded into 32 bits.
 *                  MSB's are seconds, LSB's are 10ths of milliseconds.
 ******************************************************************************/
FSC_U32 platform_timestamp(void);


#ifdef FSC_HAVE_DP
/******************************************************************************
 * Function:        platform_dp_enable_pins
 * Input:           enable - If false put dp pins to safe state and config is
 *                           don't care. When true configure the pins with valid
 *                           config.
 *                  config - 32-bit port partner config. Same as type in
 *                  DisplayPortConfig_t in display_port_types.h.
 * Return:          TRUE - pin config succeeded, FALSE - pin config failed
 * Description:     enable/disable display port pins. If enable is true, check
 *                  the configuration bits[1:0] and the pin assignment
 *                  bits[15:8] to decide the appropriate configuration.
 ******************************************************************************/
FSC_BOOL platform_dp_enable_pins(FSC_BOOL enable, FSC_U32 config);

/******************************************************************************
 * Function:        platform_dp_status_update
 * Input:           status - 32-bit status value. Same as DisplayPortStatus_t
 *                  in display_port_types.h
 * Return:          None
 * Description:     Called when new status is available from port partner
 ******************************************************************************/
void platform_dp_status_update(FSC_U32 status);
#endif

#endif  // _FSC_PLATFORM_H_

