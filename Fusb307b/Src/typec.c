/*******************************************************************************
 * @file     typec.c
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
 * typec.c
 *
 * Implements the Type-C state machine functions
 */

#include "typec.h"

#include "log.h"
#include "platform.h"
#include "protocol.h"
#include "policy.h"
#include "observer.h"
#include "vendor_info.h"

/* This is a value for checking illegal cable issues. */
#define MAX_CABLE_LOOP  20

/* Entry point to the Type-C state machine */
void StateMachineTypeC(struct Port *port)
{
  if (port->tc_enabled_ == TRUE) {
    /* Read/clear masked ints to avoid confusion in the state machines */
    if (~port->registers_.AlertMskL.byte & port->registers_.AlertL.byte)
      ClearInterrupt(port, regALERTL,
        (~port->registers_.AlertMskL.byte & port->registers_.AlertL.byte));

    if (~port->registers_.AlertMskH.byte & port->registers_.AlertH.byte)
      ClearInterrupt(port, regALERTH,
        (~port->registers_.AlertMskH.byte & port->registers_.AlertH.byte));

    /* Handle I2C_ERR, if needed */
    if (port->registers_.FaultStat.I2C_ERR) {
      ClearInterrupt(port, regFAULTSTAT, MSK_I2C_ERROR);
      ClearInterrupt(port, regALERTH, MSK_I_FAULT);
    }

    port->idle_ = FALSE;

    /* PD State Machines */
    if (port->pd_active_) {
      USBPDProtocol(port);
      USBPDPolicyEngine(port);

      /* Shortcut to transmit, if needed */
      if (port->pd_tx_status_ == txSend) {
        USBPDProtocol(port);
      }
    }

    /* Clear the interrupt here but leave the bit set for use in SM functions */
    if (port->registers_.AlertL.I_CCSTAT) {
      ClearInterrupt(port, regALERTL, MSK_I_CCSTAT);
      port->registers_.AlertL.I_CCSTAT = 1;
    }

    /* Type-C State Machine */
    switch (port->tc_state_) {
    case Disabled:
      StateMachineDisabled(port);
      break;
    case ErrorRecovery:
      StateMachineErrorRecovery(port);
      break;
    case Unattached:
      StateMachineUnattached(port);
      break;
#ifdef FSC_HAVE_SNK
    case AttachWaitSink:
      StateMachineAttachWaitSink(port);
      break;
    case AttachedSink:
      StateMachineAttachedSink(port);
      break;
    case DebugAccessorySink:
      StateMachineDebugAccessorySink(port);
      break;
#ifdef FSC_HAVE_DRP
    case TryWaitSink:
      StateMachineTryWaitSink(port);
      break;
#if (defined(FSC_HAVE_SNK) || defined(FSC_HAVE_ACC))
    case TrySink:
      StateMachineTrySink(port);
      break;
#endif /* FSC_HAVE_SNK && FSC_HAVE_ACC */
#endif /* FSC_HAVE_DRP */
#endif /* FSC_HAVE_SNK */
#ifdef FSC_HAVE_SRC
    case AttachWaitSource:
      StateMachineAttachWaitSource(port);
      break;
    case AttachedSource:
      StateMachineAttachedSource(port);
      break;
    case UnattachedWaitSource:
      StateMachineUnattachedWaitSource(port);
      break;
    case UnorientedDebugAccessorySource:
      StateMachineUnorientedDebugAccessorySource(port);
      break;
    case OrientedDebugAccessorySource:
      StateMachineOrientedDebugAccessorySource(port);
      break;
#ifdef FSC_HAVE_DRP
    case TryWaitSource:
      StateMachineTryWaitSource(port);
      break;
    case TrySource:
      StateMachineTrySource(port);
      break;
#endif /* FSC_HAVE_DRP */
#endif /* FSC_HAVE_SRC */
#ifdef FSC_HAVE_ACC
    case AudioAccessory:
      StateMachineAudioAccessory(port);
      break;
    case AttachWaitAccessory:
      StateMachineAttachWaitAccessory(port);
      break;
    case PoweredAccessory:
      StateMachinePoweredAccessory(port);
      break;
    case UnsupportedAccessory:
      StateMachineUnsupportedAccessory(port);
      break;
#endif /* FSC_HAVE_ACC */
    case IllegalCable:
      StateMachineIllegalCable(port);
      break;
    default:
      /* We shouldn't get here, so go to the unattached state just in case */
      SetStateUnattached(port);
      break;
    }
  } /* TC Enabled */
}

void StateMachineDisabled(struct Port *port)
{
#ifdef FSC_HAVE_SNK
  if (port->is_dead_battery_) {
    DetectCCPin(port);

    SetStateAttachedSink(port);
  }
  else
#endif /* FSC_HAVE_SNK */
  {
    SetStateUnattached(port);
  }
}

void StateMachineErrorRecovery(struct Port *port)
{
  if (TimerExpired(&port->tc_state_timer_)) {
    TimerDisable(&port->tc_state_timer_);
    SetStateUnattached(port);
  }
  else {
    port->idle_ = TRUE;
  }
}

void StateMachineUnattached(struct Port *port)
{
  /*
   * If we got an interrupt for a CCStat change and if LOOK4CON is clear,
   * then the device is done looking for a connection.
   */
  if (port->registers_.AlertL.I_CCSTAT == 1 ) {
    if (port->registers_.CCStat.LOOK4CON == 1) {
      /* Still looking for a connection... */
      port->idle_ = TRUE;
      return;
    }

    /* Set Source or Sink before using in pin detection */
    if (port->port_type_ == USBTypeC_Source ||
        (port->port_type_ == USBTypeC_DRP &&
         port->registers_.CCStat.CON_RES == 0) ||
        (port->port_type_ == USBTypeC_Sink &&
         port->registers_.RoleCtrl.DRP == 1 &&
         port->registers_.CCStat.CON_RES == 0)) {
      port->source_or_sink_ = Source;
    }
    else {
      port->source_or_sink_ = Sink;
    }

    /* Get both CC line terminations */
    DetectCCPin(port);
    UpdateVConnTermination(port);

#ifdef FSC_HAVE_SRC
    if (port->port_type_ == USBTypeC_Source ||
        (port->port_type_ == USBTypeC_DRP &&
         port->registers_.CCStat.CON_RES == 0)) {
      /* Operating as a Src or DRP-Src */
      SetStateAttachWaitSource(port);
    }
    else
#endif /* FSC_HAVE_SRC */
#ifdef FSC_HAVE_SNK
    if (port->port_type_ == USBTypeC_Sink ||
        (port->port_type_ == USBTypeC_DRP &&
         port->registers_.CCStat.CON_RES == 1)) {
#ifdef FSC_HAVE_ACC
      if (port->acc_support_ &&
          port->port_type_ == USBTypeC_Sink &&
          port->registers_.RoleCtrl.DRP == 1 &&
          port->registers_.CCStat.CON_RES == 0) {
        if ((port->cc_term_ == CCTypeRdUSB &&
             port->vconn_term_ == CCTypeRa) ||
            (port->cc_term_ == CCTypeRdUSB &&
             port->vconn_term_ == CCTypeRdUSB) ||
            (port->cc_term_ == CCTypeRa &&
             port->vconn_term_ == CCTypeRa)) {
          SetStateAttachWaitAccessory(port);
        }
      }
      else
#endif /* FSC_HAVE_ACC */
      /* Operating as a Snk or DRP-Snk */
      /* Else-if to prevent attaching to Rd-Open while in Snk+Acc mode */
      if ((port->cc_term_ == CCTypeRdUSB ||
           port->cc_term_ == CCTypeRd1p5 ||
           port->cc_term_ == CCTypeRd3p0) &&
          port->vconn_term_ == CCTypeOpen) {
        SetStateAttachWaitSink(port);
      }
    }
#endif /* FSC_HAVE_SNK */

    if (port->tc_state_ == Unattached)
    {
      /* Reset our CC detection variables for next time through */
      SetStateUnattached(port);
    }
  }
  else {
    port->idle_ = TRUE;
  }
}

#ifdef FSC_HAVE_SNK
void StateMachineAttachWaitSink(struct Port *port)
{
  /* If VConn is not Open, keep checking for the right CC/VConn termination */
  if (port->vconn_term_ != CCTypeOpen) {
    port->cc_pin_ = NONE;
    DetectCCPin(port);
  }

  UpdateVConnTermination(port);
  DebounceCC(port);

  if (port->registers_.AlertL.I_VBUS_ALRM_HI == 1) {
    /* Interrupt caused by VBUS crossing VSafe5V */
    ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);
  }

  if (port->cc_term_pd_debounce_ == CCTypeOpen &&
      port->vconn_term_ == CCTypeOpen) {
      /* Open detected? */
      SetStateUnattached(port);
  } else if (IsVbusVSafe5V(port)) {
    if (port->cc_term_cc_debounce_ >= CCTypeRdUSB &&
        port->cc_term_cc_debounce_ < CCTypeUndefined &&
        port->vconn_term_ >= CCTypeRdUSB &&
        port->vconn_term_ < CCTypeUndefined) {
        /* If both pins are Rp, it's a debug accessory */
        SetStateDebugAccessorySink(port);
    } else if (port->cc_term_cc_debounce_ > CCTypeOpen &&
              port->cc_term_cc_debounce_ < CCTypeUndefined &&
              port->vconn_term_ == CCTypeOpen) {
#ifdef FSC_HAVE_DRP
      if (port->port_type_ == USBTypeC_DRP &&
          port->src_preferred_ == TRUE) {
        port->registers_.AlertMskL.M_VBUS_ALRM_HI = 0;
        WriteRegister(port, regALERTMSKL);
        SetStateTrySource(port);
      } else
#endif /* FSC_HAVE_DRP */
      {
        SetStateAttachedSink(port);
      }
    }
  }
  else {
      port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void StateMachineAttachWaitSource(struct Port *port)
{
  DebounceCC(port);
  UpdateVConnTermination(port);

  if (port->registers_.AlertH.I_VBUS_ALRM_LO == 1) {
    /* VBUS has reached VSafe0V. */
    ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
  }

  /* There is a chance that RaRa is detected and one termination changes
   * to Rd.  Swap if necessary.
   */
  if (port->cc_term_previous_ == CCTypeRa &&
      port->vconn_term_ == CCTypeRdUSB) {
      port->cc_pin_ = port->cc_pin_ == CC1 ? CC2 : CC1;
      DebounceCC(port);
  }

  /* Check other line before attaching */
  if (port->cc_term_cc_debounce_ != CCTypeUndefined) {
    /* Update our cc/vconn terminations */
    port->cc_term_ = port->cc_term_cc_debounce_;
  }

#ifdef FSC_HAVE_ACC
  if (port->acc_support_ == TRUE &&
      port->cc_term_cc_debounce_ == CCTypeRa &&
      port->vconn_term_ == CCTypeRa) {
    /* If both pins are Ra, it's an audio accessory */
    SetStateAudioAccessory(port);
  }
  else
#endif /* FSC_HAVE_ACC */
  if (port->cc_term_previous_ == CCTypeOpen ||
      port->cc_term_previous_ == CCTypeUndefined) {
    SetStateUnattached(port);
  }
  else if (IsVbusVSafe0V(port)) {
    /* If both pins are Rd, it's a debug accessory */
    if (port->cc_term_cc_debounce_ == CCTypeRdUSB &&
        port->vconn_term_ == CCTypeRdUSB) {
      SetStateUnorientedDebugAccessorySource(port);
    }
    else if (port->cc_term_cc_debounce_ == CCTypeRdUSB &&
            (port->vconn_term_ == CCTypeOpen ||
             port->vconn_term_ == CCTypeRa)) {
      /* If CC1 is Rd and CC2 is not... */
#ifdef FSC_HAVE_DRP
      if (port->snk_preferred_) {
        SetStateTrySink(port);
      } else
#endif /* FSC_HAVE_DRP */
      {
        SetStateAttachedSource(port);
      }
    }
    else {
      port->idle_ = TRUE;
    }
  }
  else
  {
    port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_ACC
void StateMachineAttachWaitAccessory(struct Port *port)
{
  DebounceCC(port);
  UpdateVConnTermination(port);

  if (port->cc_term_cc_debounce_ != CCTypeUndefined) {
    /* Update our cc termination */
    port->cc_term_ = port->cc_term_cc_debounce_;
  }

  /* There is a chance that RaRa is detected and one termination changes
   * to Rd.  Swap if necessary.
   */
  if (port->cc_term_previous_ == CCTypeRa &&
      port->vconn_term_ == CCTypeRdUSB) {
      port->cc_pin_ = port->cc_pin_ == CC1 ? CC2 : CC1;
      DebounceCC(port);
  }

  if (port->cc_term_cc_debounce_ == CCTypeRa &&
      port->vconn_term_ == CCTypeRa) {
    /* If both pins are Ra, it's an audio accessory */
    SetStateAudioAccessory(port);
  }
  else if (port->cc_term_cc_debounce_ >= CCTypeRdUSB &&
           port->cc_term_cc_debounce_ < CCTypeUndefined &&
           port->vconn_term_ == CCTypeRa) {
    SetStatePoweredAccessory(port);
  }
  else if (port->cc_term_cc_debounce_ >= CCTypeRdUSB &&
           port->cc_term_cc_debounce_ < CCTypeUndefined &&
           port->vconn_term_ >= CCTypeRdUSB &&
           port->vconn_term_ < CCTypeUndefined) {
#ifdef FSC_HAVE_SNK
    SetStateDebugAccessorySink(port);
#endif /* FSC_HAVE_SNK */
  }
  else if (port->cc_term_previous_ == CCTypeOpen ||
           port->vconn_term_ == CCTypeOpen) {
    SetStateUnattached(port);
  }
  else {
      port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_ACC */

#ifdef FSC_HAVE_SNK
void StateMachineAttachedSink(struct Port *port)
{
#ifdef FSC_HAVE_FRSWAP
  /* Handle Fast Role swap operations */
  if (port->registers_.AlertH.I_VD_ALERT &&
      port->registers_.AlertVD.I_SWAP_RX) {
    ClearInterrupt(port, regALERTH, MSK_I_VD_ALERT);
    ClearInterrupt(port, regALERT_VD, MSK_I_SWAP_RX);
    port->is_fr_swap_ = TRUE;

    port->registers_.PwrCtrl.AUTO_DISCH = 0;
    WriteRegister(port, regPWRCTRL);

    /* Re-enable SOP on RxDetect after the FRS PD_RESET */
    ResetProtocolLayer(port);

    set_policy_state(port, PE_FRS_SNK_SRC_Send_Swap);

    port->idle_ = FALSE;
    /* Sink -> Source path switch happens automatically */
  }
#endif /* FSC_HAVE_FRSWAP */

  /* A VBus disconnect should generate an interrupt to wake us up */
  if (port->registers_.AlertH.I_VBUS_SNK_DISC || IsVbusVSafe0V(port)) {
    ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
    ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);

    if (port->is_pr_swap_ == FALSE &&
#ifdef FSC_HAVE_FRSWAP
        port->is_fr_swap_ == FALSE &&
#endif /* FSC_HAVE_FRSWAP */
        port->is_hard_reset_ == FALSE) {
        /* Start the disconnect process */
      SetStateUnattached(port);
      return;
    }
  }

  if (!port->is_pr_swap_) {
    DebounceCC(port);
  }

  /* If using PD, sink can monitor CC as well as VBUS to allow detach during a
   * hard rest */
  if (port->pd_active_ == TRUE && !port->is_pr_swap_ &&
      port->cc_term_pd_debounce_ == CCTypeOpen) {
      SetStateUnattached(port);
    return;
  }

  if (TimerExpired(&port->tc_state_timer_)) {
    TimerDisable(&port->tc_state_timer_);
    port->unattach_loop_counter_ = 0;
  }

  if (port->registers_.AlertL.I_PORT_PWR) {
    ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
  }

  /* Update sink current from CC level */
  UpdateSinkCurrent(port);
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void StateMachineAttachedSource(struct Port *port)
{
#ifdef FSC_HAVE_FRSWAP
  /* Handle Fast Role swap operations */
  if (port->registers_.AlertH.I_VD_ALERT &&
      port->registers_.AlertVD.I_SWAP_TX) {
    ClearInterrupt(port, regALERTH, MSK_I_VD_ALERT);
    ClearInterrupt(port, regALERT_VD, MSK_I_SWAP_TX);
    port->is_fr_swap_ = TRUE;

    port->registers_.PwrCtrl.AUTO_DISCH = 0;
    WriteRegister(port, regPWRCTRL);

    /* Re-enable SOP on RxDetect after the FRS PD_RESET */
    ResetProtocolLayer(port);

    /* Source -> Sink path switch happens automatically */
  }
#endif /* FSC_HAVE_FRSWAP */

  DebounceCC(port);

  switch(port->tc_substate_) {
  case 0:
    /* Look for CC detach */
    if (port->cc_term_previous_ == CCTypeOpen &&
#ifdef FSC_HAVE_FRSWAP
        port->is_fr_swap_ == FALSE &&
#endif /* FSC_HAVE_FRSWAP */
        port->is_pr_swap_ == FALSE) {
#ifdef FSC_HAVE_DRP
      if (port->port_type_ == USBTypeC_DRP && port->src_preferred_ == TRUE) {
        SetStateTryWaitSink(port);
      }
      else
#endif /* FSC_HAVE_DRP */
      {
        /* Start the disconnect process */
        port->tc_substate_++;
        port->idle_ = FALSE;
        port->pd_active_ = FALSE;

        /* Disable vconn to prepare for Rd discharge below */
        port->registers_.PwrCtrl.EN_VCONN = 0;
        WriteRegister(port, regPWRCTRL);

        if (port->port_type_ == USBTypeC_DRP) {
          /* Set Rd terminations for transition to unattached.snk */
          port->registers_.RoleCtrl.DRP = 0;
          port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
          port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
        }
        else {
          /* Set one Rd termination (vconn discharge) for unattachedwait.src */
          if (port->cc_pin_ == CC1) {
            port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
          }
          else {
            port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
          }
        }
        WriteRegister(port, regROLECTRL);

        SendCommand(port, DisableSourceVbus);

        /* Set up the VBus alarm to wait for vSafe0V */
        SetVBusAlarm(port, FSC_VSAFE0V, FSC_VBUS_LVL_HIGHEST);

        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
        WriteRegister(port, regALERTMSKH);

        port->registers_.PwrCtrl.AUTO_DISCH = 0;
        port->registers_.PwrCtrl.FORCE_DISCH = 1;
        port->registers_.PwrCtrl.DIS_VALARM = 0;
        WriteRegister(port, regPWRCTRL);

        /* Double check we aren't already at vSafe0V */
        if (IsVbusVSafe0V(port)) {
          SetStateUnattached(port);
        }
      }
    }
    else if (TimerExpired(&port->tc_state_timer_)) {
      TimerDisable(&port->tc_state_timer_);
      if (port->unattach_loop_counter_ != 0)
      {
        port->unattach_loop_counter_ = 0;

        if (port->policy_state_ == PE_SRC_Ready || port->pd_enabled_ == FALSE)
        {
          port->idle_ = TRUE;

          port->registers_.AlertMskL.M_CCSTAT = 1;
          WriteRegister(port, regALERTMSKL);
        }
      }
    }
    break;
  case 1:
    if (port->registers_.AlertH.I_VBUS_ALRM_LO) {
      ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
      SetStateUnattached(port);
    }
    else {
      port->idle_ = TRUE;
    }
    break;
  default:
    SetStateErrorRecovery(port);
    break;
  }

}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_DRP
void StateMachineTryWaitSink(struct Port *port)
{
  DebounceCC(port);

  if (port->cc_term_pd_debounce_ == CCTypeOpen) {
    /* Open detected? */
    SetStateUnattached(port);
  }
  else if (port->registers_.PwrStat.VBUS_VAL) {
    /* Otherwise, VBUS and Rp? */
    if (port->cc_term_cc_debounce_ > CCTypeOpen &&
        port->cc_term_cc_debounce_ < CCTypeUndefined) {
      SetStateAttachedSink(port);
    }
  }
  else {
    port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void StateMachineTrySource(struct Port *port)
{
  DebounceCC(port);

  if (port->cc_term_pd_debounce_ > CCTypeRa &&
      port->cc_term_pd_debounce_ < CCTypeUndefined &&
      (port->vconn_term_ == CCTypeOpen ||
       port->vconn_term_ == CCTypeRa)) {
    /* If the CC1 pin is Rd for at least tPDDebounce... */
    SetStateAttachedSource(port);
  }
  else if (TimerExpired(&port->tc_state_timer_)) {
    TimerDisable(&port->tc_state_timer_);
    /* If we haven't detected Rd on exactly one of the pins and we have */
    /* waited for tDRPTry, move onto the TryWait.Snk state */
    SetStateTryWaitSink(port);
  }
  else {
    port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SRC
void StateMachineUnorientedDebugAccessorySource(struct Port *port)
{
  DebounceCC(port);
  UpdateVConnTermination(port);

  port->idle_ = TRUE;

  if (port->cc_term_ == CCTypeOpen ||
      port->vconn_term_ == CCTypeOpen) {
      SetStateUnattached(port);
  }
  else if (port->cc_term_pd_debounce_ >= CCTypeRa &&
           port->cc_term_pd_debounce_ < CCTypeUndefined &&
           port->vconn_term_ >= CCTypeRa &&
           port->vconn_term_ < CCTypeUndefined &&
           port->pd_active_ == FALSE) {
    /* CCStat register issue causes only one CC termination to be monitored
     * when in "manual" debug accessory mode.
     * Can be tested using only one orientation.
     */
    if (port->cc_term_pd_debounce_ > port->vconn_term_) {
      port->cc_pin_ = CC1;
      PDEnable(port, TRUE);
      SetStateOrientedDebugAccessorySource(port);
    }
    else if (port->vconn_term_ > port->cc_term_pd_debounce_) {
      port->cc_pin_ = CC2;
      PDEnable(port, TRUE);
      SetStateOrientedDebugAccessorySource(port);
    }
  }
}

void StateMachineOrientedDebugAccessorySource(struct Port *port)
{
  DebounceCC(port);
  UpdateVConnTermination(port);

  /* CCStat register issue causes only one CC termination to be monitored
   * when in "manual" debug accessory mode so need to monitor both for detach.
   */
  if (port->cc_term_ == CCTypeOpen ||
      port->vconn_term_ == CCTypeOpen) {
    SetStateUnattached(port);
  }
  else {
    port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
void StateMachineDebugAccessorySink(struct Port *port)
{
  UpdateVConnTermination(port);
  DebounceCC(port);

  port->idle_ = TRUE;
  if (!port->registers_.PwrStat.VBUS_VAL) {
      SetStateUnattached(port);
  }
  else if (port->cc_term_pd_debounce_ >= CCTypeRdUSB &&
           port->cc_term_pd_debounce_ < CCTypeUndefined &&
           port->vconn_term_ >= CCTypeRdUSB &&
           port->vconn_term_ < CCTypeUndefined &&
           port->pd_active_ == FALSE) {
    /* CCStat register issue causes only one CC termination to be monitored
     * when in "manual" debug accessory mode.
     * Can be tested using only one orientation.
     */
    if (port->cc_term_pd_debounce_ > port->vconn_term_) {
      port->cc_pin_ = CC1;
      PDEnable(port, FALSE);
    }
    else if (port->vconn_term_ > port->cc_term_pd_debounce_) {
      port->cc_pin_ = CC2;
      PDEnable(port, FALSE);
    }
  }
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_ACC
void StateMachineAudioAccessory(struct Port *port)
{
  DebounceCC(port);
  UpdateVConnTermination(port);

  if (port->cc_term_cc_debounce_ == CCTypeOpen) {
    SetStateUnattached(port);
  }
  else if (port->vconn_term_ == CCTypeOpen) {
    /* For compliance, we may need to change which pin we are
     * monitoring for open.
     */
    port->cc_pin_ = port->cc_pin_ == CC1 ? CC2 : CC1;
  }
  else {
    port->idle_ = TRUE;
  }
}

/* TODO: Update VCONN-Powered Accessory logic */
void StateMachinePoweredAccessory(struct Port *port)
{
  DebounceCC(port);

  if (port->cc_term_previous_ == CCTypeOpen) {
    /* Transition to Unattached.Snk when monitored CC pin is Open */
    SetStateUnattached(port);
  }
#ifdef FSC_HAVE_VDM
  else if (port->mode_entered_ == TRUE) {
    TimerDisable(&port->tc_state_timer_);
    port->unattach_loop_counter_ = 0;
  }
#endif /* FSC_HAVE_VDM */
  else if (TimerExpired(&port->tc_state_timer_)) {
    TimerDisable(&port->tc_state_timer_);

    /* Time out and not in alternate mode */
    if (port->policy_has_contract_) {
      SetStateUnsupportedAccessory(port);
    }
    else {
      SetStateTrySink(port);
    }
  }
}

void StateMachineUnsupportedAccessory(struct Port *port)
{
  DebounceCC(port);

  if (port->cc_term_pd_debounce_ == CCTypeOpen) {
    /* Transition to Unattached.Snk when monitored CC pin is Open */
    SetStateUnattached(port);
  }
  else {
      port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_ACC */

#if (defined(FSC_HAVE_DRP) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC)))
void StateMachineTrySink(struct Port *port)
{
  switch (port->tc_substate_) {
  case 0:
    if (TimerExpired(&port->tc_state_timer_)) {
      TimerStart(&port->tc_state_timer_, ktDRPTryWait);
      ResetDebounceVariables(port);
      port->tc_substate_++;
    }
    else {
      port->idle_ = TRUE;
    }
    break;
  case 1:
    DebounceCC(port);

    if (port->registers_.PwrStat.VBUS_VAL &&
        port->cc_term_pd_debounce_ >= CCTypeRdUSB &&
        port->cc_term_pd_debounce_ < CCTypeUndefined) {
        /* If the CC pin is Rd for at least tPDDebounce... */
      SetStateAttachedSink(port);
    }
#ifdef FSC_HAVE_ACC
    else if(TimerExpired(&port->tc_state_timer_) &&
            port->port_type_ == USBTypeC_Sink) {
      TimerDisable(&port->tc_state_timer_);
      SetStateUnsupportedAccessory(port);
    }
#endif /* FSC_HAVE_ACC */
#ifdef FSC_HAVE_DRP
    else if(port->port_type_ == USBTypeC_DRP &&
            port->cc_term_pd_debounce_ == CCTypeOpen) {
      SetStateTryWaitSource(port);
    }
#endif /* FSC_HAVE_DRP */
    else {
      port->idle_ = TRUE;
    }
    break;
  default:
    port->tc_substate_ = 0;
    break;
  }
}
#endif /* FSC_HAVE_DRP ||  FSC_HAVE_SNK && FSC_HAVE_ACC*/

#ifdef FSC_HAVE_DRP
void StateMachineTryWaitSource(struct Port *port)
{
  DebounceCC(port);

  if (IsVbusVSafe0V(port) &&
      port->cc_term_pd_debounce_ >= CCTypeRdUSB &&
      port->cc_term_pd_debounce_ < CCTypeUndefined &&
      (port->vconn_term_ == CCTypeRa ||
       port->vconn_term_ == CCTypeOpen)) {
    SetStateAttachedSource(port);
  }
  else if (TimerExpired(&port->tc_state_timer_)) {
    TimerDisable(&port->tc_state_timer_);
    /* Go to Unattached.SNK if Rd is not detected on exactly 1 CC pin */
    if (!(port->cc_term_previous_ >= CCTypeRdUSB &&
          port->cc_term_previous_ < CCTypeUndefined) &&
         (port->vconn_term_ == CCTypeRa ||
          port->vconn_term_ == CCTypeOpen)) {
      SetStateUnattached(port);
    }
  }
  else {
    port->idle_ = TRUE;
  }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SRC
void StateMachineUnattachedWaitSource(struct Port *port)
{
  UpdateVConnTermination(port);

  /* Discharging VConn - wait for open or timeout */
  if (port->vconn_term_ == CCTypeOpen ||
      TimerExpired(&port->tc_state_timer_)) {
    TimerDisable(&port->tc_state_timer_);
    SetStateUnattached(port);
  }
}
#endif /* FSC_HAVE_SRC */

void StateMachineIllegalCable(struct Port *port)
{
  DebounceCC(port);

  /* Look for detach */
  if (port->cc_term_previous_ == CCTypeOpen) {
      SetStateUnattached(port);
  }
  else {
      port->idle_ = TRUE;
  }
}

/*
 *  State Machine Configuration
 */
void SetStateDisabled(struct Port *port)
{
  port->idle_ = FALSE;
  port->tc_state_ = Disabled;
  TimerDisable(&port->tc_state_timer_);
  ClearState(port);

  /* Present Open/Open */
  port->registers_.RoleCtrl.CC1_TERM = CCRoleOpen;
  port->registers_.RoleCtrl.CC2_TERM = CCRoleOpen;
  WriteRegister(port, regROLECTRL);

  port->registers_.PwrCtrl.AUTO_DISCH = 0;
  WriteRegister(port, regPWRCTRL);
}

void SetStateErrorRecovery(struct Port *port)
{
  platform_printf(port->port_id_, "SS ER\n", -1);
  port->idle_ = FALSE;
  port->tc_state_ = ErrorRecovery;
  TimerStart(&port->tc_state_timer_, ktErrorRecovery);
  ClearState(port);

  port->registers_.PwrCtrl.AUTO_DISCH = 0;
  WriteRegister(port, regPWRCTRL);

  /* Present Open/Open for tErrorRecovery */
  port->registers_.RoleCtrl.DRP = 0;
  port->registers_.RoleCtrl.CC1_TERM = CCRoleOpen;
  port->registers_.RoleCtrl.CC2_TERM = CCRoleOpen;
  WriteRegister(port, regROLECTRL);
}

/* SetStateUnattached configures the Toggle state machine in the device to */
/* handle all of the unattached states. */
/* This allows for the MCU to be placed in a low power mode until */
/* the device wakes it up upon detecting something */
void SetStateUnattached(struct Port *port)
{
  FSC_BOOL wasAWSnk = (port->tc_state_ == AttachWaitSink) ? TRUE : FALSE;

  platform_printf(port->port_id_, "SS UN\n", -1);

  port->idle_ = TRUE;
  port->tc_state_ = Unattached;

  ClearState(port);

  /* Clear all alert interrupts */
  ClearInterrupt(port, regALERTL, MSK_I_ALARM_LO_ALL);
  ClearInterrupt(port, regALERTH, MSK_I_ALARM_HI_ALL);
  ClearInterrupt(port, regFAULTSTAT, MSK_FAULTSTAT_ALL);

  /* Disable monitoring except for CCStat */
  port->registers_.AlertMskL.byte = MSK_I_CCSTAT;
  port->registers_.AlertMskH.byte = 0;
  WriteRegisters(port, regALERTMSKL, 2);

  /* Disable monitoring and reconfigure to look for the next connection */
  if (port->port_type_ == USBTypeC_DRP) {
    /* Config as DRP */
    port->registers_.RoleCtrl.DRP = 1;

    if (wasAWSnk) {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleRp;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleRp;
    }
    else {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
    }
  }
  else if (port->port_type_ == USBTypeC_Source) {
    /* Config as a source with Rp-Rp */
    port->registers_.RoleCtrl.DRP = 0;
    port->registers_.RoleCtrl.CC1_TERM = CCRoleRp;
    port->registers_.RoleCtrl.CC2_TERM = CCRoleRp;
  }
  else {
    /* Config as a sink with Rd-Rd - toggle for acc if supported */
    port->registers_.RoleCtrl.DRP = port->acc_support_ ? 1 : 0;
    port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
    port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
  }
  port->registers_.StdOutCfg.ORIENT = 0;
  WriteRegister(port, regSTD_OUT_CFG);

  UpdateSourceCurrent(port, port->src_current_);

  /* Driver will wait until device detects a new connection */
  SendCommand(port, Look4Con);
  TimerDisable(&port->tc_state_timer_);
  notify_observers(EVENT_TYPEC_DETACH, port->port_id_, 0);
}

#ifdef FSC_HAVE_SNK
void SetStateAttachWaitSink(struct Port *port)
{
  platform_printf(port->port_id_, "SS AWSnk\n", -1);

  port->idle_ = FALSE;

  port->tc_state_ = AttachWaitSink;
  SetStateSink(port);

   /* Check for cable looping */
  if (port->unattach_loop_counter_ > MAX_CABLE_LOOP) {
    SetStateIllegalCable(port);
    return;
  }
  else {
    port->unattach_loop_counter_++;
  }

  /* Set sink terminations.  This may be a good idea or may just be part
   * of the workaround for AWSink issue.
   */
  port->registers_.RoleCtrl.DRP = 0;
  port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
  port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
  WriteRegister(port, regROLECTRL);

  UpdateVConnTermination(port);

  TimerDisable(&port->tc_state_timer_);

  SetVBusAlarm(port, 0, FSC_VSAFE5V_L);
  port->registers_.AlertMskL.M_PORT_PWR = 0;
  port->registers_.AlertMskL.M_VBUS_ALRM_HI = 1;
  WriteRegister(port, regALERTMSKL);

  notify_observers(EVENT_TYPEC_ATTACH, port->port_id_, 0);
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void SetStateAttachWaitSource(struct Port *port)
{
  platform_printf(port->port_id_, "SS AWSrc\n", -1);

  port->idle_ = FALSE;

  /* Check for cable looping */
  if (port->unattach_loop_counter_ > MAX_CABLE_LOOP) {
    SetStateIllegalCable(port);
    return;
  }
  else {
    port->unattach_loop_counter_++;
  }

  /* Enable the CCStat interrupt */
  port->registers_.AlertMskL.M_CCSTAT = 1;
  WriteRegister(port, regALERTMSKL);

  port->tc_state_ = AttachWaitSource;
  SetStateSource(port);

  /* Set VBUS Alarm */
  SetVBusAlarm(port, FSC_VSAFE0V, FSC_VBUS_LVL_HIGHEST);
  port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
  WriteRegister(port, regALERTMSKH);

  /* TODO Add VBus Discharge before attaching */

  UpdateVConnTermination(port);

  TimerDisable(&port->tc_state_timer_);
  notify_observers(EVENT_TYPEC_ATTACH, port->port_id_, 0);
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_ACC
void SetStateAttachWaitAccessory(struct Port *port)
{
  platform_printf(port->port_id_, "SS AWAcc\n", -1);

  port->idle_ = FALSE;

  /* Swap toggle state machine current if looping */
  if (port->unattach_loop_counter_ > MAX_CABLE_LOOP) {
    SetStateIllegalCable(port);
    return;
  }
  else {
    UpdateSourceCurrent(port, port->src_current_);
    port->unattach_loop_counter_++;
  }

  port->tc_state_ = AttachWaitAccessory;

  UpdateVConnTermination(port);
  SetStateSource(port);

  TimerDisable(&port->tc_state_timer_);
  notify_observers(EVENT_TYPEC_ATTACH, port->port_id_, 0);
}
#endif /* FSC_HAVE_ACC */

#ifdef FSC_HAVE_SRC
void SetStateAttachedSource(struct Port *port)
{
  platform_printf(port->port_id_, "SS ASrc\n", -1);

  port->idle_ = TRUE;

  port->tc_state_ = AttachedSource;

  UpdateVConnTermination(port);
  UpdateOrientation(port);

  /* Turn off Bleed - VBus should be below vSafe0V */
  port->registers_.PwrCtrl.EN_BLEED_DISCH = 0;
  WriteRegister(port, regPWRCTRL);

  SetStateSource(port);

  /* Enable only the 5V output */
  SendCommand(port, SourceVbusDefaultV);

  /* Turn on VConn */
  if (Type_C_Sources_VCONN) {
    port->is_vconn_source_ = TRUE;
    SetVConn(port, TRUE);
  }

  PDEnable(port, TRUE);

  /* Delay slightly before setting auto discharge to allow CC status */
  /* to settle after supplying VBus. */
  platform_delay(1000);

  port->registers_.PwrCtrl.AUTO_DISCH = 1;
  WriteRegister(port, regPWRCTRL);

  notify_observers((port->cc_pin_ == CC1
                   ? EVENT_CC1_ORIENT : EVENT_CC2_ORIENT),
                   port->port_id_, 0);

  /* Start dangling illegal cable timeout */
  TimerStart(&port->tc_state_timer_, ktIllegalCable);
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SRC
void SetStateUnattachedWaitSource(struct Port *port)
{
  platform_printf(port->port_id_, "SS UnWSrc\n", -1);

  port->idle_ = FALSE;
  port->pd_active_ = FALSE;

  /* Disable VBus and VBus detection */
  SendCommand(port, DisableSourceVbus);

  PDDisable(port);

  /* Set the state machine variable to unattached */
  port->tc_state_ = UnattachedWaitSource;

  /* Turn off VConn */
  port->registers_.PwrCtrl.EN_VCONN = 0;
  WriteRegister(port, regPWRCTRL);

  /* Apply Ra to discharge VConn */
  /* TODO - Use Rd for now so we can monitor CCStat */
  if (port->cc_pin_ == CC1) {
    port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
  }
  else {
    port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
  }
  WriteRegister(port, regROLECTRL);

  TimerStart(&port->tc_state_timer_, ktTryTimeout);

#ifdef FSC_LOGGING
  LogTCState(port);
#endif /* FSC_LOGGING */
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
void SetStateAttachedSink(struct Port *port)
{
  platform_printf(port->port_id_, "SS ASnk\n", -1);

  port->idle_ = TRUE;

  port->tc_state_ = AttachedSink;
  SetStateSink(port);

  /* Disable VBUS Alarm */
  port->registers_.PwrCtrl.DIS_VALARM = 1;
  WriteRegister(port, regPWRCTRL);

  /* Set up the Sink Disconnect threshold/interrupt */
  SetVBusSnkDisc(port, FSC_VSAFE5V_DISC);
  SetVBusStopDisc(port, FSC_VSAFE0V_DISCH);

  port->registers_.AlertMskL.M_PORT_PWR = 1;
  port->registers_.AlertMskH.M_VBUS_SNK_DISC = 1;
  WriteRegisters(port, regALERTMSKL, 2);

  ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
  ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);

  UpdateVConnTermination(port);
  UpdateOrientation(port);

  /* Call once at the start to set the initial termination value */
  DebounceCC(port);

  PDEnable(port, FALSE);

  port->registers_.PwrCtrl.AUTO_DISCH = 1;
  port->registers_.PwrCtrl.EN_BLEED_DISCH = 1;
  WriteRegister(port, regPWRCTRL);

  SendCommand(port, SinkVbus);

  port->is_vconn_source_ = FALSE;

  notify_observers((port->cc_pin_ == CC1
                   ? EVENT_CC1_ORIENT : EVENT_CC2_ORIENT),
                   port->port_id_, 0);

  /* Start dangling illegal cable timeout */
  TimerStart(&port->tc_state_timer_, ktIllegalCable);
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_DRP
void RoleSwapToAttachedSink(struct Port *port)
{
  platform_printf(port->port_id_, "SS RStoASnk\n", -1);

  port->tc_state_ = AttachedSink;
  port->source_or_sink_ = Sink;

  port->registers_.AlertMskL.M_CCSTAT = 0;
  WriteRegister(port, regALERTMSKL);

  UpdateOrientation(port);
  SetStateSink(port);

  /* Set up the Sink Disconnect threshold/interrupt */
  SetVBusSnkDisc(port, FSC_VSAFE5V_DISC);

  port->registers_.AlertMskL.M_PORT_PWR = 1;
  port->registers_.AlertMskH.M_VBUS_SNK_DISC = 1;
  WriteRegisters(port, regALERTMSKL, 2);

  ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
  ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);

  SendCommand(port, SinkVbus);

  /* Set the current advertisement variable to none until */
  /* we determine what the current is */
  port->snk_current_ = utccOpen;

  TimerDisable(&port->tc_state_timer_);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void RoleSwapToAttachedSource(struct Port *port)
{
  platform_printf(port->port_id_, "SS RStoASrc\n", -1);

  port->tc_state_ = AttachedSource;
  port->source_or_sink_ = Source;
  ResetDebounceVariables(port);

  UpdateOrientation(port);

  SetStateSource(port);

  port->registers_.AlertMskL.M_CCSTAT = 1;
  WriteRegister(port, regALERTMSKL);

  ClearInterrupt(port, regALERTL, MSK_I_CCSTAT);

  SendCommand(port, SourceVbusDefaultV);

  /* Set the Sink current to none (not used in Src) */
  port->snk_current_ = utccOpen;

  TimerDisable(&port->tc_state_timer_);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void SetStateTryWaitSink(struct Port *port)
{
  platform_printf(port->port_id_, "SS TryWSnk\n", -1);

  port->idle_ = FALSE;


  PDDisable(port);
  port->tc_state_ = TryWaitSink;
  SetStateSink(port);

  port->registers_.AlertMskL.byte = 0;
  port->registers_.AlertMskL.M_CCSTAT = 1;
  port->registers_.AlertMskL.M_PORT_PWR = 1;
  port->registers_.AlertMskH.byte = 0;
  WriteRegisters(port, regALERTMSKL, 2);

  UpdateOrientation(port);

  TimerDisable(&port->tc_state_timer_);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void SetStateTrySource(struct Port *port)
{
  platform_printf(port->port_id_, "SS TrySrc\n", -1);

  port->idle_ = FALSE;

  port->tc_state_ = TrySource;

  SetStateSource(port);

  UpdateOrientation(port);

  TimerStart(&port->tc_state_timer_, ktDRPTry);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SNK
void SetStateTrySink(struct Port *port)
{
  platform_printf(port->port_id_, "SS TrySnk\n", -1);

  port->idle_ = FALSE;
  port->tc_state_ = TrySink;

  SetVConn(port, FALSE);

  SetStateSink(port);

  UpdateOrientation(port);

  /* Set the state timer to tDRPTry to timeout if Rd isn't detected */
  TimerStart(&port->tc_state_timer_, ktDRPTry);
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_DRP
void SetStateTryWaitSource(struct Port *port)
{
  platform_printf(port->port_id_, "SS TryWSrc\n", -1);

  port->idle_ = FALSE;

  port->tc_state_ = TryWaitSource;
  SendCommand(port, DisableSourceVbus);
  SetStateSource(port);

  UpdateOrientation(port);

  TimerStart(&port->tc_state_timer_, ktDRPTry);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SRC
void SetStateUnorientedDebugAccessorySource(struct Port *port)
{
  platform_printf(port->port_id_, "SS UDASrc\n", -1);

  port->idle_ = TRUE;

  /* TODO - Probably just follow the new TOGGLE_SM register status */

  port->registers_.AlertMskL.M_CCSTAT = 1;
  WriteRegister(port, regALERTMSKL);
  port->unattach_loop_counter_ = 0;
  port->tc_state_ = UnorientedDebugAccessorySource;
  SetStateSource(port);
  SendCommand(port, SourceVbusDefaultV);

  notify_observers(EVENT_CC_NO_ORIENT, port->port_id_, 0);
  /* Handle unoriented accessory later - for now assume CC1 */
  UpdateVConnTermination(port);

  TimerDisable(&port->tc_state_timer_);
}

void SetStateOrientedDebugAccessorySource(struct Port *port)
{
  platform_printf(port->port_id_, "SS ODASrc\n", -1);

  port->idle_ = TRUE;

  /* TODO - Probably just follow the new TOGGLE_SM register status */

  port->registers_.AlertMskL.M_CCSTAT = 1;
  WriteRegister(port, regALERTMSKL);
  port->unattach_loop_counter_ = 0;
  port->tc_state_ = OrientedDebugAccessorySource;
  SetStateSource(port);

  //port->registers_.PwrCtrl.AUTO_DISCH = 1;
  //WriteRegister(port, regPWRCTRL);

  TimerDisable(&port->tc_state_timer_);
  notify_observers(port->cc_pin_ == CC1 ? EVENT_CC1_ORIENT : EVENT_CC2_ORIENT,
          port->port_id_, 0);
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
void SetStateDebugAccessorySink(struct Port *port)
{
  port->idle_ = TRUE;
  port->tc_state_ = DebugAccessorySink;
  SetStateSink(port);

  /* Disable the VBus Value notification */
  port->registers_.PwrCtrl.DIS_VALARM = 1;
  WriteRegister(port, regPWRCTRL);

  /* Set up the Sink Disconnect threshold/interrupt */
  SetVBusSnkDisc(port, FSC_VSAFE5V_DISC);
  SetVBusStopDisc(port, FSC_VSAFE0V_DISCH);

  port->registers_.AlertMskL.M_PORT_PWR = 1;
  port->registers_.AlertMskH.M_VBUS_SNK_DISC = 1;
  WriteRegisters(port, regALERTMSKL, 2);

  ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
  ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);

  /* TODO - Add events Power role, PD contract etc*/
  notify_observers(EVENT_CC1_ORIENT | EVENT_DEBUG_ACCESSORY,
                   port->port_id_, 0);

  TimerDisable(&port->tc_state_timer_);
}
#endif /* FSC_HAVE_SNK */

#if defined(FSC_HAVE_ACC) && (defined(FSC_HAVE_SNK) || defined(FSC_HAVE_SRC))
void SetStateAudioAccessory(struct Port *port)
{
  port->idle_ = FALSE;
  port->unattach_loop_counter_ = 0;

  port->tc_state_ = AudioAccessory;
  SetStateSource(port);

  /* TODO - Handle un-oriented Accessory - For now assume CC1 */
  //UpdateOrientation(port);

  /* TODO - Add events CC Orientation, Power role and etc... */
  notify_observers(EVENT_CC1_ORIENT | EVENT_AUDIO_ACCESSORY,
                   port->port_id_, 0);

//  port->registers_.PwrCtrl.AUTO_DISCH = 1;
//  WriteRegister(port, regPWRCTRL);

  TimerDisable(&port->tc_state_timer_);
}

void SetStatePoweredAccessory(struct Port *port)
{
  port->idle_ = TRUE;
  port->tc_state_ = PoweredAccessory;
  UpdateVConnTermination(port);
  UpdateOrientation(port);

  /* Note that sourcing VBus for powered accessories is not supported in
   * Type-C 1.2, but is done here because not all accessories work without it.
   */
  if (Sources_VBus_For_Powered_Accessory) {
    SendCommand(port, SourceVbusDefaultV);
  }

  /* Turn on VConn */
  if (Type_C_Sources_VCONN) {
    port->is_vconn_source_ = TRUE;
    SetVConn(port, TRUE);
  }

  SetStateSource(port);

  PDEnable(port, TRUE);

  port->registers_.PwrCtrl.AUTO_DISCH = 1;
  WriteRegister(port, regPWRCTRL);

  /* Note/TODO: State timer should be enabled here to transition to
   * UnsupportedAccessory if an alternate mode is not entered in time.
   * This isn't working 100% so the timer has been disabled for now.
   */
  //TimerDisable(&port->tc_state_timer_);
  TimerStart(&port->tc_state_timer_, ktAMETimeout);

  /* TODO - Add more relevant notification? */
  notify_observers((port->cc_pin_ == CC1 ?
                    EVENT_CC1_ORIENT : EVENT_CC2_ORIENT),
                    port->port_id_, 0);
}

void SetStateUnsupportedAccessory(struct Port *port)
{
  port->idle_ = TRUE;

  /* Mask for COMP */
  port->registers_.AlertMskL.M_CCSTAT = 1;
  WriteRegister(port, regALERTMSKL);

  /* Vbus was enabled in PoweredAccessory - disable it here. */
  if (Sources_VBus_For_Powered_Accessory) {
    SendCommand(port, DisableSourceVbus);
  }

  /* Turn off VConn */
  if (Type_C_Sources_VCONN) {
    port->is_vconn_source_ = FALSE;
    SetVConn(port, FALSE);
  }

  port->tc_state_ = UnsupportedAccessory;

  /* Must advertise default current */
  UpdateSourceCurrent(port, utccDefault);

  SetStateSource(port);

  UpdateOrientation(port);

  notify_observers(EVENT_UNSUPPORTED_ACCESSORY, port->port_id_, 0);

  port->registers_.PwrCtrl.AUTO_DISCH = 1;
  WriteRegister(port, regPWRCTRL);

  TimerDisable(&port->tc_state_timer_);
}
#endif /* FSC_HAVE_ACC && (FSC_HAVE_SNK || FSC_HAVE_SRC) */

void SetStateIllegalCable(struct Port *port)
{
  platform_printf(port->port_id_, "SS IllCab\n", -1);

  port->tc_state_ = IllegalCable;
  port->unattach_loop_counter_ = 0;

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
  SetStateSource(port);

  UpdateOrientation(port);

  /* Advertise Default current */
  UpdateSourceCurrent(port, utccDefault);

  /* Turn on VBus Bleed Discharge resistor to provide path to ground */
  port->registers_.PwrCtrl.EN_BLEED_DISCH = 1;
  WriteRegister(port, regPWRCTRL);

  /* Wait for 10ms for the CC line level to stabilize */
  platform_delay(10000);

  /* Set AUTO_DISCH and wait for a detach */
  port->registers_.PwrCtrl.AUTO_DISCH = 1;
  WriteRegister(port, regPWRCTRL);
#endif /* FSC_HAVE_SRC ||  FSC_HAVE_SNK && FSC_HAVE_ACC */
  /* No contract could be negotiated. */
  notify_observers(EVENT_CC_NO_ORIENT | EVENT_ILLEGAL_CBL,
                   port->port_id_, 0);

  port->idle_ = TRUE;
}

