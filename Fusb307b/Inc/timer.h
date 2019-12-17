/*******************************************************************************
 * @file     timer.h
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
 * Defines a timer class for time tracking against a system clock
 */
#ifndef FSCPM_TIMER_H_
#define FSCPM_TIMER_H_

#include "platform.h"

/* Constant timer values. Defined in milliseconds * a factor to align
 * the value with the system timer resolution.
 */

#define ktAMETimeout            (900 * kMSTimeFactor)
#define ktCCDebounce            (120 * kMSTimeFactor)
#define ktPDDebounce            (15 * kMSTimeFactor)
#define ktDRPTry                (125 * kMSTimeFactor)
#define ktDRPTryWait            (600 * kMSTimeFactor)
#define ktErrorRecovery         (30 * kMSTimeFactor)
#define ktTryTimeout            (550 * kMSTimeFactor)

#define ktDeviceToggle          (3 * kMSTimeFactor)
#define ktTOG2                  (30 * kMSTimeFactor)
#define ktIllegalCable          (150 * kMSTimeFactor)
#define ktVBusPollShort         (10 * kMSTimeFactor)

#define ktNoResponse            (5000 * kMSTimeFactor)
#define ktSenderResponse        (25 * kMSTimeFactor)
#define ktTypeCSendSourceCap    (150 * kMSTimeFactor)
#define ktRejectRecovery        (250 * kMSTimeFactor)
#define ktTypeCSinkWaitCap      (350 * kMSTimeFactor)
#define ktSrcTransition         (30 * kMSTimeFactor)
#define ktHardResetComplete     (5 * kMSTimeFactor)
#define ktPSHardReset           (30 * kMSTimeFactor)
#define ktPSHardResetMax        (35 * kMSTimeFactor)
#define ktPSHardResetOverhead   (3 * kMSTimeFactor)
#define ktPSTransition          (500 * kMSTimeFactor)
#define ktPSSourceOff           (770 * kMSTimeFactor)
#define ktPSSourceOn            (410 * kMSTimeFactor)
#define ktVCONNSourceOn         (90 * kMSTimeFactor)
#define ktBISTContMode          (50 * kMSTimeFactor)
#define ktSwapSourceStart       (25 * kMSTimeFactor)
#define ktSrcRecover            (830 * kMSTimeFactor)
#define ktSrcRecoverMax         (1000 * kMSTimeFactor)
#define ktGoodCRCDelay          (1 * kMSTimeFactor)
#define ktSwitchDelay           (10 * kMSTimeFactor)
#define ktSafe0V                (650 * kMSTimeFactor)
#define ktSrcTurnOn             (275 * kMSTimeFactor)
#define ktSrcStartupVbus        (150 * kMSTimeFactor)
#define ktSrcTransitionSupply   (350 * kMSTimeFactor)
#define ktSnkTransDefVbus       (150 * kMSTimeFactor)  /* 1.5s for VBus */
#define ktSinkTx                (16 * kMSTimeFactor)
#define ktChunkReceiverRequest  (15 * kMSTimeFactor)
#define ktChunkReceiverResponse (15 * kMSTimeFactor)
#define ktChunkSenderRequest    (30 * kMSTimeFactor)
#define ktChunkSenderResponse   (30 * kMSTimeFactor)
#define ktChunkingNotSupported  (40 * kMSTimeFactor)


#define ktPPSTimeout            (14000 * kMSTimeFactor)
#define ktPPSRequest            (10000 * kMSTimeFactor)

#ifdef FSC_HAVE_VDM
#define ktDiscoverIdentity       (40 * kMSTimeFactor)
#define ktVDMSenderResponse      (27 * kMSTimeFactor)
#define ktVDMWaitModeEntry       (50 * kMSTimeFactor)
#define ktVDMWaitModeExit        (50 * kMSTimeFactor)
#endif /* FSC_HAVE_VDM */

/* Struct object to contain the timer related members */
struct TimerObj {
  FSC_U32 starttime_;         /* Time-stamp when timer started */
  FSC_U32 period_;            /* Timer period */
  FSC_U8 count_;
};

/* Start the timer using the argument in microseconds. */
/* time must be greater than 0. */
void TimerStart(struct TimerObj *obj, FSC_U32 time);

/* Restart the timer using the last used delay value. */
void TimerRestart(struct TimerObj *obj);

/* Set time and period to zero to indicate no current period. */
void TimerDisable(struct TimerObj *obj);
FSC_BOOL TimerDisabled(struct TimerObj *obj);

/* Returns true when the time passed to Start is up. */
FSC_BOOL TimerExpired(struct TimerObj *obj);

/* Returns the time remaining in microseconds, or zero if disabled/done. */
FSC_U32 TimerRemaining(struct TimerObj *obj);

#endif /* FSCPM_TIMER_H_ */

