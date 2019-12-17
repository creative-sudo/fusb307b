/*******************************************************************************
 * @file     policy.c
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
 * policy.c
 *
 * Implements the Policy state machine functions
 */

#include "policy.h"

#include "platform.h"
#include "PDTypes.h"
#include "timer.h"
#include "typec.h"
#include "protocol.h"
#include "dpm.h"
#include "observer.h"
#include "vendor_info.h"

#ifdef FSC_HAVE_VDM
#include "vdm.h"
#ifdef FSC_HAVE_DP
#include "display_port.h"
#endif /* FSC_HAVE_DP */
#endif /* FSC_HAVE_VDM */

void USBPDPolicyEngine(struct Port *port)
{
  switch (port->policy_state_) {
    case PE_ErrorRecovery:
      PolicyErrorRecovery(port);
      break;
/* ###################### Source States  ##################### */
#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined (FSC_HAVE_ACC))
    case PE_SRC_Hard_Reset:
      PolicySourceSendHardReset(port);
      break;
    case PE_SRC_Send_Soft_Reset:
      PolicySourceSendSoftReset(port);
      break;
    case PE_SRC_Soft_Reset:
      PolicySourceSoftReset(port);
      break;
    case PE_SRC_Startup:
      PolicySourceStartup(port);
      break;
    case PE_SRC_Discovery:
      PolicySourceDiscovery(port);
      break;
    case PE_SRC_Send_Capabilities:
      PolicySourceSendCaps(port);
      break;
    case PE_SRC_Disabled:
      PolicySourceDisabled(port);
      break;
    case PE_SRC_Transition_To_Default:
      PolicySourceTransitionDefault(port);
      break;
    case PE_SRC_Negotiate_Capability:
      PolicySourceNegotiateCap(port);
      break;
    case PE_SRC_Capability_Response:
      PolicySourceCapabilityResponse(port);
      break;
    case PE_SRC_Transition_Supply:
      PolicySourceTransitionSupply(port);
      break;
    case PE_SRC_Ready:
      PolicySourceReady(port);
      break;
    case PE_SRC_Get_Sink_Cap:
      PolicySourceGetSinkCap(port);
      break;
    case PE_SRC_Ping:
      PolicySourceSendPing(port);
      break;
    case PE_DR_SRC_Give_Sink_Cap:
      PolicySourceGiveSinkCap(port);
      break;
    case PE_DR_SRC_Get_Source_Cap:
      PolicySourceGetSourceCap(port);
      break;
    case PE_DRS_DFP_UFP_Send_Swap:
      PolicySourceSendDRSwap(port);
      break;
    case PE_DRS_DFP_UFP_Evaluate_Swap:
      PolicySourceEvaluateDRSwap(port);
      break;
    case PE_PRS_SRC_SNK_Send_Swap:
      PolicySourceSendPRSwap(port);
      break;
    case PE_PRS_SRC_SNK_Evaluate_Swap:
      PolicySourceEvaluatePRSwap(port);
      break;
#ifdef FSC_HAVE_FRSWAP
    case PE_FRS_SRC_SNK_Evaluate_Swap:
      PolicySourceEvaluateFRSwap(port);
      break;
#endif /* FSC_HAVE_FRSWAP */
    case PE_SRC_Wait_New_Capabilities:
      PolicySourceWaitNewCapabilities(port);
      break;
#ifdef FSC_HAVE_EXTENDED
    case PE_SRC_Give_PPS_Status:
      PolicyGivePPSStatus(port);
      break;
#ifdef FSC_HAVE_SRC
    case PE_SRC_Give_Source_Cap_Ext:
      PolicySourceGiveSourceCapExtended(port);
      break;
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACC) */
#endif /* FSC_HAVE_EXTENDED */
    case PE_SRC_Chunk_Received:
      PolicySourceChunkReceived(port);
      break;
    case PE_SRC_Send_Not_Supported:
      PolicySendNotSupported(port);
      break;
    case PE_SRC_Not_Supported_Received:
      PolicyNotSupported(port);
      break;
#endif /* FSC_HAVE_SRC */
/* ###################### Sink States  ####################### */
#ifdef FSC_HAVE_SNK
    case PE_SNK_Startup:
      PolicySinkStartup(port);
      break;
    case PE_SNK_Hard_Reset:
      PolicySinkSendHardReset(port);
      break;
    case PE_SNK_Soft_Reset:
      PolicySinkSoftReset(port);
      break;
    case PE_SNK_Send_Soft_Reset:
      PolicySinkSendSoftReset(port);
      break;
    case PE_SNK_Transition_To_Default:
      PolicySinkTransitionDefault(port);
      break;
    case PE_SNK_Discovery:
      PolicySinkDiscovery(port);
      break;
    case PE_SNK_Wait_For_Capabilities:
      PolicySinkWaitCaps(port);
      break;
    case PE_SNK_Evaluate_Capability:
      PolicySinkEvaluateCaps(port);
      break;
    case PE_SNK_Select_Capability:
      PolicySinkSelectCapability(port);
      break;
    case PE_SNK_Transition_Sink:
      PolicySinkTransitionSink(port);
      break;
    case PE_SNK_Ready:
      PolicySinkReady(port);
      break;
    case PE_SNK_Give_Sink_Cap:
      PolicySinkGiveSinkCap(port);
      break;
    case PE_SNK_Get_Source_Cap:
      PolicySinkGetSourceCap(port);
      break;
    case PE_DR_SNK_Get_Sink_Cap:
      PolicySinkGetSinkCap(port);
      break;
    case PE_DR_SNK_Give_Source_Cap:
      PolicySinkGiveSourceCap(port);
      break;
    case PE_DRS_UFP_DFP_Send_Swap:
      PolicySinkSendDRSwap(port);
      break;
    case PE_DRS_UFP_DFP_Evaluate_Swap:
      PolicySinkEvaluateDRSwap(port);
      break;
    case PE_PRS_SNK_SRC_Send_Swap:
      PolicySinkSendPRSwap(port);
      break;
    case PE_PRS_SNK_SRC_Evaluate_Swap:
      PolicySinkEvaluatePRSwap(port);
      break;
#ifdef FSC_HAVE_EXTENDED
    case PE_SNK_Get_PPS_Status:
      PolicyGetPPSStatus(port);
      break;
    case PE_SNK_Get_Source_Cap_Ext:
      PolicySinkGetSourceCapExt(port);
      break;
#ifdef FSC_HAVE_SRC
    case PE_DR_SNK_Give_Source_Cap_Ext:
      PolicySinkGiveSourceCapExtended(port);
      break;
#endif /* FSC_HAVE_DRP */
#endif /* FSC_HAVE_EXTENDED */
#ifdef FSC_HAVE_FRSWAP
    case PE_FRS_SNK_SRC_Send_Swap:
      PolicySinkSendFRSwap(port);
      break;
#endif /* FSC_HAVE_FRSWAP */
    case PE_SNK_Send_Not_Supported:
      PolicySendNotSupported(port);
      break;
    case PE_SNK_Not_Supported_Received:
      PolicyNotSupported(port);
      break;
#endif /* FSC_HAVE_SNK */
    case PE_VCS_Send_Swap:
      if (port->policy_is_source_) {
#ifdef FSC_HAVE_SRC
        PolicySourceSendVCONNSwap(port);
#endif /* FSC_HAVE_SRC */
      }
      else {
#ifdef FSC_HAVE_SNK
        PolicySinkSendVCONNSwap(port);
#endif /* FSC_HAVE_SNK */
      }
      break;
    case PE_VCS_Evaluate_Swap:
      if (port->policy_is_source_) {
#ifdef FSC_HAVE_SRC
        PolicySourceEvaluateVCONNSwap(port);
#endif /* FSC_HAVE_SRC */
      }
      else {
#ifdef FSC_HAVE_SNK
        PolicySinkEvaluateVCONNSwap(port);
#endif /* FSC_HAVE_SNK */
      }
      break;
    case PE_DFP_CBL_Send_Soft_Reset:
      PolicyDFPCBLSendSoftReset(port);
      break;
    case PE_DFP_CBL_Send_Cable_Reset:
      PolicyDFPCBLSendReset(port);
      break;
#ifdef FSC_HAVE_EXTENDED
    case PE_Send_Security_Request:
      PolicyGetSecurityMsg(port);
      break;
    case PE_Send_Security_Response:
      PolicySendSecurityMsg(port);
      break;
    case PE_Security_Response_Received:
      PolicySecurityMsgReceived(port);
      break;
    case PE_Give_Manufacturer_Info:
      PolicyGiveManufacturerInfo(port);
      break;
#endif /* FSC_HAVE_EXTENDED */
#ifdef FSC_HAVE_VDM
    case PE_GIVE_VDM:
      PolicyGiveVdm(port);
      break;
#endif /* FSC_HAVE_VDM */

    case PE_Send_Generic_Cmd:
      PolicySendGenericCommand(port);
      break;
    case PE_Send_Generic_Data:
      PolicySendGenericData(port);
      break;

    /* ---------- BIST Carrier Mode and Eye Pattern ----- */
    case PE_BIST_Carrier_Mode:     /* BIST Carrier Mode 2 */
      PolicyBISTCarrierMode2(port);
      break;

    case PE_BIST_Test_Data:
      PolicyBISTTestData(port);
      break;

    default:
#ifdef FSC_HAVE_VDM
      if ((port->policy_state_ >= FIRST_VDM_STATE) &&
          (port->policy_state_ <= LAST_VDM_STATE) ) {
        /* valid VDM state */
        PolicyVdm(port);
      }
      else
#endif /* FSC_HAVE_VDM */
      {
        /* invalid state, reset */
        PolicyInvalidState(port);
      }
      break;
  }
}

void PolicyErrorRecovery(struct Port *port)
{
  SetStateErrorRecovery(port);
}

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
void PolicySourceSendHardReset(struct Port *port)
{
  set_policy_state(port, PE_SRC_Hard_Reset);

  PolicySendHardReset(port, PE_SRC_Transition_To_Default, FALSE);
}

void PolicySourceSendSoftReset(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTSoftReset, 0, 0, PE_SRC_Send_Soft_Reset,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
        port->waiting_on_hr_ = TRUE;
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        port->pd_tx_status_ = txIdle;
        if ((port->policy_rx_header_.NumDataObjects == 0) &&
            (port->policy_rx_header_.MessageType == CMTAccept)) {
#ifdef FSC_HAVE_VDM
          /* Reset the cable vdm id counter */
          port->discover_id_counter_ = 0;
#endif /* FSC_HAVE_VDM */
          set_policy_state(port, PE_SRC_Send_Capabilities);
          TimerDisable(&port->policy_state_timer_);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySourceSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceSoftReset(struct Port *port)
{
  if (PolicySend(port, CMTAccept, 0, 0, PE_SRC_Send_Capabilities,
             0, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS)
  {
#ifdef FSC_HAVE_VDM
    port->discover_id_counter_ = 0;
#endif
  }
}

void PolicySourceStartup(struct Port *port)
{
  FSC_U8 i = 0;

  switch (port->policy_subindex_) {
    case 0:
      /* Reset the protocol layer */
      for (i = SOP_TYPE_SOP; i < NUM_SOP_SUPPORTED; i++) {
        ResetProtocolLayer(port, i);
      }

      /* Unmask for a source */
      port->registers_.AlertMskL.byte = 0;
      port->registers_.AlertMskL.M_TXSUCC = 1;
      port->registers_.AlertMskL.M_TX_DISC = 1;
      port->registers_.AlertMskL.M_TXFAIL = 1;
      port->registers_.AlertMskL.M_RXHRDRST = 1;
      port->registers_.AlertMskL.M_RXSTAT = 1;
      port->registers_.AlertMskL.M_CCSTAT = 1;
      WriteRegister(port, regALERTMSKL);

      /* Clear the BIST TMODE bit, if needed. */
      if(port->registers_.TcpcCtrl.BIST_TMODE == 1) {
        port->registers_.TcpcCtrl.BIST_TMODE = 0;
        WriteRegister(port, regTCPC_CTRL);
      }

      port->usb_pd_contract_.object = 0;
      port->sink_partner_max_power_ = 0;
      port->partner_caps_.object = 0;
      port->partner_caps_available_ = FALSE;
#ifdef FSC_HAVE_FRSWAP
      port->is_fr_swap_ = FALSE;
#endif /* FSC_HAVE_FRSWAP */
      port->is_pr_swap_ = FALSE;
      port->policy_is_source_ = TRUE;
      port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
      port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
      WriteRegister(port, regMSGHEADR);

      port->caps_counter_ = 0;
      port->collision_counter_ = 0;
      TimerStart(&port->policy_state_timer_, ktSrcStartupVbus);
      TimerDisable(&port->pps_timer_);
      port->policy_subindex_++;
#ifdef FSC_HAVE_VDM
      port->vdm_cbl_present_ = FALSE;
      port->vdm_check_cbl_ = (Attempts_DiscvId_SOP_P_First &&
                              DPM_IsSOPPAllowed(port)) ? TRUE : FALSE;
#endif /* FSC_HAVE_VDM */
      break;
    case 1:
      /* Wait until we reach vSafe5V and delay if coming from PR Swap */
      if ((IsVbusVSafe5V(port) &&
           (TimerExpired(&port->swap_source_start_timer_) ||
            TimerDisabled(&port->swap_source_start_timer_))) ||
          TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        TimerDisable(&port->swap_source_start_timer_);
        /* Set policy state so SOP' discovery will return to
         * PE_SRC_Send_Capabilities */
        set_policy_state(port, PE_SRC_Send_Capabilities);
#ifdef FSC_HAVE_VDM
        if (port->vdm_check_cbl_) {
            RequestDiscoverIdentity(port, SOP_TYPE_SOP1);
            port->discover_id_counter_++;
        }
        /* AUTO_VDM_INIT if needed */
        port->vdm_auto_state_ = Attempts_Discov_SOP ?
                                AUTO_VDM_INIT : AUTO_VDM_DONE;
        port->mode_entered_ = FALSE;
        port->core_svid_info_.num_svids = 0;
        for (i = 0; i < MAX_NUM_SVIDS; i++) {
          port->core_svid_info_.svids[i] = 0;
        }
        port->auto_mode_entry_pos_ = -1;
        port->svid_discvry_done_ = FALSE;
        port->svid_discv_idx_  = -1;
#endif /* FSC_HAVE_VDM */
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceDiscovery(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (TimerDisabled(&port->policy_state_timer_) == TRUE)
      {
        /* If the timer is not active then activate it. If it already
         * active then send sop' discovery before expiry. */
        TimerStart(&port->policy_state_timer_, ktTypeCSendSourceCap);
#ifdef FSC_HAVE_VDM
        if (Attempts_DiscvId_SOP_P_First &&
            port->discover_id_counter_ *
            (DPM_Retries(port, SOP_TYPE_SOP1) + 1) < MAX_DISC_ID_COUNT &&
            port->vdm_check_cbl_ == TRUE)
        {
          /* SOP' discovery has not completed so request again*/
          port->discover_id_counter_++;
          RequestDiscoverIdentity(port, SOP_TYPE_SOP1);
        }
#endif /* FSC_HAVE_VDM */
        port->policy_subindex_++;
        break;
      }
      /* fall through if timer already active */
    case 1:
      if ((port->hard_reset_counter_ > HARD_RESET_COUNT) &&
          (TimerExpired(&port->no_response_timer_))) {
        TimerDisable(&port->policy_state_timer_);
        TimerDisable(&port->no_response_timer_);
        if (port->policy_has_contract_ == TRUE) {
          /* Something went wrong... */
          set_policy_state(port, PE_ErrorRecovery);
        }
        else {
          /* Assuming no PD sink attached */
          set_policy_state(port, PE_SRC_Disabled);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        if (port->caps_counter_ > MAX_CAPS_COUNT) {
          /* No PD sink connected */
          set_policy_state(port, PE_SRC_Disabled);
        }
        else {
          set_policy_state(port, PE_SRC_Send_Capabilities);
        }
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceSendCaps(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      DPM_PrepareSrcCaps(port);

      if (PolicySend(port, DMTSourceCapabilities,
                     port->caps_header_source_.NumDataObjects * 4,
                     (FSC_U8 *)&port->caps_source_,
                     PE_SRC_Send_Capabilities, 1,
                     SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        port->is_hard_reset_ = FALSE;
        port->hard_reset_counter_ = 0;
        port->caps_counter_ = 0;
        port->waiting_on_hr_ = TRUE;
        TimerDisable(&port->no_response_timer_);
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        port->pd_tx_status_ = txIdle;
        TimerDisable(&port->policy_state_timer_);
        if ((port->policy_rx_header_.NumDataObjects == 1) &&
            (port->policy_rx_header_.MessageType == DMTRequest)) {
          set_policy_state(port, PE_SRC_Negotiate_Capability);
          DPM_SetSOPVersion(port, port->policy_rx_header_.SpecRevision);
        }
        else {
          /* Unexpected message */
          set_policy_state(port, PE_SRC_Send_Soft_Reset);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->protocol_msg_rx_ = FALSE;
        PolicySourceSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceDisabled(struct Port *port)
{
  port->usb_pd_contract_.object = 0;
  /* Idle until COMP or RX_HRDRST or RX_STAT */
  if(port->unattach_loop_counter_ == 0) {
    port->idle_ = TRUE;
  }
  else if (TimerExpired(&port->no_response_timer_)) {
    /* TODO Add "had_contract" port item. */
    TimerDisable(&port->no_response_timer_);
  }
}

void PolicySourceTransitionDefault(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      /* Transition to 0V */
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_hard_reset_ = TRUE;
        port->policy_has_contract_ = FALSE;
        port->policy_subindex_++;

        DPM_Reset(port);

        port->registers_.PwrCtrl.AUTO_DISCH = 0;
        WriteRegister(port, regPWRCTRL);

        /* Set up alert to wait for vSafe0V */
        SetVBusAlarm(port, FSC_VSAFE0V, FSC_VBUS_LVL_HIGHEST);

        port->registers_.AlertMskL.M_PORT_PWR = 0;
        WriteRegister(port, regALERTMSKL);
        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
        WriteRegister(port, regALERTMSKH);

        ClearInterrupt(port, regALERTL, MSK_I_ALARM_LO_ALL);
        ClearInterrupt(port, regALERTH, MSK_I_ALARM_HI_ALL);

        /* Disable VBUS and force discharge */
        SendCommand(port, DisableSourceVbus);

        SetVBusStopDisc(port, FSC_VSAFE0V_DISCH);

        port->registers_.PwrCtrl.DIS_VALARM = 0;
        port->registers_.PwrCtrl.FORCE_DISCH = 1;
        port->registers_.PwrCtrl.EN_VCONN = 0;

        WriteRegister(port, regPWRCTRL);

        if (!port->policy_is_dfp_) {
          port->policy_is_dfp_ = TRUE;
          port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
          WriteRegister(port, regMSGHEADR);
        }
#ifdef FSC_HAVE_DP
        DP_Initialize(port);
#endif /* FSC_HAVE_DP */
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 1:
      if(port->registers_.AlertH.I_VBUS_ALRM_LO) {
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);

        /* We've reached vSafe0V */
        port->registers_.PwrCtrl.FORCE_DISCH = 0;
        port->registers_.PwrCtrl.DIS_VALARM = 1;
        WriteRegister(port, regPWRCTRL);

        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
        WriteRegister(port, regALERTMSKH);

        TimerStart(&port->policy_state_timer_, ktSrcRecover);
        port->policy_subindex_++;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        SendCommand(port, SourceVbusDefaultV);

        /* Turn on VConn */
        if (Type_C_Sources_VCONN) {
          port->is_vconn_source_ = TRUE;
          SetVConn(port, TRUE);
        }

        port->registers_.PwrCtrl.AUTO_DISCH = 1;
        WriteRegister(port, regPWRCTRL);

        TimerStart(&port->no_response_timer_, ktNoResponse);
        set_policy_state(port, PE_SRC_Startup);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceNegotiateCap(struct Port *port)
{
  if (DPM_EvaluateRequest(port)) {
    set_policy_state(port, PE_SRC_Transition_Supply);
  }
  else {
    set_policy_state(port, PE_SRC_Capability_Response);
    notify_observers(EVENT_PD_CONTRACT_FAILED, port->port_id_, 0);
  }
}

void PolicySourceCapabilityResponse(struct Port *port)
{
  if (port->policy_has_contract_) {
    if (port->is_contract_valid_) {
      PolicySend(port, CMTReject, 0, 0, PE_SRC_Ready, 0, SOP_TYPE_SOP, FALSE);
    }
    else {
      PolicySend(port, CMTReject, 0, 0, PE_SRC_Hard_Reset,
                 0, SOP_TYPE_SOP, FALSE);
    }
  }
  else {
    PolicySend(port, CMTReject, 0, 0, PE_SRC_Wait_New_Capabilities,
               0, SOP_TYPE_SOP, FALSE);
  }
}

void PolicySourceTransitionSupply(struct Port *port)
{
  FSC_BOOL transition_success = FALSE;

  switch (port->policy_subindex_) {
    case 0:
      if (port->needs_goto_min_) {
          if (PolicySend(port, CMTGotoMin, 0, 0, PE_SRC_Transition_Supply,
                         3, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
              /* This will send GotoMin followed by PSRdy.
               * Additional support for adjusting power limits is needed
               * to make this a useful feature.
               */
              TimerStart(&port->policy_state_timer_, ktSrcTransition);
              port->needs_goto_min_ = FALSE;
          }
          break;
      }

      if (PolicySend(port, CMTAccept, 0, 0, PE_SRC_Transition_Supply,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerDisable(&port->policy_state_timer_);

        if ((port->usb_pd_contract_.object != 0) &&
            (port->policy_rx_data_obj_[0].FVRDO.ObjectPosition ==
             port->usb_pd_contract_.FVRDO.ObjectPosition) &&
            (port->caps_source_[port->policy_rx_data_obj_[0]
                                .FVRDO.ObjectPosition - 1]
                                .FPDOSupply.SupplyType == pdoTypeAugmented) &&
            (port->caps_source_[port->policy_rx_data_obj_[0]
                                .FVRDO.ObjectPosition - 1]
                                .APDO.APDOType == apdoTypePPS))
        {
          /* If contract already exists and it is PPS increase/decrease in
           * power then don't start the ktSrcTransition timer. */
        }
        else
        {
          /* Not a PPS voltage/current change  */
          TimerStart(&port->policy_state_timer_, ktSrcTransition);
        }
      }
      break;
    case 1:
      if (TimerExpired(&port->policy_state_timer_) ||
          TimerDisabled(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);

        port->usb_pd_contract_.object = port->policy_rx_data_obj_[0].object;

        if (port->source_is_apdo_) {
          /* OpVoltage is 20mv LSB - ok for platform control */
          platform_setPPSVoltage(port->port_id_,
                                 port->usb_pd_contract_.PPSRDO.OpVoltage);
          platform_setPPSCurrent(port->port_id_,
                                 port->usb_pd_contract_.PPSRDO.OpCurrent * 50);

          /* Check the transition direction - vPpsValid is +/- 100mv */
          if (port->usb_pd_contract_.PPSRDO.OpVoltage * 20 >
              (port->sink_selected_voltage_ + 100)) {
            /* Going up */
            SetVBusAlarm(port, 0,
              FSC_VBUS_LVL_L(port->usb_pd_contract_.PPSRDO.OpVoltage * 20));
            port->registers_.AlertMskL.M_VBUS_ALRM_HI = 1;
            WriteRegister(port, regALERTMSKL);
          }
          else if (port->usb_pd_contract_.PPSRDO.OpVoltage * 20 <
                   (port->sink_selected_voltage_ - 100)) {
            /* Going down */
            SetVBusAlarm(port,
              FSC_VBUS_LVL_H(port->usb_pd_contract_.PPSRDO.OpVoltage * 20),
              FSC_VBUS_LVL_HIGHEST);
            port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
            WriteRegister(port, regALERTMSKH);
          }
          else {
            /* Within existing range */
          }

          ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
          ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);

          port->sink_selected_voltage_ =
                  port->usb_pd_contract_.PPSRDO.OpVoltage * 20;

          /* Use HighV source path for PPS */
          SendCommand(port, SourceVbusHighV);

          TimerStart(&port->policy_state_timer_, ktSrcTransitionSupply);
          port->policy_subindex_++;
        }
        else if (port->caps_source_[
                 port->usb_pd_contract_.FVRDO.ObjectPosition - 1]
                .FPDOSupply.Voltage == FSC_VBUS_LVL_PD(FSC_VBUS_05_V)) {
          /* If the requested contract is 5V, the three possible cases are: */
          /* - Already at 5V - Do nothing.    */
          /* - At HV - Discharge to vSafe5V.  */
          /* - At 0V - Transition to vSafe5V. */
          if (port->registers_.PwrStat.SOURCE_VBUS) {
            /* Do nothing */
            port->policy_subindex_ = 4;
          }
          else if(port->registers_.PwrStat.SOURCE_HV) {
          //else if(platform_getHVSwitch()) {
            /* Need to discharge */
            port->registers_.PwrCtrl.DIS_VALARM = 1;
            port->registers_.PwrCtrl.AUTO_DISCH = 0;
            WriteRegister(port, regPWRCTRL);

            SetVBusStopDisc(port, FSC_VSAFE5V);

            port->registers_.AlertMskL.M_PORT_PWR = 1;
            WriteRegister(port, regALERTMSKL);
            port->registers_.AlertVDMsk.M_DISCH_SUCC = 1;
            WriteRegister(port, regALERT_VD_MSK);

            SendCommand(port, SourceVbusDefaultV);

            port->registers_.PwrCtrl.FORCE_DISCH = 1;
            WriteRegister(port, regPWRCTRL);

            TimerStart(&port->policy_state_timer_, ktSrcTransitionSupply);
            port->policy_subindex_++;
          }
          else {
            /* Transition to vSafe5V */
            port->registers_.PwrCtrl.DIS_VALARM = 0;
            port->registers_.PwrCtrl.AUTO_DISCH = 0;
            WriteRegister(port, regPWRCTRL);

            SetVBusAlarm(port, FSC_VBUS_LVL_H(FSC_VBUS_05_V),
                    FSC_VBUS_LVL_L(FSC_VBUS_05_V));

            port->registers_.AlertMskL.M_VBUS_ALRM_HI = 1;
            port->registers_.AlertMskL.M_PORT_PWR = 1;
            port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
            WriteRegisters(port, regALERTMSKL, 2);

            ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
            ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);

            SendCommand(port, SourceVbusDefaultV);
            TimerStart(&port->policy_state_timer_, ktSrcTransitionSupply);
            port->policy_subindex_++;
          }

          port->sink_selected_voltage_ = FSC_VBUS_05_V;
        }
        else { /* Higher Voltage Option */
          /* If the requested contract is HV, the two possible cases are: */
          /* - Already at HV - Do nothing.   */
          /* - At LV - Ramp up to HV. */
          if(port->registers_.PwrStat.SOURCE_HV && !port->source_is_apdo_) {
          //if(platform_getHVSwitch()) {
            /* If the supply is already enabled, go to PS_READY */
            port->policy_subindex_ = 4;
          }
          else {
            /* Go to HV */
            port->registers_.PwrCtrl.DIS_VALARM = 0;
            port->registers_.PwrCtrl.AUTO_DISCH = 0;
            WriteRegister(port, regPWRCTRL);

            /* Using PPS Supply */
            platform_setPPSVoltage(port->port_id_,
                                   port->pd_HV_option_ / 20); /* 20mv units */
            platform_setPPSCurrent(port->port_id_,
                                   port->usb_pd_contract_.FVRDO.OpCurrent * 10);

            SetVBusAlarm(port, FSC_VBUS_LVL_H(port->pd_HV_option_),
                    FSC_VBUS_LVL_L(port->pd_HV_option_));

            port->registers_.AlertMskL.M_VBUS_ALRM_HI = 1;
            port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
            WriteRegisters(port, regALERTMSKL, 2);

            ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
            ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);

            SendCommand(port, SourceVbusHighV);

            /* Set the policy state timer to wait for transition */
            TimerStart(&port->policy_state_timer_, ktSrcTransitionSupply);
            port->policy_subindex_++;
          }

          port->sink_selected_voltage_ = port->pd_HV_option_;
        }
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      /* Verify we've hit our target voltage, or the transition timer expires */
      if (port->registers_.AlertH.I_VBUS_ALRM_LO ||
          port->registers_.AlertL.I_VBUS_ALRM_HI ||
          IsVbusInRange(port, port->sink_selected_voltage_)) {
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
        ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);
        transition_success = TRUE;
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        transition_success = TRUE;
      }
      else {
          port->idle_ = TRUE;
      }

      /* Clear I_PORT_PWR if it appears and continue monitoring vbus */
      if (port->registers_.AlertL.I_PORT_PWR) {
        ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
      }

      if (transition_success) {
        port->registers_.AlertMskL.M_PORT_PWR = 0;
        port->registers_.AlertMskL.M_VBUS_ALRM_HI = 0;
        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
        WriteRegisters(port, regALERTMSKL, 2);
        port->registers_.AlertVDMsk.M_DISCH_SUCC = 0;
        WriteRegister(port, regALERT_VD_MSK);

        port->registers_.PwrCtrl.AUTO_DISCH = 1;
        port->registers_.PwrCtrl.FORCE_DISCH = 0;
        port->registers_.PwrCtrl.DIS_VALARM = 0;
        WriteRegister(port, regPWRCTRL);
        /* Optional delay to allow for external switching delays */
        TimerStart(&port->policy_state_timer_, ktSwitchDelay);

        port->policy_subindex_++;
      }
      break;
    case 3:
        if (TimerExpired(&port->policy_state_timer_)) {
            TimerDisable(&port->policy_state_timer_);
            port->policy_subindex_++;
        }
        else {
          port->idle_ = TRUE;
        }
        break;
    case 4:
      if (PolicySend(port, CMTPS_RDY, 0, 0, PE_SRC_Ready,
                     0, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        /* Have entered into a new contract */
        if (port->policy_has_contract_ == FALSE) {
          port->policy_has_contract_ = TRUE;
          SetSinkTx(port, SinkTxNG);
        }

        if (port->source_is_apdo_) {
          TimerStart(&port->pps_timer_, ktPPSTimeout);
        }
        else {
          TimerDisable(&port->pps_timer_);
        }
        notify_observers(EVENT_PD_NEW_CONTRACT, port->port_id_,
                         &port->usb_pd_contract_);
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceReady(struct Port *port)
{
  if (port->protocol_msg_rx_) {
    port->protocol_msg_rx_ = FALSE;
    if (port->policy_rx_header_.NumDataObjects == 0) {
      switch (port->policy_rx_header_.MessageType) {
        case CMTGetSourceCap:
          set_policy_state(port, PE_SRC_Send_Capabilities);
          break;
#ifdef FSC_HAVE_SNK
        case CMTGetSinkCap:
          set_policy_state(port, PE_DR_SRC_Give_Sink_Cap);
          break;
#endif /* FSC_HAVE_SNK */
        case CMTDR_Swap:
          set_policy_state(port, PE_DRS_DFP_UFP_Evaluate_Swap);
          break;
#ifdef FSC_HAVE_SNK
        case CMTPR_Swap:
          set_policy_state(port, PE_PRS_SRC_SNK_Evaluate_Swap);
          break;
#endif /* FSC_HAVE_SNK */
#ifdef FSC_HAVE_FRSWAP
        case CMTFR_Swap:
          set_policy_state(port, PE_FRS_SRC_SNK_Evaluate_Swap);
          break;
#endif /* FSC_HAVE_FRSWAP */
        case CMTVCONN_Swap:
          set_policy_state(port, PE_VCS_Evaluate_Swap);
          break;
        case CMTSoftReset:
          set_policy_state(port, PE_SRC_Soft_Reset);
          break;
#ifdef FSC_HAVE_EXTENDED
        case CMTGetPPSStatus:
          set_policy_state(port, PE_SRC_Give_PPS_Status);
          break;
        case CMTGetSourceCapExt:
          set_policy_state(port, PE_SRC_Give_Source_Cap_Ext);
          break;
#endif /* FSC_HAVE_EXTENDED */
        case CMTReject:
        case CMTNotSupported:
            /* Rx'd Reject/NS are ignored - notify DPM if needed */
            break;
        case CMTAccept:
        case CMTWait:
          /* Unexpected messages */
          set_policy_state(port, PE_SRC_Send_Soft_Reset);
          break;
        default:
          /* Send a reject message for all other commands */
          set_policy_state(port, PE_SRC_Send_Not_Supported);
          break;
      }
      port->pd_tx_status_ = txIdle;
    }
    else if (port->policy_rx_header_.Extended == 1) {
      switch (port->policy_rx_header_.MessageType) {
#ifdef FSC_HAVE_EXTENDED
#if 0
      /* Not supporting Security req/response - just for testing chunking */
      case EMTSecurityRequest:
        set_policy_state(port, PE_Send_Security_Response);
        break;
      case EMTSecurityResponse:
        set_policy_state(port, PE_Security_Response_Received);
        break;
#endif /* 0 */
      case EMTGetManufacturerInfo:
        set_policy_state(port, PE_Give_Manufacturer_Info);
        break;
#endif /* FSC_HAVE_EXTENDED */
      default:
#ifndef FSC_HAVE_EXTENDED
        port->protocol_ext_header_.byte[0] =
            port->policy_rx_data_obj_[0].byte[0];
        port->protocol_ext_header_.byte[1] =
            port->policy_rx_data_obj_[0].byte[1];
        if (port->protocol_ext_header_.DataSize > MAX_EXT_MSG_LEGACY_LEN) {
          port->wait_for_not_supported_ = TRUE;
        } else {
          port->wait_for_not_supported_ = FALSE;
        }
#endif /* FSC_HAVE_EXTENDED */
        set_policy_state(port, PE_SRC_Send_Not_Supported);
        break;
      }
      port->pd_tx_status_ = txIdle;
    }
    else {
      switch (port->policy_rx_header_.MessageType) {
        case DMTSourceCapabilities:
        case DMTSinkCapabilities:
          break;
        case DMTRequest:
          set_policy_state(port, PE_SRC_Negotiate_Capability);
          break;
#ifdef FSC_HAVE_VDM
        case DMTVendorDefined:
          ConvertAndProcessVdmMessage(port);
          break;
#endif /* FSC_HAVE_VDM */
        case DMTBIST:
          ProcessDmtBist(port);
          break;
        default:
          /* Otherwise we've rec'd a message we don't know how to handle yet */
          set_policy_state(port, PE_SRC_Send_Not_Supported);
          break;
      }
      port->policy_subindex_ = 0;
      port->pd_tx_status_ = txIdle;
    }
  }
  else if (port->pd_tx_flag_) {
    port->policy_is_ams_ = TRUE;
    if (port->pd_transmit_header_.NumDataObjects == 0) {
      switch (port->pd_transmit_header_.MessageType) {
        case CMTGetSinkCap:
          set_policy_state(port, PE_SRC_Get_Sink_Cap);
          break;
        case CMTGetSourceCap:
          set_policy_state(port, PE_DR_SRC_Get_Source_Cap);
          break;
        case CMTPing:
          set_policy_state(port, PE_SRC_Ping);
          break;
        case CMTGotoMin:
          port->needs_goto_min_ = TRUE;
          set_policy_state(port, PE_SRC_Transition_Supply);
          break;
#ifdef FSC_HAVE_SNK
        case CMTPR_Swap:
          set_policy_state(port, PE_PRS_SRC_SNK_Send_Swap);
          break;
#endif /* FSC_HAVE_SNK */
        case CMTDR_Swap:
          set_policy_state(port, PE_DRS_DFP_UFP_Send_Swap);
          break;
        case CMTVCONN_Swap:
          set_policy_state(port, PE_VCS_Send_Swap);
          break;
        case CMTSoftReset:
          set_policy_state(port, PE_SRC_Send_Soft_Reset);
          break;
        default:
          /* Unknown command */
#ifdef FSC_DEBUG
          set_policy_state(port, PE_Send_Generic_Cmd);
#endif /* FSC_DEBUG */
          break;
      }
      port->pd_tx_status_ = txIdle;
    }
    else {
      switch (port->pd_transmit_header_.MessageType) {
        case DMTSourceCapabilities:
          set_policy_state(port, PE_SRC_Send_Capabilities);
          break;
        case DMTVendorDefined:
          port->policy_subindex_ = 0;
#ifdef FSC_HAVE_VDM
          DoVdmCommand(port);
#endif /* FSC_HAVE_VDM */
          break;
        default:
#ifdef FSC_DEBUG
          set_policy_state(port, PE_Send_Generic_Data);
#endif /* FSC_DEBUG */
          break;
      }
    }
    port->pd_tx_flag_ = FALSE;
  }
  else if (port->partner_caps_.object == 0) {
    if (!port->policy_wait_on_sink_caps_) {
      TimerStart(&port->policy_state_timer_, 20 * kMSTimeFactor);
      port->policy_wait_on_sink_caps_ = TRUE;
      port->idle_ = TRUE;
    }
    else {
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SRC_Get_Sink_Cap);
        port->policy_is_ams_ = TRUE;

        port->pd_tx_status_ = txIdle;
        port->policy_wait_on_sink_caps_ = FALSE;
      }
      else {
        port->idle_ = TRUE;
      }
    }
  }
  else if (port->cbl_rst_state_ > CBL_RST_DISABLED)
  {
       ProcessCableResetState(port);
  }
  else if ((port->port_type_ == USBTypeC_DRP) &&
           (port->req_pr_swap_as_src_ == TRUE) &&
           (port->partner_caps_.FPDOSink.DualRolePower == 1))
  {
      port->policy_is_ams_ = TRUE;
      set_policy_state(port, PE_PRS_SRC_SNK_Send_Swap);
  }
#ifdef FSC_HAVE_VDM
  else if (port->vdm_check_cbl_ && DPM_IsSOPPAllowed(port)) {
    RequestDiscoverIdentity(port, SOP_TYPE_SOP1);
    port->vdm_check_cbl_ = FALSE;
  }
  else if (port->policy_is_dfp_ &&
           port->vdm_auto_state_ != AUTO_VDM_DONE) {
    AutoVdmDiscovery(port);
  }
#endif /* FSC_HAVE_VDM */
  else if (port->req_dr_swap_To_ufp_as_src_ == TRUE &&
           port->policy_is_dfp_ == TRUE &&
           DR_Swap_To_UFP_Supported)
  {
    port->req_dr_swap_To_ufp_as_src_ = FALSE;
    port->policy_is_ams_ = TRUE;
    set_policy_state(port, PE_DRS_DFP_UFP_Send_Swap);
  }
  else if (port->req_vconn_swap_to_off_as_src_ == TRUE &&
           GetVConn(port) == TRUE &&
           VCONN_Swap_To_Off_Supported)
  {
    port->req_vconn_swap_to_off_as_src_ = FALSE;
    port->policy_is_ams_ = TRUE;
    set_policy_state(port, PE_VCS_Send_Swap);
  }
  else if (port->renegotiate_) {
    port->renegotiate_ = FALSE;
    port->policy_is_ams_ = TRUE;

    set_policy_state(port, PE_SRC_Send_Capabilities);
    port->pd_tx_status_ = txIdle;
  }
  else if (TimerExpired(&port->pps_timer_))
  {
    /* No PPS re-request within time limit */
    PolicySourceSendHardReset(port);
    TimerDisable(&port->pps_timer_);
  }
  else
  {
    /* Wait for COMP or RX_HRDRST or RX_STAT */
    port->idle_ = TRUE;
    TimerDisable(&port->policy_state_timer_);
    TimerDisable(&port->no_response_timer_);
    SetSinkTx(port, SinkTxOK);
  }
}

void PolicySourceGiveSourceCap(struct Port *port)
{
  set_policy_state(port, PE_SRC_Send_Capabilities);
  PolicySourceSendCaps(port);
}

#if defined(FSC_HAVE_SRC) && defined(FSC_HAVE_EXTENDED)
void PolicySourceGiveSourceCapExtended(struct Port *port)
{
  PolicySend(port, EMTSourceCapsExtended, 24, port->policy_src_cap_ext_.byte,
             PE_SRC_Ready, 0, SOP_TYPE_SOP, TRUE);
}
#endif /* defined(FSC_HAVE_SRC) && defined(FSC_HAVE_EXTENDED) */

void PolicySourceGetSinkCap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTGetSinkCap, 0, 0, PE_SRC_Get_Sink_Cap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        port->pd_tx_status_ = txIdle;
        if ((port->policy_rx_header_.NumDataObjects > 0) &&
            (port->policy_rx_header_.MessageType == DMTSinkCapabilities)) {
          UpdateCapabilitiesRx(port, FALSE);
          port->partner_caps_available_ = TRUE;
          set_policy_state(port, PE_SRC_Ready);
        }
        else {
          /* No valid sink caps message */
          PolicySourceSendSoftReset(port);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SRC_Ready);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceGiveSinkCap(struct Port *port)
{
#ifdef FSC_HAVE_DRP
  if ((port->protocol_msg_rx_sop_ == SOP_TYPE_SOP) &&
      (port->port_type_ == USBTypeC_DRP))
    PolicySend(port, DMTSinkCapabilities,
               port->caps_header_sink_.NumDataObjects * 4,
               (FSC_U8 *)&port->caps_sink_,
               PE_SRC_Ready, 0, SOP_TYPE_SOP, FALSE);
  else
#endif /* FSC_HAVE_DRP */
    PolicySendNotSupported(port);
}

void PolicySourceGetSourceCap(struct Port *port)
{
    switch (port->policy_subindex_) {
      case 0:
        if (PolicySend(port, CMTGetSourceCap, 0, 0, PE_DR_SRC_Get_Source_Cap,
                       1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
          TimerStart(&port->policy_state_timer_, ktSenderResponse);
        }
        break;
      case 1:
        if (port->protocol_msg_rx_) {
          port->protocol_msg_rx_ = FALSE;
          port->pd_tx_status_ = txIdle;
          if ((port->policy_rx_header_.NumDataObjects > 0) &&
              (port->policy_rx_header_.MessageType == DMTSourceCapabilities)) {
            UpdateCapabilitiesRx(port, TRUE);
            port->partner_caps_available_ = TRUE;
            set_policy_state(port, PE_SRC_Ready);
          }
          else if ((port->policy_rx_header_.NumDataObjects == 0) &&
                   (port->policy_rx_header_.MessageType == CMTReject ||
                    port->policy_rx_header_.MessageType == CMTNotSupported)) {
            set_policy_state(port, PE_SRC_Ready);
          }
          else {
            set_policy_state(port, PE_SRC_Send_Soft_Reset);
          }
        }
        else if (TimerExpired(&port->policy_state_timer_)) {
          TimerDisable(&port->policy_state_timer_);
          set_policy_state(port, PE_SRC_Ready);
        }
        else {
          port->idle_ = TRUE;
        }
        break;
      default:
        set_policy_state(port, PE_ErrorRecovery);
        break;
    }
}


void PolicySourceSendPing(struct Port *port)
{
  PolicySend(port, CMTPing, 0, 0, PE_SRC_Ready, 0, SOP_TYPE_SOP, FALSE);
}

void PolicySourceSendDRSwap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTDR_Swap, 0, 0, PE_DRS_DFP_UFP_Send_Swap,
                          1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          /* Received a control message */
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->policy_is_dfp_ = (port->policy_is_dfp_ == TRUE)?FALSE:TRUE;
              port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
              WriteRegister(port, regMSGHEADR);
              DPM_ReConfigureRxDetect(port);
              set_policy_state(port, PE_SRC_Ready);
              notify_observers(port->policy_is_dfp_ == TRUE
                               ? EVENT_DATA_ROLE_DFP : EVENT_DATA_ROLE_UFP,
                               port->port_id_, 0);
              break;
            case CMTReject:
            case CMTNotSupported:
            case CMTWait:
              set_policy_state(port, PE_SRC_Ready);
              break;
            default:
              set_policy_state(port, PE_SRC_Send_Soft_Reset);
              break;
          }
        }
        else {
          /* Received data message */
          set_policy_state(port, PE_SRC_Send_Soft_Reset);
        }
        port->pd_tx_status_ = txIdle;
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SRC_Ready);
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;  }
}

void PolicySourceEvaluateDRSwap(struct Port *port)
{
#ifdef FSC_HAVE_VDM
  if (port->mode_entered_ == TRUE) {
    PolicySourceSendHardReset(port);
    return;
  }
#endif /* FSC_HAVE_VDM */
  if (port->protocol_msg_rx_sop_ != SOP_TYPE_SOP ||
      (port->policy_is_dfp_ && !DR_Swap_To_UFP_Supported) ||
      (!port->policy_is_dfp_ && !DR_Swap_To_DFP_Supported)) {
    PolicySendNotSupported(port);
  }
  else {
    if (PolicySend(port, CMTAccept, 0, 0, PE_SRC_Ready,
            0, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
      port->policy_is_dfp_ = (port->policy_is_dfp_ == TRUE) ? FALSE : TRUE;
      port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
      WriteRegister(port, regMSGHEADR);
      DPM_ReConfigureRxDetect(port);
      notify_observers(port->policy_is_dfp_ == TRUE
                       ? EVENT_DATA_ROLE_DFP : EVENT_DATA_ROLE_UFP,
                       port->port_id_, 0);
    }
  }
}

void PolicySourceSendVCONNSwap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTVCONN_Swap, 0, 0, PE_VCS_Send_Swap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->policy_subindex_++;
              TimerDisable(&port->policy_state_timer_);
              break;
            case CMTReject:
            case CMTNotSupported:
              /* If needed, can force becoming the VConn Source */
              if (port->is_vconn_source_ == FALSE) {
                port->is_vconn_source_ = TRUE;
                SetVConn(port, TRUE);
              }
              /* Fall through */
            case CMTWait:
              set_policy_state(port, PE_SRC_Ready);
              port->pd_tx_status_ = txIdle;
              break;
            default:
              set_policy_state(port, PE_SRC_Send_Soft_Reset);
              break;
          }
        }
        else {
          /* Received data message */
          set_policy_state(port, PE_SRC_Send_Soft_Reset);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SRC_Ready);
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (port->is_vconn_source_) {
        TimerStart(&port->policy_state_timer_, ktVCONNSourceOn);
        port->policy_subindex_++;
      }
      else {
        /* Apply VConn */
        port->is_vconn_source_ = TRUE;
        SetVConn(port, TRUE);

        /* Skip next state and send the PS_RDY msg */
        port->policy_subindex_ = 4;
      }
      break;
    case 3:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              /* Disable VCONN source */
              port->is_vconn_source_ = FALSE;
              SetVConn(port, FALSE);

              set_policy_state(port, PE_SRC_Ready);
              port->pd_tx_status_ = txIdle;
              break;
            default:
              /* Ignore all other commands received */
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySourceSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 4:
      PolicySend(port, CMTPS_RDY, 0, 0, PE_SRC_Ready, 0, SOP_TYPE_SOP, FALSE);
#ifdef FSC_HAVE_VDM
      port->vdm_check_cbl_ = (port->vdm_cbl_present_ == FALSE &&
                              DPM_IsSOPPAllowed(port)) ? TRUE : FALSE;
#endif /* FSC_HAVE_VDM */
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceEvaluateVCONNSwap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if ((port->protocol_msg_rx_sop_ != SOP_TYPE_SOP) ||
          (port->is_vconn_source_ && !VCONN_Swap_To_Off_Supported) ||
          (!port->is_vconn_source_ && !VCONN_Swap_To_On_Supported)) {
        PolicySendNotSupported(port);
      }
      else {
        PolicySend(port, CMTAccept, 0, 0, PE_VCS_Evaluate_Swap,
                   1, SOP_TYPE_SOP, FALSE);
      }
      break;
    case 1:
      if (port->is_vconn_source_) {
        TimerStart(&port->policy_state_timer_, ktVCONNSourceOn);
        port->policy_subindex_++;
      }
      else {
        /* Apply VConn */
        port->is_vconn_source_ = TRUE;
        SetVConn(port, TRUE);
        port->policy_subindex_ = 3;
      }
      break;
    case 2:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              /* Disable VConn */
              port->is_vconn_source_ = FALSE;
              SetVConn(port, FALSE);

              set_policy_state(port, PE_SRC_Ready);
              port->pd_tx_status_ = txIdle;
              break;
            default:
              /* Ignore all other commands received */
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySourceSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 3:
      PolicySend(port, CMTPS_RDY, 0, 0, PE_SRC_Ready,
                 0, port->protocol_msg_rx_sop_, FALSE);
#ifdef FSC_HAVE_VDM
      port->vdm_check_cbl_ = (port->vdm_cbl_present_ == FALSE &&
                              DPM_IsSOPPAllowed(port)) ? TRUE : FALSE;
#endif /* FSC_HAVE_VDM */
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySourceSendPRSwap(struct Port *port)
{
#ifdef FSC_HAVE_DRP
  FSC_U8 status = 0;
  port->req_pr_swap_as_src_ = FALSE;
  switch (port->policy_subindex_) {
    case 0:
      /* Send the PRSwap command */
      if (PolicySend(port, CMTPR_Swap, 0, 0, PE_PRS_SRC_SNK_Send_Swap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      /* Require Accept message to move on or go back to ready state */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->is_pr_swap_ = TRUE;
              port->policy_has_contract_ = FALSE;
              TimerStart(&port->policy_state_timer_, ktSrcTransition);
              port->policy_subindex_++;
              break;
            case CMTWait:
            case CMTReject:
            case CMTNotSupported:
              set_policy_state(port, PE_SRC_Ready);
              port->is_pr_swap_ = FALSE;
              port->pd_tx_status_ = txIdle;
              break;
            default:
              /* Interrupted */
              set_policy_state(port, PE_SRC_Send_Soft_Reset);
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SRC_Ready);
        port->is_pr_swap_ = FALSE;
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      /* Wait for tSrcTransition and then turn off power (and Rd on/Rp off) */
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        /* Disable VBUS, set alarm (vSafe0V), and force discharge */
        port->registers_.PwrCtrl.AUTO_DISCH = 0;
        port->registers_.PwrCtrl.DIS_VBUS_MON = 0;
        WriteRegister(port, regPWRCTRL);

        SetVBusAlarm(port, FSC_VSAFE0V, FSC_VBUS_LVL_HIGHEST);
        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
        WriteRegister(port, regALERTMSKH);
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);

        SendCommand(port, DisableSourceVbus);
        port->source_or_sink_ = Sink;

        SetVBusStopDisc(port, FSC_VSAFE0V_DISCH);

        port->registers_.PwrCtrl.FORCE_DISCH = 1;
        port->registers_.PwrCtrl.DIS_VALARM = 0;
        WriteRegister(port, regPWRCTRL);

        port->policy_subindex_++;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 3:
      if (port->registers_.AlertH.I_VBUS_ALRM_LO) {
        /* We've reached vSafe0V */
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);

        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
        WriteRegister(port, regALERTMSKH);

        port->registers_.PwrCtrl.FORCE_DISCH = 0;
        port->registers_.PwrCtrl.DIS_VALARM = 1;
        WriteRegister(port, regPWRCTRL);

        RoleSwapToAttachedSink(port);

        port->policy_is_source_ = FALSE;
        port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
        WriteRegister(port, regMSGHEADR);
        port->policy_subindex_++;
      }
      break;
    case 4:
      /* Allow time for the supply to fall and then send the PS_RDY message */
      status = PolicySend(port, CMTPS_RDY, 0, 0,  PE_PRS_SRC_SNK_Send_Swap,
                          5, SOP_TYPE_SOP, FALSE);
      if (status == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktPSSourceOn);
      }
      else if (status == STAT_ERROR)
        set_policy_state(port, PE_ErrorRecovery);
      break;
    case 5:
      /* Wait to receive a PS_RDY message from the new DFP */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              port->policy_subindex_++;
              TimerStart(&port->policy_state_timer_, ktGoodCRCDelay);
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_pr_swap_ = FALSE;

        /* Note: Compliance testing seems to require BOTH HR and ER here. */
        PolicySourceSendHardReset(port);
        set_policy_state(port, PE_ErrorRecovery);

        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 6:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SNK_Startup);

        SetVBusSnkDisc(port, FSC_VSAFE5V_DISC);

        ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
        ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);
        ClearInterrupt(port, regALERTL, MSK_I_CCSTAT);

        port->registers_.AlertMskH.M_VBUS_SNK_DISC = 1;
        //port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
        WriteRegister(port, regALERTMSKH);

        port->registers_.PwrCtrl.AUTO_DISCH = 1;
        WriteRegister(port, regPWRCTRL);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
#endif /* FSC_HAVE_DRP */
}

void PolicySourceEvaluatePRSwap(struct Port *port)
{
#ifdef FSC_HAVE_DRP
  FSC_U8 status;
  switch (port->policy_subindex_) {
    case 0:
      /* Sending accept or reject */
      if (port->protocol_msg_rx_sop_ != SOP_TYPE_SOP ||
          port->caps_source_[0].FPDOSupply.DualRolePower == FALSE ||
          Accepts_PR_Swap_As_Src == FALSE) {
        /* Send the reject/NS if we are not a DRP */
        PolicySendNotSupported(port);
      }
      else if (!port->partner_caps_available_ ||
               (port->caps_header_received_.MessageType !=
                   DMTSourceCapabilities)) {
        /* Reject and try to get updated caps from port partner */
        PolicySend(port, CMTWait, 0, 0, PE_DR_SRC_Get_Source_Cap,
                   0, port->protocol_msg_rx_sop_, FALSE);
      }
      else if (port->partner_caps_.FPDOSupply.DualRolePower == FALSE ||
               (port->caps_source_[0].FPDOSupply.ExternallyPowered == TRUE &&
                port->partner_caps_.FPDOSupply.SupplyType == pdoTypeFixed &&
                port->partner_caps_.FPDOSupply.ExternallyPowered == FALSE)) {
        /* Send the reject if partner is not a DRP or Ext powered */
        PolicySend(port, CMTReject, 0, 0, PE_SRC_Ready, 0,
                   port->protocol_msg_rx_sop_, FALSE);
      }
      else {
        if (PolicySend(port, CMTAccept, 0, 0, PE_PRS_SRC_SNK_Evaluate_Swap,
                        1, port->protocol_msg_rx_sop_, FALSE) == STAT_SUCCESS) {
          /* Disable auto discharge here to prevent an automatic detach */
          port->registers_.PwrCtrl.AUTO_DISCH = 0;
          WriteRegister(port, regPWRCTRL);

          port->is_pr_swap_ = TRUE;
          port->policy_has_contract_ = FALSE;

          TimerStart(&port->policy_state_timer_, ktSrcTransition);
        }
      }
      break;
    case 1:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        /* Set alarm (vSafe0V), and force discharge */
        port->registers_.PwrCtrl.DIS_VBUS_MON = 0;
        WriteRegister(port, regPWRCTRL);

        SetVBusAlarm(port, FSC_VSAFE0V, FSC_VBUS_LVL_HIGHEST);
        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
        WriteRegister(port, regALERTMSKH);
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
        port->registers_.AlertVDMsk.M_DISCH_SUCC = 1;
        WriteRegister(port, regALERT_VD_MSK);
        ClearInterrupt(port, regALERT_VD, MSK_I_DISCH_SUCC);

        SendCommand(port, DisableSourceVbus);
        port->source_or_sink_ = Sink;

        SetVBusStopDisc(port, FSC_VSAFE0V_DISCH);

        port->registers_.PwrCtrl.FORCE_DISCH = 1;
        port->registers_.PwrCtrl.DIS_VALARM = 0;
        WriteRegister(port, regPWRCTRL);

        port->policy_subindex_++;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (port->registers_.AlertH.I_VBUS_ALRM_LO ||
          port->registers_.AlertVD.I_DISCH_SUCC) {
        /* We've reached vSafe0V */
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
        ClearInterrupt(port, regALERT_VD, MSK_I_DISCH_SUCC);

        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
        WriteRegister(port, regALERTMSKH);
        port->registers_.AlertVDMsk.M_DISCH_SUCC = 0;
        WriteRegister(port, regALERT_VD_MSK);

        port->registers_.PwrCtrl.FORCE_DISCH = 0;
        port->registers_.PwrCtrl.DIS_VALARM = 1;
        WriteRegister(port, regPWRCTRL);

        RoleSwapToAttachedSink(port);

        port->policy_is_source_ = FALSE;
        port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
        WriteRegister(port, regMSGHEADR);

        port->policy_subindex_++;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 3:
      port->req_pr_swap_as_src_ = FALSE;
      status = PolicySend(port, CMTPS_RDY, 0, 0,
                          PE_PRS_SRC_SNK_Evaluate_Swap, 4,
                          port->protocol_msg_rx_sop_, FALSE);
      if (status == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktPSSourceOn);
      }
      else if (status == STAT_ERROR) {
        set_policy_state(port, PE_ErrorRecovery);
      }
      break;
    case 4:
      /* Wait to receive a PS_RDY message from the new DFP */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              TimerDisable(&port->policy_state_timer_);
              port->policy_subindex_++;
              TimerStart(&port->policy_state_timer_, ktGoodCRCDelay);
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_pr_swap_ = FALSE;

        /* Note: Compliance testing seems to require BOTH HR and ER here. */
        PolicySourceSendHardReset(port);
        set_policy_state(port, PE_ErrorRecovery);

        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 5:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SNK_Startup);

        SetVBusSnkDisc(port, FSC_VSAFE5V_DISC);

        SendCommand(port, SinkVbus);

        ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);
        ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);
        ClearInterrupt(port, regALERTL, MSK_I_CCSTAT);

        port->registers_.AlertMskH.M_VBUS_SNK_DISC = 1;
        //port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
        WriteRegister(port, regALERTMSKH);

        port->registers_.PwrCtrl.AUTO_DISCH = 1;
        WriteRegister(port, regPWRCTRL);
        port->is_pr_swap_ = FALSE;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
#else
  PolicySendNotSupported(port);
#endif /* FSC_HAVE_DRP */
}

#ifdef FSC_HAVE_FRSWAP
void PolicySourceEvaluateFRSwap(struct Port *port)
{
  FSC_U8 status;
  switch (port->policy_subindex_) {
    case 0:
      /* Sending accept */
      if (PolicySend(port, CMTAccept, 0, 0, PE_FRS_SRC_SNK_Evaluate_Swap,
                     1, port->protocol_msg_rx_sop_, FALSE) == STAT_SUCCESS) {
          /* Disable auto discharge here to prevent an automatic detach */
          port->registers_.PwrCtrl.AUTO_DISCH = 0;
          WriteRegister(port, regPWRCTRL);

          port->is_fr_swap_ = TRUE;
          port->policy_has_contract_ = FALSE;

          /* Not an official time - just an error time out option */
          TimerStart(&port->policy_state_timer_, ktSrcRecover);
      }
      break;
    case 1:
      /* Swap terminations */
      RoleSwapToAttachedSink(port);

      port->policy_is_source_ = FALSE;

      port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
      WriteRegister(port, regMSGHEADR);
      port->policy_subindex_++;
      break;
    case 2:
      /* Send PS_RDY */
      status = PolicySend(port, CMTPS_RDY, 0, 0,
                          PE_FRS_SRC_SNK_Evaluate_Swap, 3,
                          port->protocol_msg_rx_sop_, FALSE);
      if (status == STAT_SUCCESS) {
      }
      else if (status == STAT_ERROR ||
               TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_ErrorRecovery);
      }
      break;
    case 3:
      /* Wait to receive a PS_RDY message from the new DFP */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              TimerDisable(&port->policy_state_timer_);
              port->is_fr_swap_ = FALSE;

              port->registers_.PwrCtrl.AUTO_DISCH = 1;
              WriteRegister(port, regPWRCTRL);

              set_policy_state(port, PE_SNK_Startup);
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_fr_swap_ = FALSE;

        set_policy_state(port, PE_ErrorRecovery);

        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}
#endif /* FSC_HAVE_FRSWAP */

void PolicySourceWaitNewCapabilities(struct Port *port)
{
  if (port->unattach_loop_counter_ == 0) {
    port->idle_ = TRUE;
    port->registers_.AlertMskL.M_RXHRDRST = 1;
    port->registers_.AlertMskL.M_RXSTAT = 1;
    port->registers_.AlertMskL.M_CCSTAT = 1;
    WriteRegister(port, regALERTMSKL);
  }

  set_policy_state(port, PE_SRC_Send_Capabilities);
}

void PolicySourceChunkReceived(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      TimerStart(&port->policy_state_timer_, ktChunkingNotSupported);
      port->policy_subindex_++;
      break;
    case 1:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SRC_Send_Not_Supported);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
void PolicySinkStartup(struct Port *port)
{
  FSC_U8 i = 0;

  /* Reset the protocol layer */
  for (i = SOP_TYPE_SOP; i < NUM_SOP_SUPPORTED; i++) {
    ResetProtocolLayer(port, i);
  }

  /* Enable Masks */
  port->registers_.AlertMskL.M_TX_DISC = 1;
  port->registers_.AlertMskL.M_TXFAIL = 1;
  port->registers_.AlertMskL.M_TXSUCC = 1;
  port->registers_.AlertMskL.M_RXSTAT = 1;
  port->registers_.AlertMskL.M_RXHRDRST = 1;
  WriteRegister(port, regALERTMSKL);

  /* Disable BIST TMODE bit if needed */
  if(port->registers_.TcpcCtrl.BIST_TMODE == 1) {
    port->registers_.TcpcCtrl.BIST_TMODE = 0;
    WriteRegister(port, regTCPC_CTRL);
  }

  port->sink_selected_voltage_ = FSC_VBUS_05_V;

  port->usb_pd_contract_.object = 0;
  port->sink_partner_max_power_ = 0;
  port->partner_caps_.object = 0;
#ifdef FSC_HAVE_FRSWAP
  port->is_fr_swap_ = FALSE;
#endif /* FSC_HAVE_FRSWAP */
  port->is_pr_swap_ = FALSE;
  port->is_hard_reset_ = FALSE;
  port->policy_is_source_ = FALSE;
  port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
  port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
  WriteRegister(port, regMSGHEADR);

  /* If coming out of a hard reset, this will signal the Type-C state
   * machine to re-check the VBus level.
   */
  port->registers_.AlertL.I_PORT_PWR = 1;

  port->caps_counter_ = 0;
  port->collision_counter_ = 0;
  TimerDisable(&port->no_response_timer_);
  TimerDisable(&port->policy_state_timer_);
  TimerDisable(&port->pps_timer_);
  set_policy_state(port, PE_SNK_Discovery);

#ifdef FSC_HAVE_VDM
  port->vdm_auto_state_ = Attempts_Discov_SOP ? AUTO_VDM_INIT : AUTO_VDM_DONE;
  port->mode_entered_ = FALSE;
  port->core_svid_info_.num_svids = 0;
  for (i = 0; i < MAX_NUM_SVIDS; i++) {
    port->core_svid_info_.svids[i] = 0;
  }
  port->auto_mode_entry_pos_ = -1;
  port->auto_mode_entry_pos_ = -1;
  port->svid_discvry_done_ = FALSE;
  port->svid_discv_idx_ = -1;
#endif /* FSC_HAVE_VDM */
}

void PolicySinkSendHardReset(struct Port *port)
{
  set_policy_state(port, PE_SNK_Hard_Reset);

  PolicySendHardReset(port, PE_SNK_Transition_To_Default, FALSE);
}

void PolicySinkSoftReset(struct Port *port)
{
  if (PolicySend(port, CMTAccept, 0, 0,  PE_SNK_Wait_For_Capabilities,
                 0, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
#ifdef FSC_HAVE_VDM
    port->discover_id_counter_ = 0;
#endif /* FSC_HAVE_VDM */
    TimerStart(&port->policy_state_timer_, ktTypeCSinkWaitCap);
  }
}

void PolicySinkSendSoftReset(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTSoftReset, 0, 0, PE_SNK_Send_Soft_Reset,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
        port->waiting_on_hr_ = TRUE;
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        TimerDisable(&port->policy_state_timer_);
        port->protocol_msg_rx_ = FALSE;
        port->pd_tx_status_ = txIdle;
        if ((port->policy_rx_header_.NumDataObjects == 0) &&
            (port->policy_rx_header_.MessageType == CMTAccept)) {
#ifdef FSC_HAVE_VDM
          port->discover_id_counter_ = 0;
#endif /* FSC_HAVE_VDM */
          set_policy_state(port, PE_SNK_Wait_For_Capabilities);
          TimerStart(&port->policy_state_timer_, ktTypeCSinkWaitCap);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySinkSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkTransitionDefault(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      port->is_hard_reset_ = TRUE;
      port->policy_has_contract_ = FALSE;

      DPM_Reset(port);

      /* Disable auto-discharge system */
      port->registers_.PwrCtrl.AUTO_DISCH = 0;
      port->registers_.PwrCtrl.DIS_VALARM = 0;
      WriteRegister(port, regPWRCTRL);

      /* Timeout (Vbus Off) handling required for Type-C only connections */
      TimerStart(&port->policy_state_timer_, ktPSHardResetMax + ktSafe0V);

      if (port->policy_is_dfp_) {
        port->policy_is_dfp_ = FALSE;
        port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
        WriteRegister(port, regMSGHEADR);
      }

      /* Disable VConn source */
      if (port->is_vconn_source_) {
        port->registers_.PwrCtrl.EN_VCONN = 0;
        WriteRegister(port, regPWRCTRL);
        port->is_vconn_source_ = FALSE;
      }

      /* Disable VBus sinking during the reset */
      SendCommand(port, DisableSinkVbus);

      /* Set up alert to wait for vSafe0V */
      SetVBusAlarm(port, FSC_VSAFE0V, FSC_VBUS_LVL_HIGHEST);

      ClearInterrupt(port, regALERTL, MSK_I_ALARM_LO_ALL);
      ClearInterrupt(port, regALERTH, MSK_I_ALARM_HI_ALL);

      port->registers_.AlertMskH.M_VBUS_ALRM_LO = 1;
      WriteRegister(port, regALERTMSKH);
#ifdef FSC_HAVE_DP
      DP_Initialize(port);
#endif /* FSC_HAVE_DP */
      port->policy_subindex_++;
      break;
    case 1:
      if (port->registers_.AlertH.I_VBUS_ALRM_LO) {
        /* We've reached vSafe0V */
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);

        /* Set up to wait for vSafe5V */
        SetVBusAlarm(port, 0, FSC_VSAFE5V_L);

        ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI | MSK_I_PORT_PWR);
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_ALRM_LO);

        port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
        WriteRegister(port, regALERTMSKH);
        port->registers_.AlertMskL.M_VBUS_ALRM_HI = 1;
        WriteRegister(port, regALERTMSKL);

        /* Timeout (Vbus On) handling required for Type-C only connections */
        TimerStart(&port->policy_state_timer_, ktSrcRecoverMax + ktSrcTurnOn);

        port->policy_subindex_++;
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);

        /* VBus never dropped - try to continue */
        set_policy_state(port, PE_SNK_Startup);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (port->registers_.AlertL.I_VBUS_ALRM_HI) {
        ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
        ClearInterrupt(port, regALERTL, MSK_I_VBUS_ALRM_HI);

        /* Re-enable sinking VBus and discharge system */
        SendCommand(port, SinkVbus);

        SetVBusSnkDisc(port, FSC_VSAFE5V_DISC);

        port->registers_.AlertMskL.M_VBUS_ALRM_HI = 0;
        port->registers_.AlertMskL.M_PORT_PWR = 1;
        WriteRegister(port, regALERTMSKL);
        port->registers_.AlertMskH.M_VBUS_SNK_DISC = 1;
        WriteRegister(port, regALERTMSKH);

        ClearInterrupt(port, regALERTL, MSK_I_PORT_PWR);
        ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);

        port->registers_.PwrCtrl.AUTO_DISCH = 1;
        port->registers_.PwrCtrl.DIS_VALARM = 1;
        WriteRegister(port, regPWRCTRL);

        set_policy_state(port, PE_SNK_Startup);
        port->pd_tx_status_ = txIdle;

        TimerDisable(&port->policy_state_timer_);
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);

        /* VBus never recovered */
        set_policy_state(port, PE_ErrorRecovery);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkDiscovery(struct Port *port)
{
  set_policy_state(port, PE_SNK_Wait_For_Capabilities);
  port->is_hard_reset_ = FALSE;
  TimerStart(&port->policy_state_timer_, ktTypeCSinkWaitCap);
}

void PolicySinkWaitCaps(struct Port *port)
{
  if (port->protocol_msg_rx_) {
    port->protocol_msg_rx_ = FALSE;
    if ((port->policy_rx_header_.NumDataObjects > 0) &&
        (port->policy_rx_header_.MessageType == DMTSourceCapabilities)) {
      UpdateCapabilitiesRx(port, TRUE);
      set_policy_state(port, PE_SNK_Evaluate_Capability);
      TimerDisable(&port->policy_state_timer_);
      DPM_SetSOPVersion(port, port->policy_rx_header_.SpecRevision);
    }
    else if ((port->policy_rx_header_.NumDataObjects == 0) &&
             (port->policy_rx_header_.MessageType == CMTSoftReset)) {
      set_policy_state(port, PE_SNK_Soft_Reset);
    }
  }
  else if ((port->policy_has_contract_ == TRUE) &&
           TimerExpired(&port->no_response_timer_) &&
           (port->hard_reset_counter_ > HARD_RESET_COUNT)) {
    TimerDisable(&port->no_response_timer_);
    set_policy_state(port, PE_ErrorRecovery);
  }
  else if (TimerExpired(&port->policy_state_timer_) &&
           (port->hard_reset_counter_ <= HARD_RESET_COUNT)) {
    TimerDisable(&port->policy_state_timer_);
    PolicySinkSendHardReset(port);
  }
  else if ((port->policy_has_contract_ == FALSE) &&
           TimerExpired(&port->no_response_timer_) &&
           (port->hard_reset_counter_ > HARD_RESET_COUNT)) {
    port->idle_ = TRUE;
    TimerDisable(&port->no_response_timer_);
  }
  else {
    port->idle_ = TRUE;
  }
}

void PolicySinkEvaluateCaps(struct Port *port)
{
  /* All math here should be in mv, ma, mw.  Conversion done on incoming
   * and outgoing values.
   */
  FSC_U8 i = 0;
  FSC_S32 req_position = 0;
  FSC_U32 obj_voltage = 0;
  FSC_U32 obj_current = 0;
  FSC_U32 sel_voltage = 0;
  FSC_U32 max_power = 0;
  FSC_U32 obj_power = 0;
  FSC_U32 req_current = 0;

  TimerDisable(&port->no_response_timer_);
  port->hard_reset_counter_ = 0;

  /* Select the highest power object that we are compatible with */
  for (i = 0; i < port->caps_header_received_.NumDataObjects; i++) {
    switch (port->caps_received_[i].PDO.SupplyType) {
      case pdoTypeFixed:
        obj_voltage = port->caps_received_[i].FPDOSupply.Voltage * 50;
        if (obj_voltage > port->sink_request_max_voltage_) {
          continue;
        }
        else {
          obj_current = port->caps_received_[i].FPDOSupply.MaxCurrent * 10;
          obj_power = (obj_voltage * obj_current) / 1000;
        }
        break;
      case pdoTypeVariable:
        obj_voltage = port->caps_received_[i].VPDO.MaxVoltage * 50;
        if (obj_voltage > port->sink_request_max_voltage_) {
          continue;
        }
        else {
          obj_voltage = port->caps_received_[i].VPDO.MinVoltage * 50;
          obj_current = port->caps_received_[i].VPDO.MaxCurrent * 10;
          obj_power = (obj_voltage * obj_current) / 1000;
        }
        break;
      case pdoTypeBattery:
        /* Ignore battery powered sources (for now) */
        obj_power = 0;
        break;
      case pdoTypeAugmented:
        //obj_voltage = port->caps_received_[i].PPSDOSupply.MaxVoltage * 100;
        //obj_current = port->caps_received_[i].PPSDOSupply.MaxCurrent * 50;
        //obj_power = (obj_voltage * obj_current) / 1000;
        /* Set object power to 0 to ignore Augmented/PPS sources (for now) */
        obj_power = 0;
        break;
      default:
        /* Ignore undefined/unsupported supply types */
        obj_power = 0;
        break;
    }

    /* Track object with highest power */
    if (obj_power >= max_power) {
      max_power = obj_power;
      sel_voltage = obj_voltage;
      req_position = i + 1;

      port->sink_partner_max_power_ = max_power;
    }
  }

  /* If another port is sinking the highest power available, we'll just */
  /* request a basic low power PDO here. */
  if (port->sink_request_low_power_) {
    if (port->caps_received_[0].PDO.SupplyType == pdoTypeFixed) {
      sel_voltage = port->caps_received_[0].FPDOSupply.Voltage * 50;
    }
    else if (port->caps_received_[0].PDO.SupplyType == pdoTypeVariable){
      sel_voltage = port->caps_received_[0].VPDO.MaxVoltage * 50;
    }
    else {
      /* Skipping battery sources for now... */
    }

    /* Make sure the first position is a 5V object */
    if (sel_voltage == (PD_05_V * 50)) {
      req_position = 1;
    }
  }

  if ((req_position > 0) && (sel_voltage > 0)) {
    port->partner_caps_.object = port->caps_received_[0].object;
    port->sink_request_.FVRDO.ObjectPosition = req_position & 0x07;
    port->sink_request_.FVRDO.GiveBack = port->sink_goto_min_compatible_;
    port->sink_request_.FVRDO.NoUSBSuspend = port->sink_usb_suspend_compatible_;
    port->sink_request_.FVRDO.USBCommCapable = port->sink_usb_comm_capable_;
    req_current = (port->sink_request_op_power_ * 1000) / sel_voltage;
    /* Set the current based on the selected voltage (in 10mA units) */
    port->sink_request_.FVRDO.OpCurrent = ((req_current / 10) & 0x3FF);
    req_current = (port->sink_request_max_power_ * 1000) / sel_voltage;
    /* Set the min/max current based on the selected voltage (in 10mA units) */
    port->sink_request_.FVRDO.MinMaxCurrent = ((req_current / 10) & 0x3FF);
    if (port->sink_goto_min_compatible_) {
      port->sink_request_.FVRDO.CapabilityMismatch = FALSE;
    }
    else {
      if (obj_current < req_current) {
        /* Indicate that we need more power */
        port->sink_request_.FVRDO.CapabilityMismatch = TRUE;
        port->sink_request_.FVRDO.MinMaxCurrent = obj_current / 10;
        port->sink_request_.FVRDO.OpCurrent = obj_current / 10;
      }
      else {
        port->sink_request_.FVRDO.CapabilityMismatch = FALSE;
      }
    }
    set_policy_state(port, PE_SNK_Select_Capability);
  }
  else {
    /* TODO: For now, we just go back to the wait state instead of */
    /* sending a reject or reset (may change in future) */
    set_policy_state(port, PE_SNK_Wait_For_Capabilities);
    TimerStart(&port->policy_state_timer_, ktTypeCSinkWaitCap);
    port->idle_ = TRUE;

    port->sink_partner_max_power_ = 0;
  }
}

void PolicySinkSelectCapability(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, DMTRequest, 4, (FSC_U8 *)&port->sink_request_,
            PE_SNK_Select_Capability, 1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
        port->waiting_on_hr_ = TRUE;
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        TimerDisable(&port->policy_state_timer_);
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->policy_has_contract_ = TRUE;
              port->source_is_apdo_ = FALSE;

              port->usb_pd_contract_.object = port->sink_request_.object;

              if ((port->caps_received_[
                port->usb_pd_contract_.FVRDO.ObjectPosition - 1].PDO.
                  SupplyType == pdoTypeAugmented) &&
                  (port->caps_received_[
                port->usb_pd_contract_.FVRDO.ObjectPosition - 1].APDO.
                  APDOType == apdoTypePPS)) {
                port->source_is_apdo_ = TRUE;
                /* Set to minimum in case of current fold back */
                port->sink_selected_voltage_ = FSC_VBUS_03_V;
                port->sink_transition_up_ = FALSE;
                port->stored_apdo_.object = port->sink_request_.object;
                TimerStart(&port->pps_timer_, ktPPSRequest);
              }
              else if (port->caps_received_[
                port->usb_pd_contract_.FVRDO.ObjectPosition - 1].PDO.SupplyType
                  == pdoTypeFixed) {
                /* TODO - Not reliable if received caps have changed/cleared */
                port->sink_transition_up_ =
                    port->sink_selected_voltage_ < port->caps_received_[
                        port->usb_pd_contract_.FVRDO.ObjectPosition - 1].
                            FPDOSupply.Voltage * 50 ? TRUE : FALSE;

                port->sink_selected_voltage_ = port->caps_received_
                    [port->usb_pd_contract_.FVRDO.ObjectPosition - 1].
                        FPDOSupply.Voltage * 50; /* mV */
              }
              else {
                /* TODO - Other supply types */
              }

              if (!port->sink_transition_up_) {
                /* Set up the new disconnect level - before the level drops */
                SetVBusSnkDisc(port,
                               FSC_VBUS_LVL_DISC(port->sink_selected_voltage_));
              }

              TimerStart(&port->policy_state_timer_, ktPSTransition);
              set_policy_state(port, PE_SNK_Transition_Sink);
              break;
            case CMTWait:
            case CMTReject:
              if (port->policy_has_contract_) {
                set_policy_state(port, PE_SNK_Ready);
              }
              else {
                set_policy_state(port, PE_SNK_Wait_For_Capabilities);
                /* Set the counter to avoid a hard reset loop */
                port->hard_reset_counter_ = HARD_RESET_COUNT + 1;
              }
              notify_observers(EVENT_PD_CONTRACT_FAILED, port->port_id_, 0);
              break;
            case CMTSoftReset:
              set_policy_state(port, PE_SNK_Soft_Reset);
              break;
            default:
              set_policy_state(port, PE_SNK_Send_Soft_Reset);
              break;
          }
        }
        else {
          switch (port->policy_rx_header_.MessageType) {
            case DMTSourceCapabilities:
              UpdateCapabilitiesRx(port, TRUE);
              set_policy_state(port, PE_SNK_Evaluate_Capability);
              break;
            default:
              set_policy_state(port, PE_SNK_Send_Soft_Reset);
              break;
          }
        }
        port->pd_tx_status_ = txIdle;
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySinkSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkTransitionSink(struct Port *port)
{
  if (port->protocol_msg_rx_) {
    port->protocol_msg_rx_ = FALSE;
    if (port->policy_rx_header_.NumDataObjects == 0) {
      /* Disable the PSTransition timer */
      TimerDisable(&port->policy_state_timer_);
      switch (port->policy_rx_header_.MessageType) {
        case CMTPS_RDY:
          /* Set up the new disconnect level */
          /* FPDO Option only for now! */
          if (port->sink_transition_up_) {
            /* Set up the new disconnect level - once we've reached our
             * new level */
            SetVBusSnkDisc(port,
                           FSC_VBUS_LVL_DISC(port->sink_selected_voltage_));
          }

          SetVBusStopDisc(port, FSC_VSAFE0V_DISCH);
          notify_observers(EVENT_PD_NEW_CONTRACT, port->port_id_,
                           &port->usb_pd_contract_);

          set_policy_state(port, PE_SNK_Ready);
          break;
        case CMTSoftReset:
          set_policy_state(port, PE_SNK_Soft_Reset);
          break;
        default:
          PolicySinkSendHardReset(port);
          break;
      }
    }
    else {
      switch (port->policy_rx_header_.MessageType) {
        case DMTSourceCapabilities:
          UpdateCapabilitiesRx(port, TRUE);
          set_policy_state(port, PE_SNK_Evaluate_Capability);
          break;
        default:
          /* Unexpected data message */
          set_policy_state(port, PE_SNK_Send_Soft_Reset);
          break;
      }
    }
    port->pd_tx_status_ = txIdle;
  }
  else if (TimerExpired(&port->policy_state_timer_)) {
    TimerDisable(&port->policy_state_timer_);
    PolicySinkSendHardReset(port);
  }
  else {
    port->idle_ = TRUE;
  }
}

void PolicySinkReady(struct Port *port)
{
  if (port->protocol_msg_rx_) {
    port->protocol_msg_rx_ = FALSE;
    if (port->policy_rx_header_.NumDataObjects == 0) {
      switch (port->policy_rx_header_.MessageType) {
        case CMTGotoMin:
          set_policy_state(port, PE_SNK_Transition_Sink);
          TimerStart(&port->policy_state_timer_, ktPSTransition);
          break;
        case CMTGetSinkCap:
          set_policy_state(port, PE_SNK_Give_Sink_Cap);
          break;
#ifdef FSC_HAVE_SRC
        case CMTGetSourceCap:
          set_policy_state(port, PE_DR_SNK_Give_Source_Cap);
          break;
#endif /* FSC_HAVE_SRC */
        case CMTDR_Swap:
          set_policy_state(port, PE_DRS_UFP_DFP_Evaluate_Swap);
          break;
#ifdef FSC_HAVE_SRC
        case CMTPR_Swap:
          set_policy_state(port, PE_PRS_SNK_SRC_Evaluate_Swap);
          break;
#ifdef FSC_HAVE_EXTENDED
        case CMTGetSourceCapExt:
          set_policy_state(port, PE_DR_SNK_Give_Source_Cap_Ext);
          break;
#endif /* FSC_HAVE_EXTENDED */
#endif /* FSC_HAVE_SRC */
       case CMTVCONN_Swap:
          set_policy_state(port, PE_VCS_Evaluate_Swap);
          break;
        case CMTSoftReset:
          set_policy_state(port, PE_SNK_Soft_Reset);
          break;
        case CMTPing:
            /* Ping ignored */
            break;
        case CMTReject:
        case CMTNotSupported:
            /* Rx'd Reject/NS are ignored - notify DPM if needed */
            break;
        case CMTAccept:
        case CMTWait:
          /* Unexpected messages */
          set_policy_state(port, PE_SNK_Send_Soft_Reset);
          break;
        default:
          /* Send a reject message for all other commands */
          set_policy_state(port, PE_SNK_Send_Not_Supported);
          break;
      }
      port->pd_tx_status_ = txIdle;
    }
    else if (port->policy_rx_header_.Extended == 1) {
      switch (port->policy_rx_header_.MessageType) {
#ifdef FSC_HAVE_EXTENDED
#if 0 /* Not supporting security req/response - just for testing */
      case EMTSecurityRequest:
        set_policy_state(port, PE_Send_Security_Response);
        break;
      case EMTSecurityResponse:
        set_policy_state(port, PE_Security_Response_Received);
        break;
#endif /* 0 */
      case EMTGetManufacturerInfo:
        set_policy_state(port, PE_Give_Manufacturer_Info);
        break;
      case EMTPPSStatus:
        break;
#endif /* FSC_HAVE_EXTENDED */
      default:
#ifndef FSC_HAVE_EXTENDED
        port->protocol_ext_header_.byte[0] =
            port->policy_rx_data_obj_[0].byte[0];
        port->protocol_ext_header_.byte[1] =
            port->policy_rx_data_obj_[0].byte[1];
        if (port->protocol_ext_header_.DataSize > MAX_EXT_MSG_LEGACY_LEN) {
          port->wait_for_not_supported_ = TRUE;
        } else {
          port->wait_for_not_supported_ = FALSE;
        }
#endif /* FSC_HAVE_EXTENDED */
        /* Send a reject message for all other commands */
        set_policy_state(port, PE_SNK_Send_Not_Supported);
        break;
      }
      port->pd_tx_status_ = txIdle;
    }
    else {
      switch (port->policy_rx_header_.MessageType) {
        case DMTSourceCapabilities:
          UpdateCapabilitiesRx(port, TRUE);
          set_policy_state(port, PE_SNK_Evaluate_Capability);
          break;
        case DMTSinkCapabilities:
          break;
#ifdef FSC_HAVE_VDM
        case DMTVendorDefined:
          ConvertAndProcessVdmMessage(port);
          break;
#endif /* FSC_HAVE_VDM */
        case DMTBIST:
          ProcessDmtBist(port);
          break;
        default:
          /* Send a reject message for all other commands */
          set_policy_state(port, PE_SNK_Send_Not_Supported);
          break;
      }
      port->pd_tx_status_ = txIdle;
    }
  }
  else if (port->pd_tx_flag_) {
    if (port->pd_transmit_header_.NumDataObjects == 0) {
      switch (port->pd_transmit_header_.MessageType) {
        case CMTGetSourceCap:
          set_policy_state(port, PE_SNK_Get_Source_Cap);
          break;
        case CMTGetSinkCap:
          set_policy_state(port, PE_DR_SNK_Get_Sink_Cap);
          break;
        case CMTDR_Swap:
          set_policy_state(port, PE_DRS_UFP_DFP_Send_Swap);
          break;
#ifdef FSC_HAVE_SRC
        case CMTPR_Swap:
          set_policy_state(port, PE_PRS_SNK_SRC_Send_Swap);
          break;
#endif /* FSC_HAVE_SRC */
        case CMTVCONN_Swap:
          set_policy_state(port, PE_VCS_Send_Swap);
          break;
        case CMTSoftReset:
          set_policy_state(port, PE_SNK_Send_Soft_Reset);
          break;
#ifdef FSC_HAVE_EXTENDED
        case CMTGetSourceCapExt:
          set_policy_state(port, PE_SNK_Get_Source_Cap_Ext);
          break;
#endif /* FSC_HAVE_EXTENDED */
        case CMTGetPPSStatus:
          set_policy_state(port, PE_SNK_Get_PPS_Status);
        default:
#ifdef FSC_DEBUG
          set_policy_state(port, PE_Send_Generic_Cmd);
#endif /* FSC_DEBUG */
          break;
      }
      port->pd_tx_status_ = txIdle;
    }
    else {
      switch (port->pd_transmit_header_.MessageType) {
        case DMTRequest:
          port->sink_request_.object = port->pd_transmit_objects_[0].object;
          set_policy_state(port, PE_SNK_Select_Capability);
          break;
        case DMTVendorDefined:
#ifdef FSC_HAVE_VDM
          DoVdmCommand(port);
#endif /* FSC_HAVE_VDM */
          break;
        default:
#ifdef FSC_DEBUG
            set_policy_state(port, PE_Send_Generic_Data);
#endif /* FSC_DEBUG */
          break;
      }
    }
    port->pd_tx_flag_ = FALSE;
  }
  else if (port->cbl_rst_state_ > CBL_RST_DISABLED)
  {
    ProcessCableResetState(port);
  }
  else if ((port->port_type_ == USBTypeC_DRP) &&
           (port->req_pr_swap_as_snk_ == TRUE) &&
           (port->partner_caps_.FPDOSupply.DualRolePower == TRUE))
  {
    set_policy_state(port, PE_PRS_SNK_SRC_Send_Swap);
  }
  else if (port->req_dr_swap_to_dfp_as_sink_ == TRUE &&
           port->policy_is_dfp_ == FALSE &&
           DR_Swap_To_DFP_Supported)
  {
    port->req_dr_swap_to_dfp_as_sink_ = FALSE;
    set_policy_state(port, PE_DRS_UFP_DFP_Send_Swap);
  }
  else if (port->req_vconn_swap_to_on_as_sink_ == TRUE &&
           GetVConn(port) == FALSE &&
           VCONN_Swap_To_On_Supported)
  {
    port->req_vconn_swap_to_on_as_sink_ = FALSE;
    set_policy_state(port, PE_VCS_Send_Swap);
  }
#ifdef FSC_HAVE_VDM
  else if (port->vdm_check_cbl_ && DPM_IsSOPPAllowed(port)) {
    RequestDiscoverIdentity(port, SOP_TYPE_SOP1);
    port->vdm_check_cbl_ = FALSE;
  }
  else if (port->policy_is_dfp_ == TRUE &&
      port->vdm_auto_state_ != AUTO_VDM_DONE) {
    AutoVdmDiscovery(port);
  }
#endif /* FSC_HAVE_VDM */
  else if (port->renegotiate_) {
    port->renegotiate_ = FALSE;
    set_policy_state(port, PE_SNK_Evaluate_Capability);
  }
  else if (port->source_is_apdo_ && TimerExpired(&port->pps_timer_)) {
    TimerDisable(&port->pps_timer_);
    port->sink_request_.object = port->stored_apdo_.object;
    set_policy_state(port, PE_SNK_Select_Capability);
  }
#ifdef FSC_HAVE_FRSWAP
  else if (port->is_fr_swap_) {
    set_policy_state(port, PE_FRS_SNK_SRC_Send_Swap);
  }
#endif /* FSC_HAVE_FRSWAP */
  else {
    port->idle_ = TRUE;
    TimerDisable(&port->policy_state_timer_);
    TimerDisable(&port->no_response_timer_);
  }
}

void PolicySinkGetSinkCap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTGetSinkCap, 0, 0, PE_DR_SNK_Get_Sink_Cap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        port->pd_tx_status_ = txIdle;
        if ((port->policy_rx_header_.NumDataObjects > 0) &&
            (port->policy_rx_header_.MessageType == DMTSinkCapabilities)) {
          /* Process new sink caps here if necessary */
          set_policy_state(port, PE_SNK_Ready);
        }
        else if ((port->policy_rx_header_.NumDataObjects == 0) &&
                 (port->policy_rx_header_.MessageType == CMTReject ||
                  port->policy_rx_header_.MessageType == CMTNotSupported)) {
          set_policy_state(port, PE_SNK_Ready);
        }
        else {
          set_policy_state(port, PE_SNK_Send_Soft_Reset);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SNK_Ready);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkGiveSinkCap(struct Port *port)
{
  PolicySend(port, DMTSinkCapabilities,
             port->caps_header_sink_.NumDataObjects * 4,
             (FSC_U8 *)port->caps_sink_, PE_SNK_Ready, 0, SOP_TYPE_SOP, FALSE);
}

void PolicySinkGetSourceCap(struct Port *port)
{
    switch (port->policy_subindex_) {
      case 0:
        if (PolicySend(port, CMTGetSourceCap, 0, 0, PE_SNK_Get_Source_Cap,
                       1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
          TimerStart(&port->policy_state_timer_, ktSenderResponse);
        }
        break;
      case 1:
        if (port->protocol_msg_rx_) {
          port->protocol_msg_rx_ = FALSE;
          port->pd_tx_status_ = txIdle;
          if ((port->policy_rx_header_.NumDataObjects > 0) &&
              (port->policy_rx_header_.MessageType == DMTSourceCapabilities)) {
            UpdateCapabilitiesRx(port, FALSE);
            TimerDisable(&port->policy_state_timer_);
            port->partner_caps_available_ = TRUE;
            set_policy_state(port, PE_SNK_Evaluate_Capability);
          }
          else {
            /* No valid source caps message */
              set_policy_state(port, PE_SNK_Send_Soft_Reset);
          }
        }
        else if (TimerExpired(&port->policy_state_timer_)) {
          TimerDisable(&port->policy_state_timer_);
          set_policy_state(port, PE_SNK_Ready);
        }
        else {
          port->idle_ = TRUE;
        }
        break;
      default:
        set_policy_state(port, PE_ErrorRecovery);
        break;
    }
}

void PolicySinkGiveSourceCap(struct Port *port)
{
#ifdef FSC_HAVE_DRP
  if (port->port_type_ == USBTypeC_DRP)
    PolicySend(port, DMTSourceCapabilities,
               port->caps_header_source_.NumDataObjects * 4,
               (FSC_U8 *)port->caps_source_,
               PE_SNK_Ready, 0, SOP_TYPE_SOP, FALSE);
  else
#endif /* FSC_HAVE_DRP */
    PolicySendNotSupported(port);
}

void PolicySinkSendDRSwap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTDR_Swap, 0, 0, PE_DRS_UFP_DFP_Send_Swap,
                          1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->policy_is_dfp_ =
                  (port->policy_is_dfp_ == TRUE) ? FALSE : TRUE;
              port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
              WriteRegister(port, regMSGHEADR);
              DPM_ReConfigureRxDetect(port);
              notify_observers(port->policy_is_dfp_ == TRUE
                               ? EVENT_DATA_ROLE_DFP : EVENT_DATA_ROLE_UFP,
                               port->port_id_, 0);
              set_policy_state(port, PE_SNK_Ready);
              break;
            case CMTSoftReset:
              set_policy_state(port, PE_SNK_Soft_Reset);
              break;
            default:
              set_policy_state(port, PE_SNK_Ready);
              break;
          }
        }
        else {
          set_policy_state(port, PE_SNK_Ready);
        }
        port->pd_tx_status_ = txIdle;
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SNK_Ready);
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkEvaluateDRSwap(struct Port *port)
{
#ifdef FSC_HAVE_VDM
  if (port->mode_entered_ == TRUE) {
    PolicySinkSendHardReset(port);
    return;
  }
#endif /* FSC_HAVE_VDM */
  if ((port->protocol_msg_rx_sop_ != SOP_TYPE_SOP) ||
      (port->policy_is_dfp_ && !DR_Swap_To_UFP_Supported) ||
      (!port->policy_is_dfp_ && !DR_Swap_To_DFP_Supported)) {
    PolicySendNotSupported(port);
  }
  else
  {
    if (PolicySend(port, CMTAccept, 0, 0, PE_SNK_Ready,
            0, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
      port->policy_is_dfp_ = (port->policy_is_dfp_ == TRUE) ? FALSE : TRUE;
      port->registers_.MsgHeadr.DATA_ROLE = port->policy_is_dfp_;
      WriteRegister(port, regMSGHEADR);
      DPM_ReConfigureRxDetect(port);
      notify_observers(port->policy_is_dfp_ == TRUE
                       ? EVENT_DATA_ROLE_DFP : EVENT_DATA_ROLE_UFP,
                       port->port_id_, 0);
    }
  }
}

void PolicySinkSendVCONNSwap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTVCONN_Swap, 0, 0, PE_VCS_Send_Swap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->policy_subindex_++;
              TimerDisable(&port->policy_state_timer_);
              break;
            case CMTReject:
            case CMTNotSupported:
              /* If needed, can force becoming the VConn Source */
              if (port->is_vconn_source_ == FALSE) {
                port->is_vconn_source_ = TRUE;
                SetVConn(port, TRUE);
              }
              /* Fall through */
            case CMTWait:
              set_policy_state(port, PE_SNK_Ready);
              port->pd_tx_status_ = txIdle;
              break;
            default:
              /* Ignore all other commands */
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SNK_Ready);
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (port->is_vconn_source_) {
        TimerStart(&port->policy_state_timer_, ktVCONNSourceOn);
        port->policy_subindex_++;
      }
      else {
        /* Apply VConn */
        port->is_vconn_source_ = TRUE;
        SetVConn(port, TRUE);

        /* Skip next state and send the PS_RDY msg */
        port->policy_subindex_ = 4;
      }
      break;
    case 3:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              /* Disable VCONN source */
              port->is_vconn_source_ = FALSE;
              SetVConn(port, FALSE);

              set_policy_state(port, PE_SNK_Ready);
              port->pd_tx_status_ = txIdle;
              break;
            default:
              /* Ignore all other commands received */
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySinkSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 4:
      PolicySend(port, CMTPS_RDY, 0, 0, PE_SNK_Ready, 0, SOP_TYPE_SOP, FALSE);
#ifdef FSC_HAVE_VDM
      port->vdm_check_cbl_ = (port->vdm_cbl_present_ == FALSE &&
                              DPM_IsSOPPAllowed(port)) ? TRUE : FALSE;
#endif /* FSC_HAVE_VDM */
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkEvaluateVCONNSwap(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if ((port->protocol_msg_rx_sop_ != SOP_TYPE_SOP) ||
          (port->is_vconn_source_ && !VCONN_Swap_To_Off_Supported) ||
          (!port->is_vconn_source_ && !VCONN_Swap_To_On_Supported)) {
        PolicySendNotSupported(port);
      }
      else if (PolicySend(port, CMTAccept, 0, 0, PE_VCS_Evaluate_Swap,
                          1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        if (port->is_vconn_source_) {
          TimerStart(&port->policy_state_timer_, ktVCONNSourceOn);
        }
        else {
          /* Apply VConn */
          port->is_vconn_source_ = TRUE;
          SetVConn(port, TRUE);

          TimerStart(&port->policy_state_timer_, port->vbus_transition_time_);
          port->policy_subindex_ = 2;
        }
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
       port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              /* Disable VConn source */
              port->is_vconn_source_ = FALSE;
              SetVConn(port, FALSE);

              set_policy_state(port, PE_SNK_Ready);
              port->pd_tx_status_ = txIdle;
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        PolicySinkSendHardReset(port);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->policy_subindex_++;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 3:
      PolicySend(port, CMTPS_RDY, 0, 0, PE_SNK_Ready, 0, SOP_TYPE_SOP, FALSE);
#ifdef FSC_HAVE_VDM
      port->vdm_check_cbl_ = (port->vdm_cbl_present_ == FALSE &&
                              DPM_IsSOPPAllowed(port)) ? TRUE : FALSE;
#endif /* FSC_HAVE_VDM */
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicySinkSendPRSwap(struct Port *port)
{
#ifdef FSC_HAVE_DRP
  FSC_U8 status;
  port->req_pr_swap_as_snk_  = FALSE;
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTPR_Swap, 0, 0, PE_PRS_SNK_SRC_Send_Swap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      /* Require Accept message to move on or go back to ready state */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->is_pr_swap_ = TRUE;
              port->policy_has_contract_ = FALSE;

              port->registers_.PwrCtrl.AUTO_DISCH = 0;
              WriteRegister(port, regPWRCTRL);

              SendCommand(port, DisableSinkVbus);

              TimerStart(&port->policy_state_timer_, ktPSSourceOff);
              port->policy_subindex_++;
              break;
            case CMTWait:
            case CMTReject:
            case CMTNotSupported:
              set_policy_state(port, PE_SNK_Ready);
              port->is_pr_swap_ = FALSE;
              port->pd_tx_status_ = txIdle;
              break;
            default:
              /* Interrupted */
              set_policy_state(port, PE_SNK_Send_Soft_Reset);
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_SNK_Ready);
        port->is_pr_swap_ = FALSE;
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      /* Wait for a PS_RDY message to be received to indicate that the */
      /* original source is no longer supplying VBUS */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              port->policy_is_source_ = TRUE;

              RoleSwapToAttachedSource(port);

              port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
              WriteRegister(port, regMSGHEADR);
              TimerDisable(&port->policy_state_timer_);
              port->policy_subindex_++;
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_pr_swap_ = FALSE;

        /* Note: Compliance testing seems to require BOTH HR and ER here. */
        PolicySinkSendHardReset(port);
        set_policy_state(port, PE_ErrorRecovery);

        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 3:
      if (IsVbusVSafe5V(port)) {
        status = PolicySend(port, CMTPS_RDY, 0, 0, PE_SRC_Startup,
                            0, SOP_TYPE_SOP, FALSE);
        if (status == STAT_ERROR) {
          set_policy_state(port, PE_ErrorRecovery);
        }
        else if (status == STAT_SUCCESS){
          port->registers_.PwrCtrl.AUTO_DISCH = 1;
          WriteRegister(port, regPWRCTRL);

          TimerStart(&port->swap_source_start_timer_, ktSwapSourceStart);
        }
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
#endif /* FSC_HAVE_DRP */
}

void PolicySinkEvaluatePRSwap(struct Port *port)
{
#ifdef FSC_HAVE_DRP
  FSC_U8 status;
  switch (port->policy_subindex_) {
    case 0:
      if (port->protocol_msg_rx_sop_ != SOP_TYPE_SOP ||
          Accepts_PR_Swap_As_Snk == FALSE) {
        PolicySendNotSupported(port);
      }
      else if (port->partner_caps_.FPDOSupply.SupplyType == pdoTypeFixed &&
               port->partner_caps_.FPDOSupply.DualRolePower == FALSE) {
        PolicySend(port, CMTReject, 0, 0, PE_SNK_Ready, 0,
                   port->protocol_msg_rx_sop_, FALSE);
      }
      else if (PolicySend(port, CMTAccept, 0, 0, PE_PRS_SNK_SRC_Evaluate_Swap,
                  1, port->protocol_msg_rx_sop_, FALSE) == STAT_SUCCESS) {
        port->is_pr_swap_ = TRUE;
        port->policy_has_contract_ = FALSE;

        port->registers_.PwrCtrl.AUTO_DISCH = 0;
        WriteRegister(port, regPWRCTRL);

        SendCommand(port, DisableSinkVbus);

        TimerStart(&port->policy_state_timer_, ktPSSourceOff);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              TimerDisable(&port->policy_state_timer_);
              port->policy_is_source_ = TRUE;

              RoleSwapToAttachedSource(port);

              port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
              WriteRegister(port, regMSGHEADR);
              TimerStart(&port->policy_state_timer_, 0);
              port->policy_subindex_++;
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_pr_swap_ = FALSE;

        /* Note: Compliance testing seems to require BOTH HR and ER here. */
        PolicySinkSendHardReset(port);
        set_policy_state(port, PE_ErrorRecovery);

        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (IsVbusVSafe5V(port)) {
        port->req_pr_swap_as_snk_  = FALSE;
        status = PolicySend(port, CMTPS_RDY, 0, 0, PE_SRC_Startup, 0,
                            port->protocol_msg_rx_sop_, FALSE);
        if (status == STAT_ERROR) {
          set_policy_state(port, PE_ErrorRecovery);
        }
        else if (status == STAT_SUCCESS) {
          port->is_pr_swap_ = FALSE;

          port->registers_.PwrCtrl.AUTO_DISCH = 1;
          WriteRegister(port, regPWRCTRL);

          TimerStart(&port->swap_source_start_timer_, ktSwapSourceStart);
          port->idle_ = TRUE;
        } else
        {

        }
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
#else
  PolicySendNotSupported(port);
#endif /* FSC_HAVE_DRP */
}

#if defined(FSC_HAVE_EXTENDED) && defined(FSC_HAVE_SRC)
void PolicySinkGiveSourceCapExtended(struct Port *port)
{
  PolicySend(port, EMTSourceCapsExtended, 24, port->policy_src_cap_ext_.byte,
             PE_SNK_Ready, 0, SOP_TYPE_SOP, TRUE);
}
#endif /* FSC_HAVE_EXTENDED  && FSC_HAVE_DRP */


#ifdef FSC_HAVE_FRSWAP
void PolicySinkSendFRSwap(struct Port *port)
{
  FSC_U8 status;
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTFR_Swap, 0, 0, PE_FRS_SNK_SRC_Send_Swap,
                     1, SOP_TYPE_SOP, FALSE) == STAT_SUCCESS) {
        /* Not an official time - just an error time out option */
        TimerStart(&port->policy_state_timer_, ktSrcRecover);
      }
      break;
    case 1:
      /* Require Accept message to move on or go back to ready state */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTAccept:
              port->is_fr_swap_ = TRUE;
              port->policy_has_contract_ = FALSE;

              port->registers_.PwrCtrl.AUTO_DISCH = 0;
              WriteRegister(port, regPWRCTRL);
              port->policy_subindex_++;
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_ErrorRecovery);
        port->is_fr_swap_ = FALSE;
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      /* Wait for a PS_RDY message to be received to indicate that the */
      /* original source is no longer supplying VBUS */
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        if (port->policy_rx_header_.NumDataObjects == 0) {
          switch (port->policy_rx_header_.MessageType) {
            case CMTPS_RDY:
              port->policy_is_source_ = TRUE;

              RoleSwapToAttachedSource(port);

              port->registers_.MsgHeadr.POWER_ROLE = port->policy_is_source_;
              WriteRegister(port, regMSGHEADR);
              port->policy_subindex_++;
              break;
            default:
              break;
          }
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->is_fr_swap_ = FALSE;
        set_policy_state(port, PE_ErrorRecovery);
        port->pd_tx_status_ = txIdle;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 3:
      status = PolicySend(port, CMTPS_RDY, 0, 0, PE_SRC_Startup,
                            0, SOP_TYPE_SOP, FALSE);
      if (status == STAT_ERROR) {
        set_policy_state(port, PE_ErrorRecovery);
      }
      else if (status == STAT_SUCCESS){
        TimerDisable(&port->policy_state_timer_);

        port->registers_.PwrCtrl.AUTO_DISCH = 1;
        WriteRegister(port, regPWRCTRL);
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}
#endif /* FSC_HAVE_FRSWAP */
#endif /* FSC_HAVE_SNK */

void PolicySendNotSupported(struct Port *port)
{
  set_policy_state(port, port->policy_is_source_ ?
          PE_SRC_Send_Not_Supported : PE_SNK_Send_Not_Supported);

  if (((port->protocol_msg_rx_sop_ == SOP_TYPE_SOP) && !port->dpm_pd_30_) ||
      ((port->protocol_msg_rx_sop_ == SOP_TYPE_SOP1) &&
       !port->dpm_pd_30_srccab_)) {
    PolicySend(port, CMTReject, 0, 0,
               port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready, 0,
               port->protocol_msg_rx_sop_, FALSE);
  }
  else {
    switch(port->policy_subindex_) {
#ifndef FSC_HAVE_EXTENDED
    /* When chunking is not supported wait for timer tChunkingNotSupported
     * timer timeout */
    case 0:
      if (port->wait_for_not_supported_) {
        TimerStart(&port->policy_state_timer_, ktChunkingNotSupported);
        port->policy_subindex_++;
        port->wait_for_not_supported_ = FALSE;
      }
      else {
        port->policy_subindex_ = 2;
      }
      break;
    case 1:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        port->policy_subindex_++;
        /* Fall through to default immediately */
      } else {
        break;
      }
    case 2:
#endif /* FSC_HAVE_EXTENDED */
    default:
      PolicySend(port, CMTNotSupported, 0, 0,
                 port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready, 0,
                 port->protocol_msg_rx_sop_, FALSE);
      break;
    }
  }
}

void PolicyNotSupported(struct Port *port)
{
  /* TODO Inform DPM if needed. */
  set_policy_state(port, port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready);
}

void PolicyDFPCBLSendSoftReset(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      if (PolicySend(port, CMTSoftReset, 0, 0, PE_DFP_CBL_Send_Soft_Reset,
                     1, SOP_TYPE_SOP1, FALSE) == STAT_SUCCESS) {
        TimerStart(&port->policy_state_timer_, ktSenderResponse);
      }
      break;
    case 1:
      if (port->protocol_msg_rx_) {
        port->protocol_msg_rx_ = FALSE;
        port->pd_tx_status_ = txIdle;
        if ((port->policy_rx_header_.NumDataObjects == 0) &&
            (port->policy_rx_header_.MessageType == CMTAccept)) {
          set_policy_state(port, port->policy_is_source_ ?
              PE_SRC_Ready : PE_SNK_Ready);
          ResetProtocolLayer(port, SOP_TYPE_SOP1);
        }
        else {
          /* Unexpected message */
          set_policy_state(port, PE_DFP_CBL_Send_Cable_Reset);
        }
      }
      else if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        set_policy_state(port, PE_DFP_CBL_Send_Cable_Reset);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      break;
  }
}

void PolicyDFPCBLSendReset(struct Port *port)
{
  port->cbl_rst_state_ = CBL_RST_DISABLED;
  PolicySendHardReset(port,
      port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready, TRUE);
}

#ifdef FSC_HAVE_EXTENDED
void PolicyGetSecurityMsg(struct Port *port)
{
  FSC_U16 i;

  /* Send a request to the partner for a Security Response msg */
  for (i = 0; i < MAX_EXT_MSG_LEN; i++) {
    port->protocol_ext_buffer_[i] = 0;
  }

  PolicySend(port, EMTSecurityRequest, 0, 0,
             port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready,
             0, SOP_TYPE_SOP, TRUE);
}

void PolicySendSecurityMsg(struct Port *port)
{
  FSC_U16 i;

  /* Send a Security Response msg to the partner */
  for (i = 0; i < MAX_EXT_MSG_LEN; i++) {
    /* Not a security message - just an array for testing chunking */
    port->protocol_ext_buffer_[i] = (i < 50) ? i : 0;
  }

  /* Note: protocol_ext_buffer_ gets copied into itself in this call,
   * but this way we don't have to define an extra buffer for testing...
   */
  PolicySend(port, EMTSecurityResponse, 50, port->protocol_ext_buffer_,
             port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready,
             0, SOP_TYPE_SOP, TRUE);
}

void PolicySecurityMsgReceived(struct Port *port)
{
  /* Nothing happening for now */
  set_policy_state(port, port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready);
}

void PolicyGetPPSStatus(struct Port *port)
{
  switch (port->policy_subindex_)
  {
  case 0:
    if (PolicySend(port, CMTGetPPSStatus, 0, 0, PE_SNK_Get_PPS_Status, 1,
                   SOP_TYPE_SOP, FALSE) == STAT_SUCCESS)
    {
      TimerStart(&port->policy_state_timer_, ktSenderResponse);
    }
    break;
  default:
    if (port->protocol_msg_rx_) {
      port->protocol_msg_rx_ = FALSE;
      port->pd_tx_status_ = txIdle;
      if ((port->policy_rx_header_.Extended == 1) &&
          (port->policy_rx_header_.MessageType ==  EMTPPSStatus))
      {
        set_policy_state(port,
                         port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready);
        notify_observers(EVENT_PPS_STATUS_RECIEVED, port->port_id_,
                         port->protocol_ext_buffer_);
      }
      else
      {
        set_policy_state(port, port->policy_is_source_ ?
                PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset);
      }
    }
    else if (TimerExpired(&port->policy_state_timer_))
    {
      set_policy_state(port,
                       port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready);
    }
    else
    {
      port->idle_ = TRUE;
    }
    break;
  }
}

void PolicyGivePPSStatus(struct Port *port)
{
  PPSStatus_t ppsstatus;

  switch (port->policy_subindex_)
  {
  case 0:
    /* Only need to get the values once */
    ppsstatus.OutputVoltage = platform_getPPSVoltage(port->port_id_) / 20;
    ppsstatus.OutputCurrent = 0xFF; /* Not supported field for now */
    ppsstatus.byte[3] = 0x00;

    port->policy_subindex_++;
    /* Fall through */
  case 1:
    PolicySend(port, EMTPPSStatus, 4, ppsstatus.byte,
               port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready, 0,
               SOP_TYPE_SOP, TRUE);
    break;
  default:
    break;
  }
}

void PolicySinkGetSourceCapExt(struct Port *port)
{
  FSC_U8 status = STAT_BUSY;
  switch(port->policy_subindex_)
  {
  case 0:
    status = PolicySend(port, CMTGetSourceCapExt, 0, 0,
                         PE_SNK_Get_Source_Cap_Ext, 1, SOP_TYPE_SOP,
                         FALSE);
    if (status == STAT_SUCCESS)
    {
      TimerStart(&port->policy_state_timer_, ktSenderResponse);
    }
    else if (status == STAT_ERROR)
    {
      /* Error received at sending. May be protocol did not receive
       *  goodcrc or timed out.  HW issue prevents GSCE from correctly
       *  sending retries and requires a PDReset to recover. */
      PortPDReset(port);
      /* Workaround since there is no I_TXFAIL interrupt for GSCE */
      port->message_id_counter_[SOP_TYPE_SOP] =
              (port->message_id_counter_[SOP_TYPE_SOP] + 1) & 0x07;
      set_policy_state(port, PE_SNK_Ready);
    }
    break;
  case 1:
    if (port->protocol_msg_rx_ == TRUE)
    {
      TimerDisable(&port->policy_state_timer_);
      port->protocol_msg_rx_ = FALSE;
      if (port->policy_rx_header_.Extended == 1 &&
          port->policy_rx_header_.NumDataObjects > 0 &&
          port->policy_rx_header_.MessageType == EMTSourceCapsExtended)
      {
        /* Unchunked Extended message Source Cap Extended received.
         *  Process it or inform DPM here. */
      }
      PortPDReset(port);
      set_policy_state(port, PE_SNK_Ready);
    }
    else if (TimerExpired(&port->policy_state_timer_))
    {
      TimerDisable(&port->policy_state_timer_);
      PortPDReset(port);
      set_policy_state(port, PE_SNK_Ready);
    }
    break;
  }
}

void PolicyGiveManufacturerInfo(struct Port *port)
{
  /* Allocate at least 18 bytes */
  FSC_U8 buf[] =
      {0,0,0,0,'N','o','t',' ','S','u','p','p','o','r','t','e','d',0};
  FSC_U8 len;

  if (Manufacturer_Info_Supported_Port) {
    buf[0] = Manufacturer_Info_VID_Port & 0x00FF; /* VID */
    buf[1] = (Manufacturer_Info_VID_Port & 0xFF00) >> 8;
    buf[2] = Manufacturer_Info_PID_Port & 0x00FF; /* PID */
    buf[3] = (Manufacturer_Info_PID_Port & 0xFF00) >> 8;

    /* Manufacturer string */
    if (port->protocol_ext_buffer_[0] > 1 ||
        port->protocol_ext_buffer_[1] > 7) {
      /* Unknown/reserved target/ref request */
      len = sizeof(buf);
    } else {
      /* Valid target, no info string for now */
      len = 5;
    }

    PolicySend(port, EMTManufacturerInfo, len, buf,
        (port->policy_is_source_ == TRUE) ? PE_SRC_Ready : PE_SNK_Ready, 0,
        SOP_TYPE_SOP, TRUE);
  }
  else
  {
    PolicySendNotSupported(port);
  }
}

#endif /* FSC_HAVE_EXTENDED */

void UpdateCapabilitiesRx(struct Port *port, FSC_BOOL is_source_cap_update)
{
  FSC_U8 i = 0;

#ifdef FSC_HAVE_USBHID
  /* Set the source caps updated flag to trigger an update of the GUI */
  port->source_caps_updated_ = is_source_cap_update;
#endif /* FSC_HAVE_USBHID */

  port->caps_header_received_.word = port->policy_rx_header_.word;
  for (i = 0; i < port->caps_header_received_.NumDataObjects; i++) {
    port->caps_received_[i].object =
        port->policy_rx_data_obj_[i].object;
  }

  for (i = port->caps_header_received_.NumDataObjects; i < 7; i++) {
    port->caps_received_[i].object = 0;
  }
  port->partner_caps_.object = port->caps_received_[0].object;
  notify_observers(EVENT_SRC_CAPS_UPDATED, port->port_id_, 0);
}

FSC_BOOL PolicySendHardReset(struct Port *port, PolicyState_t next_state,
                             FSC_BOOL cable)
{
  FSC_BOOL Success = FALSE;

  switch (port->pd_tx_status_) {
    case txReset:
    case txWait:
      /* Do nothing until the protocol layer finishes generating the hard */
      /* reset setting the next state as either txCollision or txSuccess */
      break;
    case txSuccess:
      TimerStart(&port->policy_state_timer_,
                 ktPSHardReset - ktPSHardResetOverhead);
      port->idle_ = TRUE;
      port->hard_reset_counter_++;
      set_policy_state(port, next_state);
      port->pd_tx_status_ = txIdle;
      TimerDisable(&port->no_response_timer_);
      Success = TRUE;
      if (cable) {
        /* Reset the cable protocol variables here before returning to SRC or
         * SNK Ready */
        ResetProtocolLayer(port, SOP_TYPE_SOP1);
      }
      break;
    case txIdle:
    default:
      ProtocolSendHardReset(port, cable);
      break;
  }
  return Success;
}

void PolicySendGenericCommand(struct Port *port)
{
    FSC_U8 status;
    switch (port->policy_subindex_)
    {
    case 0:
        port->policy_is_ams_ = FALSE;
        status = PolicySend(port, port->pd_transmit_header_.MessageType, 0, 0,
                   PE_Send_Generic_Cmd, 1, port->policy_msg_tx_sop_, FALSE);

        if (status == STAT_SUCCESS)
        {
            TimerStart(&port->policy_state_timer_, ktSenderResponse);
        }
        else if (status == STAT_ERROR)
        {
            set_policy_state(port, port->policy_is_source_ ?
                    PE_SRC_Ready : PE_SNK_Ready);
        }
        break;
    default:
        if (port->protocol_msg_rx_)
        {
            port->protocol_msg_rx_ = FALSE;

            /* Check and handle message response */

            set_policy_state(port, port->policy_is_source_ ?
                    PE_SRC_Ready : PE_SNK_Ready);
        }
        else if (TimerExpired(&port->policy_state_timer_))
        {
            /* If no response, or no response expected, state will time out
             * after tSenderResponse.
             */
            set_policy_state(port, port->policy_is_source_ ?
                    PE_SRC_Ready : PE_SNK_Ready);
        }
        else
        {
            port->idle_ = TRUE;
        }
        break;
    }
}

void PolicySendGenericData(struct Port *port)
{
    FSC_U8 status;
    switch (port->policy_subindex_)
    {
    case 0:
        port->policy_is_ams_ = FALSE;
        status = PolicySend(port, port->pd_transmit_header_.MessageType,
                   port->pd_transmit_header_.NumDataObjects * 4,
                   (FSC_U8*)port->pd_transmit_objects_, PE_Send_Generic_Data,
                   1, port->policy_msg_tx_sop_, FALSE);

        if (status == STAT_SUCCESS)
        {
            TimerStart(&port->policy_state_timer_, ktSenderResponse);
        }
        else if (status == STAT_ERROR)
        {
            set_policy_state(port, port->policy_is_source_ ?
                    PE_SRC_Ready : PE_SNK_Ready);
        }
        break;
    default:
        if (port->protocol_msg_rx_)
        {
            port->protocol_msg_rx_ = FALSE;

            /* Check and handle message response */

            set_policy_state(port, port->policy_is_source_ ?
                    PE_SRC_Ready : PE_SNK_Ready);
        }
        else if (TimerExpired(&port->policy_state_timer_))
        {
            /* If no response, or no response expected, state will time out
             * after tSenderResponse.
             */
            set_policy_state(port, port->policy_is_source_ ?
                    PE_SRC_Ready : PE_SNK_Ready);
        }
        else
        {
            port->idle_ = TRUE;
        }
        break;
    }
}

FSC_U8 PolicySend(struct Port *port, FSC_U8 message_type,
                  FSC_U16 num_bytes, FSC_U8 *data,
                  PolicyState_t next_state,
                  FSC_U8 subindex, SopType sop, FSC_BOOL extended)
{
  FSC_U16 i = 0;
  FSC_U8 status = STAT_BUSY;

  switch (port->pd_tx_status_) {
    case txIdle:
      if (port->policy_is_source_ && port->dpm_pd_30_ && port->policy_is_ams_ &&
          (port->policy_sinktx_state_ == SinkTxOK)) {
        /* If PD 3.0, set CC SinkTxNG and wait out SinkTx timer */
        SetSinkTx(port, SinkTxNG);
        TimerStart(&port->policy_sinktx_timer_, ktSinkTx);
        port->pd_tx_status_ = txPending;
        break;
      }
      else {
        TimerDisable(&port->policy_sinktx_timer_);
      }
      /* Else fall through */
    case txPending:
      if (!TimerExpired(&port->policy_sinktx_timer_) &&
          !TimerDisabled(&port->policy_sinktx_timer_)) {
        /* If we need to wait on the sinkTx timer */
        port->idle_ = TRUE;
        break;
      }

      /* Continue on with transmission */
      TimerDisable(&port->policy_sinktx_timer_);

      port->policy_tx_header_.word = 0x0000;
      port->policy_tx_header_.MessageType = message_type & PDMsgTypeMask;
      if (sop == SOP_TYPE_SOP)
      {
        port->policy_tx_header_.PortDataRole = port->policy_is_dfp_;
        port->policy_tx_header_.PortPowerRole = port->policy_is_source_;
      }

      port->policy_tx_header_.SpecRevision = DPM_CurrentSpecRev(port, sop);
      port->protocol_retries_ =
          (port->policy_tx_header_.SpecRevision == PDSpecRev3p0) ?
                  RETRIES_PD30 : RETRIES_PD20;
#ifdef FSC_HAVE_EXTENDED
      if (extended) {
        /* Extended Data Message */
        port->protocol_ext_num_bytes_ =
          (num_bytes > MAX_EXT_MSG_LEN) ? MAX_EXT_MSG_LEN : num_bytes;

        for (i = 0; i < port->protocol_ext_num_bytes_; i++) {
          port->protocol_ext_buffer_[i] = data[i];
        }

        port->policy_tx_header_.Extended = TRUE;
        port->protocol_ext_chunk_number_ = 0;
        port->protocol_ext_send_chunk_ = TRUE;
        port->protocol_ext_state_active_ = TRUE;
      }
      else
#endif /* FSC_HAVE_EXTENDED */
        if (num_bytes > 0) {
        /* Standard Data Message */
        if (num_bytes > MAX_MSG_LEGACY_LEN) {
          num_bytes = MAX_MSG_LEGACY_LEN;
        }

        port->policy_tx_header_.NumDataObjects = num_bytes / 4;
        for (i = 0; i < port->policy_tx_header_.NumDataObjects; i++) {
          port->policy_tx_data_obj_[i].byte[0] = data[i * 4 + 0];
          port->policy_tx_data_obj_[i].byte[1] = data[i * 4 + 1];
          port->policy_tx_data_obj_[i].byte[2] = data[i * 4 + 2];
          port->policy_tx_data_obj_[i].byte[3] = data[i * 4 + 3];
        }

        if (port->policy_state_ == PE_SRC_Send_Capabilities) {
          port->caps_counter_++;
        }
      }
      else {
        /* Control Message */
      }

      port->protocol_msg_tx_sop_ = sop;
      port->pd_tx_status_ = txSend;
      if (port->protocol_state_ == PRLIdle)
      {
        ProtocolIdle(port);
      }
      break;
    case txSend:
    case txBusy:
    case txWait:
      /* Waiting for GoodCRC or timeout of the protocol */
      if (TimerExpired(&port->protocol_timer_)){
        TimerDisable(&port->protocol_timer_);
        port->protocol_state_ = PRLIdle;
        port->pd_tx_status_ = txIdle;
        set_policy_state(port, next_state);
        port->policy_subindex_ = subindex;
        status = STAT_ERROR;
      }
      break;
    case txCollision:
      status = STAT_ERROR;
      break;
    case txSuccess:
#ifdef FSC_HAVE_EXTENDED
    if (extended && port->protocol_ext_num_bytes_ >
        port->protocol_ext_chunk_number_ * MAX_EXT_MSG_LEGACY_LEN) {
      port->pd_tx_status_ = txBusy;
      /* keep alive to send */
      port->idle_ = FALSE;
      break;
    } else {
      /* Completed transferring extended message */
      port->protocol_ext_state_active_ = FALSE;
    }
#endif /* FSC_HAVE_EXTENDED */
      set_policy_state(port, next_state);
      port->policy_subindex_ = subindex;
      port->pd_tx_status_ = txIdle;
      status = STAT_SUCCESS;
      break;
    case txError: /* No good CRC */
      if (sop == SOP_TYPE_SOP) {
        if ((port->policy_state_ == PE_SRC_Send_Capabilities) &&
            !port->policy_has_contract_) {
          set_policy_state(port, PE_SRC_Discovery);
        }
        else if (port->policy_state_ == PE_SRC_Send_Soft_Reset) {
  #ifdef FSC_HAVE_SRC
          PolicySourceSendHardReset(port);
  #endif /* FSC_HAVE_SRC */
        }
        else if (port->policy_state_ == PE_SNK_Send_Soft_Reset) {
  #ifdef FSC_HAVE_SNK
          PolicySinkSendHardReset(port);
  #endif /* FSC_HAVE_SRC */
        }
        else if (port->policy_is_source_) {
          set_policy_state(port, PE_SRC_Send_Soft_Reset);
        }
        else {
          set_policy_state(port, PE_SNK_Send_Soft_Reset);
        }
      }
      else if (sop == SOP_TYPE_SOP1) {
#ifdef FSC_HAVE_VDM
        if (port->vdm_cbl_present_ == FALSE) {
          /* Skip cable resets for initial discover ID */
        }
        else
#endif /*FSC_HAVE_VDM  */
        if (port->policy_state_ == PE_DFP_CBL_Send_Soft_Reset) {
          set_policy_state(port, PE_DFP_CBL_Send_Cable_Reset);
        }
        else {
          set_policy_state(port, PE_DFP_CBL_Send_Soft_Reset);
        }
        /* TODO - UFP CBL Resets? */
      }
      port->pd_tx_status_ = txIdle;
      status = STAT_ERROR;
      break;
    case txAbort:
      set_policy_state(port,
                       port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready);

      status = STAT_ABORT;
      port->pd_tx_status_ = txIdle;
      break;
    default:
      /* Reset everything */
      port->pd_tx_status_ = txReset;
      status = STAT_ERROR;

      if (port->policy_is_source_) {
#ifdef FSC_HAVE_SRC
        PolicySourceSendHardReset(port);
#endif /* FSC_HAVE_SRC */
      }
      else {
#ifdef FSC_HAVE_SNK
        PolicySinkSendHardReset(port);
#endif /* FSC_HAVE_SNK */
      }
      break;
  }

  if (status == STAT_ERROR) {
#ifdef FSC_HAVE_EXTENDED
    port->protocol_ext_state_active_ = FALSE;
#endif /* FSC_HAVE_EXTENDED */
  }
  return status;
}

/* ---------------- BIST Functionality ---------------- */
void ProcessDmtBist(struct Port *port)
{
  FSC_U8 bdo = port->policy_rx_data_obj_[0].byte[3] >> 4;
  switch (bdo) {
    case BDO_BIST_Carrier_Mode_2:
      if (port->caps_source_[port->usb_pd_contract_.FVRDO.ObjectPosition - 1]
          .FPDOSupply.Voltage == PD_05_V) {
        set_policy_state(port, PE_BIST_Carrier_Mode);
        port->protocol_state_ = PRLIdle;
        notify_observers(EVENT_BIST_ENABLED, port->port_id_, 0);
      }
      break;
    case BDO_BIST_Test_Data: /* Fall through */
    default:
      /* Mask everything but HARDRST and VBUSOK */
      if (port->caps_source_[port->usb_pd_contract_.FVRDO.ObjectPosition - 1]
          .FPDOSupply.Voltage == PD_05_V) {
        port->registers_.TcpcCtrl.BIST_TMODE = 1;
        WriteRegister(port, regTCPC_CTRL);

        set_policy_state(port, PE_BIST_Test_Data);
        port->protocol_state_ = PRLDisabled;
        notify_observers(EVENT_BIST_ENABLED, port->port_id_, 0);
      }
      break;
  }
}

void PolicyBISTCarrierMode2(struct Port *port)
{
  switch (port->policy_subindex_) {
    case 0:
      port->registers_.Transmit.TX_SOP = TRANSMIT_BIST_CM2;
      WriteRegister(port, regTRANSMIT);
      TimerStart(&port->policy_state_timer_, ktBISTContMode);
      port->policy_subindex_++;
      break;
    case 1:
      if (TimerExpired(&port->policy_state_timer_)) {
        /* Delay for >200us to allow preamble to finish */
        TimerDisable(&port->policy_state_timer_);
        TimerStart(&port->policy_state_timer_, ktGoodCRCDelay);
        port->policy_subindex_++;
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    case 2:
      if (TimerExpired(&port->policy_state_timer_)) {
        TimerDisable(&port->policy_state_timer_);
        if (port->policy_is_source_) {
#ifdef FSC_HAVE_SRC
          set_policy_state(port, PE_SRC_Ready);
#endif /* FSC_HAVE_SRC */
        }
        else {
#ifdef FSC_HAVE_SNK
          set_policy_state(port, PE_SNK_Ready);
#endif /* FSC_HAVE_SNK */
        }
        notify_observers(EVENT_BIST_DISABLED, port->port_id_, 0);
      }
      else {
        port->idle_ = TRUE;
      }
      break;
    default:
      set_policy_state(port, PE_ErrorRecovery);
      notify_observers(EVENT_BIST_DISABLED, port->port_id_, 0);
      break;
  }
}

void PolicyBISTTestData(struct Port *port)
{
  /* Nothing needed here.  Wait for detach or reset to end this mode. */
}

void PolicyInvalidState(struct Port *port)
{
  /* reset if we get to an invalid state */
  if (port->policy_is_source_) {
#ifdef FSC_HAVE_SRC
    PolicySourceSendHardReset(port);
#endif /* FSC_HAVE_SRC */
  }
  else {
#ifdef FSC_HAVE_SNK
    PolicySinkSendHardReset(port);
#endif /* FSC_HAVE_SNK */
  }
}

void ProcessCableResetState(struct Port *port)
{
  switch (port->cbl_rst_state_)
  {
  case CBL_RST_START:
    if (port->is_vconn_source_) {
      port->cbl_rst_state_ = CBL_RST_VCONN_SOURCE;
    } else if (port->policy_is_dfp_) {
      port->cbl_rst_state_ = CBL_RST_DR_DFP;
    } else {
      /* Must be dfp and vconn source. Start with VCONN Swap */
      set_policy_state(port, PE_VCS_Send_Swap);
      port->cbl_rst_state_ = CBL_RST_VCONN_SOURCE;
    }
    break;
  case CBL_RST_VCONN_SOURCE:
    if (port->is_vconn_source_) {
      if (port->policy_is_dfp_) {
        port->cbl_rst_state_ = CBL_RST_SEND;
      } else {
        /* Must be dfp and vconn source*/
        set_policy_state(port, port->policy_is_source_ ?
                         PE_DRS_DFP_UFP_Send_Swap : PE_DRS_UFP_DFP_Send_Swap);
        port->cbl_rst_state_ = CBL_RST_DR_DFP;
      }
    } else {
      /* VCONN Swap might have failed */
      port->cbl_rst_state_ = CBL_RST_DISABLED;
    }
    break;
  case CBL_RST_DR_DFP:
    if (port->policy_is_dfp_) {
      if (port->is_vconn_source_) {
        port->cbl_rst_state_ = CBL_RST_SEND;
      } else {
        /* Must be dfp and vconn source*/
        set_policy_state(port, PE_VCS_Send_Swap);
        port->cbl_rst_state_ = CBL_RST_VCONN_SOURCE;
      }
    } else {
      /* DR Swap might have failed */
      port->cbl_rst_state_ = CBL_RST_DISABLED;
    }
    break;
  case CBL_RST_SEND:
    if (port->policy_is_dfp_ &&
        port->is_vconn_source_) {
      set_policy_state(port, PE_DFP_CBL_Send_Cable_Reset);
    } else {
      port->cbl_rst_state_ = CBL_RST_DISABLED;
    }
    break;
  case CBL_RST_DISABLED:
  default:
    break;
  }
}
