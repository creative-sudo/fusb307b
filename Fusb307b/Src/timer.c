/*******************************************************************************
 * @file     timer.c
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
 * timer.c
 *
 * Implements a timer class for time tracking against a system clock
 */

#include "timer.h"
#include "platform.h"

void TimerStart(struct TimerObj *obj, FSC_U32 time) {
  /* Grab the current time stamp and store the wait period. */
  /* Time must be > 0 */
#ifdef FSC_HAVE_UART
  /* Warning that the timer is already in use. Used strictly to catch the
   * culprit trying to start a timer already in use.*/
  if (obj->count_ > 0)
  {
    platform_printf(-1, "Timer already in use???\n", time);
  }
#endif /* FSC_HAVE_UART */
  obj->starttime_ = platform_current_time();
  obj->period_ = time;
  obj->count_ += 1;
  if (obj->period_ == 0) obj->period_ = 1;
}

void TimerRestart(struct TimerObj *obj) {
  /* Grab the current time stamp for the next period. */
  obj->starttime_ = platform_current_time();;
}

void TimerDisable(struct TimerObj *obj) {
  /* Zero means disabled */
  obj->starttime_ = obj->period_ = 0;
  if (obj->count_ > 0)
  {
     obj->count_--;
  }
}

FSC_BOOL TimerDisabled(struct TimerObj *obj) {
  /* Zero means disabled */
  return (obj->period_ == 0) ? TRUE : FALSE;
}

FSC_BOOL TimerExpired(struct TimerObj *obj) {
  if (TimerDisabled(obj)) {
    /* Disabled */
    return FALSE;
  }
  else{
    /* Elapsed time >= period? */
    return ((FSC_U32)(platform_current_time() - obj->starttime_) >=
            obj->period_) ? TRUE : FALSE;
  }
}

FSC_U32 TimerRemaining(struct TimerObj *obj)
{
  FSC_U32 currenttime = platform_current_time();

  if (TimerDisabled(obj)) {
    return 0;
  }

  if (TimerExpired(obj))
  {
    if (obj->count_ > 0)
    {
      /* Timer has expired and in use so do not let
       * the FSM go to sleep. Decrement the counter */
      obj->count_--;
      return 1;
    }
    else
    {
      /* Counter reached zero by repeatedly calling TimerRemaining()
       * on timer that might not have been disabled once enabled.
       * Disable the timer to allow idling. Set breakpoint in TimerStart
       * logging to see which code enabled the timer and did not stop
       *  it after expiration. */
      TimerDisable(obj);
      return 0;
    }
  }

  /* Timer hasn't expired, return time left */
  return (FSC_U32)(obj->starttime_ + obj->period_ - currenttime);
}

