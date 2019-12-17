/*******************************************************************************
 * @file     core.c
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
#include "core.h"
#include "typec.h"
#include "dpm.h"

void core_initialize(struct Port *port)
{
  InitializeVars(port, 1, 0xA0);
  InitializePort(port);
  platform_printf(port->port_id_, "Port Initialized.\n", -1);
}

void core_state_machine(struct Port *port)
{
  FSC_U8 data = TRANSMIT_HARDRESET;

  if (port->tc_enabled_ == TRUE) {
    /* Hard reset timeout shortcut.
     * Helps prevent driver interrupt latency issues
     */
    if (port->waiting_on_hr_ && TimerExpired(&port->policy_state_timer_)) {
      /* Don't disable the timer here as we expect the states might be waiting for
       * expiration. */
      platform_i2c_write(port->i2c_addr_, regTRANSMIT, 1, &data);
    }

    /* Read status registers for ALL chip features */
    ReadStatusRegisters(port);

    /* Check and handle a chip reset */
    if (port->registers_.FaultStat.ALL_REGS_RESET) {
      core_initialize(port);
      return;
    }

    /* TypeC/PD state machines */
    StateMachineTypeC(port);
  }
}

void core_enable_typec(struct Port *port, FSC_BOOL enable)
{
  port->tc_enabled_ = enable;
}

FSC_U32 core_get_next_timeout(struct Port *port)
{
  FSC_U32 time = 0;
  FSC_U32 nexttime = 0xFFFFFFFF;

  time = TimerRemaining(&port->pps_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  time = TimerRemaining(&port->tc_state_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  time = TimerRemaining(&port->policy_state_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

#ifdef FSC_HAVE_VDM
  time = TimerRemaining(&port->vdm_timer_);
  if (time > 0 && time < nexttime) nexttime = time;
#endif /* FSC_HAVE_VDM */

  time = TimerRemaining(&port->no_response_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  time = TimerRemaining(&port->swap_source_start_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  time = TimerRemaining(&port->pd_debounce_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  time = TimerRemaining(&port->cc_debounce_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  time = TimerRemaining(&port->policy_sinktx_timer_);
  if (time > 0 && time < nexttime) nexttime = time;

  if (nexttime == 0xFFFFFFFF) nexttime = 0;

  return nexttime;
}

void core_set_advertised_current(struct Port *port, USBTypeCCurrent src_cur)
{
    UpdateSourceCurrent(port, src_cur);
}

/*
 * Call this function to get the lower 8-bits of the core revision number.
 */
FSC_U8 core_get_rev_lower(void)
{
    return FSC_TYPEC_CORE_FW_REV_LOWER;
}

/*
 * Call this function to get the middle 8-bits of the core revision number.
 */
FSC_U8 core_get_rev_middle(void)
{
    return FSC_TYPEC_CORE_FW_REV_MIDDLE;
}

/*
 * Call this function to get the upper 8-bits of the core revision number.
 */
FSC_U8 core_get_rev_upper(void)
{
    return FSC_TYPEC_CORE_FW_REV_UPPER;
}

void core_set_state_unattached(struct Port *port)
{
    SetStateUnattached(port);
}

