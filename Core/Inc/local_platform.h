/*******************************************************************************
 * @file     local_platform.h
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
 * platform.h
 *
 * Implements hardware interfaces for the port manager.
 * Currently supports the ARM STM32F072B microcontroller for the
 * FUSB307B evaluation board.
 */

#ifndef FSCPM_PLATFORM_H_
#define FSCPM_PLATFORM_H_

#include "FSCTypes.h"

/* PlatformInitialize
 *
 * Arguments:   None
 * Return:      None
 * Description: Initialize peripherals used by the firmware.
 *              (I2C, timers, clocks, GPIO, etc.)
 */
void PlatformInitialize(void);

/* EnableTickTimer
 *
 * Arguments:   enable: True - on
 * Return:      None
 * Description: Enable or disable the timer (2) used for tick counters.
 */
void EnableTickTimer(FSC_BOOL enable);

/* SetTimeInterrupt
 *
 * Arguments:   microseconds
 * Return:      None
 * Description: Set timer interrupt to trigger after microseconds.
 */
void SetTimeInterrupt(FSC_U32 microseconds);

/* ClearTimeInterrupt
 *
 * Arguments:   None
 * Return:      None
 * Description: Disable the timer interrupt.
 */
void ClearTimeInterrupt();

/* GetCurrentTime
 *
 * Arguments:   None
 * Return:      Current time
 * Description: Return Timer2's current CNT value. 32-bits, 1us resolution.
 */
FSC_U32 platform_GetCurrentTime(void);

/* Delay
 *
 * Arguments:   Delay count measured in microseconds
 * Return:      None
 * Description: A software delay timer that blocks until the time runs out.
 */
void platform_Delay(FSC_U32 microseconds);

/* EnableTSTimer
 *
 * Arguments:   enable: True - on
 * Return:      None
 * Description: Enable or disable the timers (1,3) used for time stamps.
 */
void EnableTSTimer(FSC_BOOL enable);

/* GetTimeStamp
 *
 * Arguments:   None
 * Return:      Current time stamp time as 0xSSSSMMMM
 *              SSSS: Seconds, MMMM: milliseconds
 * Description: Return a time stamp based on Timer1 (0.1ms) and Timer3 (sec)
 */
FSC_U32 platform_GetTimeStamp(void);

/* platform_printf
 *
 * Arguments:   Port number (1-3)
 *              Character message (null or \n terminated)
 *              Optional numeric value (displayed as 4-byte hex value).
 * Return:      None
 * Description: A simplified printf function.
 *              Prints a timestamp, port number, message, and optional value
 */
void platform_printf(FSC_U8 port, const char *msg, FSC_S32 value);

/* Write one character at a time to the UART */
void WriteUART(FSC_S8 c);

/* Flush the UART buffers using DMA memory transfers */
void ProcessUART(void);

void platform_SetDebugPin(FSC_BOOL enable);

#endif /* FSCPM_PLATFORM_H_ */

