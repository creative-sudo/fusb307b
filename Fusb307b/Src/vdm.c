/*******************************************************************************
 * @file     vdm.c
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
/* **************************************************************************
 *  vdm.cpp
 *
 *  An example implementation of the VDM functionality API.
 * ************************************************************************** */
#ifdef FSC_HAVE_VDM

#ifdef FSC_HAVE_DP
#include "display_port.h"
#endif /*  FSC_HAVE_DP */
#include "platform.h" /*  Driver typedefs */
#include "policy.h"   /*  PolicySend() */
#include "port.h"     /*  Port class */
#include "timer.h"    /*  Timer values */
#include "vdm_types.h"
#include "vdm.h"
#include "dpm.h"
#include "observer.h"

#define VDM_ARRAY_LEN 1

SvdmVersion CurrentSVDMVersion(struct Port *port, SopType sop)
{
  if (DPM_CurrentSpecRev(port, sop) == PDSpecRev2p0)
    return V1P0;
  else
    return V2P0;
}

void PolicyVdm(struct Port *port)
{
  /*  Have we received a message from the source? */
  if (port->protocol_msg_rx_ == TRUE) {
    port->protocol_msg_rx_ = FALSE;
    if (port->policy_rx_header_.NumDataObjects != 0) {
      switch (port->policy_rx_header_.MessageType) {
      case DMTVendorDefined:
        if (port->policy_tx_data_obj_[0].SVDM.VDMType == STRUCTURED_VDM &&
            port->policy_rx_data_obj_[0].SVDM.VDMType == STRUCTURED_VDM &&
            port->policy_rx_data_obj_[0].SVDM.CommandType != INITIATOR &&
            (port->policy_tx_data_obj_[0].SVDM.Command ==
            port->policy_rx_data_obj_[0].SVDM.Command))
        {
          ConvertAndProcessVdmMessage(port);
          break;
        }
        /* Fall through */
      default:
        /*  If we get something we are not expecting - simply ignore them and */
        /*  kick out of VDM state (interruptible) */
        ResetPolicyState(port, port->vdm_msg_tx_sop_);
        /*  reset flag so other state can see the message and process */
        port->protocol_msg_rx_ = TRUE;
        break;
      }
    }
    else {
      /*  if not a VDM message, kick out of VDM state (interruptible) */
      ResetPolicyState(port, port->vdm_msg_tx_sop_);
      port->protocol_msg_rx_ = TRUE;
    }
    port->policy_subindex_ = 0;
    port->pd_tx_status_ = txIdle;
  }
  else if (TimerExpired(&port->vdm_timer_)) {
           TimerDisable(&port->vdm_timer_);
     /* Timed out waiting for VDM response */
     ResetPolicyState(port, port->vdm_msg_tx_sop_);
  }
  else {
    port->idle_ = TRUE;
  }


}

void PolicyGiveVdm(struct Port *port)
{
  FSC_U8 result = 0;

  if (port->protocol_msg_rx_ == TRUE &&
      port->policy_rx_header_.MessageType == DMTVendorDefined) {
    /* A new received message can interrupt and current VDM transmission */
    ResetPolicyState(port, port->vdm_msg_tx_sop_);
    port->policy_subindex_ = 0;
    port->pd_tx_status_ = txIdle;
    port->protocol_state_ = PRLIdle;
    return;
  }
  else if (port->vdm_sending_data_ == TRUE) {
    if (port->original_policy_state_ == PE_SRC_Ready) {
      /* Indicates the need for SinkTx handling */
      /* TODO - not if we are responding to a request. */
      //port->policy_is_ams_ = TRUE;
    }

    result = PolicySend(port, DMTVendorDefined, port->vdm_msg_length_ * 4,
                        (FSC_U8 *)port->vdm_msg_obj_, port->vdm_next_ps_, 0,
                        port->vdm_msg_tx_sop_, FALSE);
    if (result == STAT_SUCCESS) {
      if (port->vdm_expecting_response_ == TRUE) {
        StartVdmTimer(port);
      }
      else {
        ResetPolicyState(port, port->vdm_msg_tx_sop_);
      }
      port->vdm_sending_data_ = FALSE;
    }
    else if (result == STAT_ERROR) {
      ResetPolicyState(port, port->vdm_msg_tx_sop_);
      port->vdm_sending_data_ = FALSE;
    }
  }
  else {
    ResetPolicyState(port, port->vdm_msg_tx_sop_);
  }
}

void ConvertAndProcessVdmMessage(struct Port *port)
{
  FSC_U32 i;
  /*  form the word arrays that VDM block expects */
  FSC_U32 vdm_arr[7] = {0};

  for (i = 0; i < port->policy_rx_header_.NumDataObjects; i++) {
    vdm_arr[i] = port->policy_rx_data_obj_[i].object;
  }
  ProcessVdmMessage(port, vdm_arr, port->policy_rx_header_.NumDataObjects);
}

void DoVdmCommand(struct Port *port)
{
  FSC_U32 command;
  FSC_U32 svid;
  FSC_U32 mode_index;
  SopType sop;

#ifdef FSC_DEBUG
  if (port->pd_transmit_objects_[0].UVDM.VDMType == 0)
  {
      /* Transmit unstructured VDM messages directly from the GUI/DPM */
      PolicySend(port, DMTVendorDefined,
                 port->pd_transmit_header_.NumDataObjects * 4,
                 (FSC_U8*)port->pd_transmit_objects_,
                 port->policy_is_source_ ? PE_SRC_Ready : PE_SNK_Ready,0,
                 port->policy_msg_tx_sop_, FALSE);
      return;
  }
#endif /* FSC_DEBUG */

  command = port->pd_transmit_objects_[0].byte[0] & 0x1F;
  svid = 0;
  svid |= (port->pd_transmit_objects_[0].byte[3] << 8);
  svid |= (port->pd_transmit_objects_[0].byte[2] << 0);

  mode_index = port->pd_transmit_objects_[0].byte[1] & 0x7;

  /*  Must be set with correct type when setting port->pd_tx_flag_ */
  sop = port->policy_msg_tx_sop_;

#ifdef FSC_HAVE_DP
  if (svid == DP_SID) {
    if (command == DP_COMMAND_STATUS) {
      DP_RequestPartnerStatus(port);
    }
    else if (command == DP_COMMAND_CONFIG) {
      DisplayPortConfig_t temp;
      temp.word = port->pd_transmit_objects_[1].object;
      DP_RequestPartnerConfig(port, temp);
    }
  }
#endif /*  FSC_HAVE_DP */
  if (command == DISCOVER_IDENTITY) {
    RequestDiscoverIdentity(port, sop);
  }
  else if (command == DISCOVER_SVIDS) {
    RequestDiscoverSvids(port, sop);
  }
  else if (command == DISCOVER_MODES) {
    RequestDiscoverModes(port, sop, svid);
  }
  else if (command == ENTER_MODE) {
    RequestEnterMode(port, sop, svid, mode_index);
  }
  else if (command == EXIT_MODE) {
    RequestExitMode(port, sop, svid, mode_index);
  }
}

void AutoVdmDiscovery(struct Port *port)
{
  if (port->pd_tx_status_ == txIdle)
  {
    switch (port->vdm_auto_state_)
    {
      case AUTO_VDM_INIT:
      case AUTO_VDM_DISCOVER_ID_PP:
        RequestDiscoverIdentity(port, SOP_TYPE_SOP);
        port->vdm_auto_state_ = AUTO_VDM_DISCOVER_SVIDS_PP;
        break;
      case AUTO_VDM_DISCOVER_SVIDS_PP:
        if (port->svid_discvry_done_ == FALSE) {
          RequestDiscoverSvids(port, SOP_TYPE_SOP);
        }
        else {
          port->vdm_auto_state_ = AUTO_VDM_DISCOVER_MODES_PP;
        }
        break;
      case AUTO_VDM_DISCOVER_MODES_PP:
        if (port->svid_discv_idx_ >= 0) {
          RequestDiscoverModes(port, SOP_TYPE_SOP,
              port->core_svid_info_.svids[port->svid_discv_idx_]);
          port->vdm_auto_state_ = AUTO_VDM_ENTER_MODE_PP;
        }
        else {
          /* No known SVIDs found */
          port->vdm_auto_state_ = AUTO_VDM_DONE;
        }
        break;
      case AUTO_VDM_ENTER_MODE_PP:
        if (port->auto_mode_entry_pos_ > 0) {
#ifdef FSC_HAVE_DP
          if (port->core_svid_info_.svids[port->svid_discv_idx_] == DP_SID) {
              if (port->display_port_data_.DpConfigured == TRUE) {
                port->vdm_auto_state_ = AUTO_VDM_DONE;
                break;
              }
              else {
                port->vdm_auto_state_ = AUTO_VDM_DP_GET_STATUS;
              }
          } else {
            port->vdm_auto_state_ = AUTO_VDM_DONE;
          }
#endif /* FSC_HAVE_DP */
          RequestEnterMode(port, SOP_TYPE_SOP,
                          port->core_svid_info_.svids[port->svid_discv_idx_],
                          port->auto_mode_entry_pos_);
        } else {
          port->vdm_auto_state_ = AUTO_VDM_DONE;
        }
        break;
#ifdef FSC_HAVE_DP
      case AUTO_VDM_DP_GET_STATUS:
        if (port->display_port_data_.DpModeEntered > 0) {
          DP_RequestPartnerStatus(port);
          port->vdm_auto_state_ = AUTO_VDM_DP_SET_CONFIG;
        }
        else {
          port->vdm_auto_state_ = AUTO_VDM_DONE;
        }
        break;
      case AUTO_VDM_DP_SET_CONFIG:
        if (port->display_port_data_.DpPpStatus.Connection == DP_MODE_BOTH &&
            port->display_port_data_.DpStatus.Connection == DP_MODE_BOTH) {
          /* If reported both connected send status with only one connected */
          DP_SetPortMode(port, (DisplayPort_Preferred_Snk) ?
                                DP_MODE_UFP_D : DP_MODE_DFP_D);
          port->vdm_auto_state_ = AUTO_VDM_DP_GET_STATUS;
        }
        else if (port->display_port_data_.DpCapMatched &&
            port->display_port_data_.DpPpStatus.Connection > 0) {
          DP_RequestPartnerConfig(port, port->display_port_data_.DpPpConfig);
          port->vdm_auto_state_ = AUTO_VDM_DONE;
        }
        else {
          port->vdm_auto_state_ = AUTO_VDM_DONE;
        }
        break;
#endif /*  FSC_HAVE_DP */
    default:
      port->vdm_auto_state_ = AUTO_VDM_DONE;
      break;
    }
  }
}

/*  Sending/requesting end VDM functionality */
FSC_S32 RequestDiscoverIdentity(struct Port *port, SopType sop)
{
  doDataObject_t vdmh = {0};
  FSC_U32 length = VDM_ARRAY_LEN;
  FSC_U32 arr[VDM_ARRAY_LEN];
  PolicyState_t n_pe;

  if ((port->policy_state_ == PE_SNK_Ready) ||
      (port->policy_state_ == PE_SRC_Ready)) {
    port->original_policy_state_ = port->policy_state_;
    n_pe = PE_INIT_PORT_VDM_Identity_Request;
    /*  TODO: DiscoverIdentityCounter */

    vdmh.SVDM.SVID = PD_SID; /*  PD SID used for Discover Identity command */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = 0; /*  does not matter for Discover Identity */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating discovery */
    vdmh.SVDM.Command = DISCOVER_IDENTITY; /*  discover identity command! */
    arr[0] = vdmh.object;
    SendVdmMessageWithTimeout(port, sop, arr, length, n_pe);
  }
  /*  Allow cable discovery in special earlier states */
  else if ((sop == SOP_TYPE_SOP1) &&
           ((port->policy_state_ == PE_SRC_Startup) ||
            (port->policy_state_ == PE_SRC_Discovery) ||
            (port->policy_state_ == PE_SRC_Send_Capabilities))) {
    port->original_policy_state_ = port->policy_state_;
    n_pe = PE_SRC_VDM_Identity_Request;
    vdmh.SVDM.SVID = PD_SID; /*  PD SID used for Discover Identity command */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = 0; /*  does not matter for Discover Identity */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating discovery */
    vdmh.SVDM.Command = DISCOVER_IDENTITY; /*  discover identity command! */
    arr[0] = vdmh.object;
    SendVdmMessageWithTimeout(port, sop, arr, length, n_pe);
  }
  else {
    return 1;
  }
  return 0;
}

FSC_S32 RequestDiscoverSvids(struct Port *port, SopType sop)
{
  doDataObject_t vdmh = {0};
  FSC_U32 length = VDM_ARRAY_LEN;
  FSC_U32 arr[VDM_ARRAY_LEN];
  PolicyState_t n_pe;

  if ((port->policy_state_ != PE_SNK_Ready) &&
      (port->policy_state_ != PE_SRC_Ready)) {
    return 1;
  }
  else {
    port->original_policy_state_ = port->policy_state_;
    n_pe = PE_INIT_VDM_SVIDs_Request;
    vdmh.SVDM.SVID = PD_SID; /*  PD SID to be used for Discover SVIDs command */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = 0; /*  does not matter for Discover SVIDs */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating discovery */
    vdmh.SVDM.Command = DISCOVER_SVIDS; /*  Discover SVIDs command! */
    arr[0] = vdmh.object;
    SendVdmMessageWithTimeout(port, sop, arr, length, n_pe);
  }
  return 0;
}

FSC_S32 RequestDiscoverModes(struct Port *port, SopType sop, FSC_U16 svid)
{
  doDataObject_t vdmh = {0};
  FSC_U32 length = VDM_ARRAY_LEN;
  FSC_U32 arr[VDM_ARRAY_LEN] = {0};
  PolicyState_t n_pe;

  if ((port->policy_state_ != PE_SNK_Ready) &&
      (port->policy_state_ != PE_SRC_Ready)) {
    return 1;
  }
  else {
    port->original_policy_state_ = port->policy_state_;
    n_pe = PE_INIT_VDM_Modes_Request;
    vdmh.SVDM.SVID = svid; /*  Use the SVID that was discovered */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = 0; /*  does not matter for Discover Modes */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating discovery */
    vdmh.SVDM.Command = DISCOVER_MODES; /*  Discover MODES command! */
    arr[0] = vdmh.object;
    SendVdmMessageWithTimeout(port, sop, arr, length, n_pe);
  }
  return 0;
}

FSC_S32 RequestSendAttention(struct Port *port, SopType sop, FSC_U16 svid,
                             FSC_U8 mode)
{
  doDataObject_t vdmh = {0};
  FSC_U32 length = VDM_ARRAY_LEN;
  FSC_U32 arr[length];

  port->original_policy_state_ = port->policy_state_;

  if ((port->policy_state_ != PE_SNK_Ready) &&
      (port->policy_state_ != PE_SRC_Ready)) {
    return 1;
  }
  else {
    vdmh.SVDM.SVID = svid; /*  Use the SVID that needs attention */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = mode; /*  use the mode index that needs attention */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating attention */
    vdmh.SVDM.Command = ATTENTION; /*  Attention command! */
    arr[0] = vdmh.object;
    length = 1;
    SendVdmMessage(port, sop, arr, length, port->policy_state_);
  }
  return 0;
}

FSC_S32 RequestEnterMode(struct Port *port, SopType sop, FSC_U16 svid,
                         FSC_U32 mode_index)
{
  doDataObject_t vdmh = {0};
  FSC_U32 length = VDM_ARRAY_LEN;
  FSC_U32 arr[VDM_ARRAY_LEN];
  PolicyState_t n_pe;

  if ((port->policy_state_ != PE_SNK_Ready) &&
      (port->policy_state_ != PE_SRC_Ready)) {
    return 1;
  }
  else {
    port->original_policy_state_ = port->policy_state_;
    n_pe = PE_DFP_VDM_Mode_Entry_Request;
    vdmh.SVDM.SVID = svid; /*  Use SVID specified upon function call */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = mode_index; /*  select mode */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating mode entering */
    vdmh.SVDM.Command = ENTER_MODE; /*  Enter Mode command! */
    arr[0] = vdmh.object;
    SendVdmMessageWithTimeout(port, sop, arr, length, n_pe);
  }
  return 0;
}

FSC_S32 RequestExitMode(struct Port *port, SopType sop, FSC_U16 svid,
                        FSC_U32 mode_index)
{
  doDataObject_t vdmh = {0};
  FSC_U32 length = VDM_ARRAY_LEN;
  FSC_U32 arr[VDM_ARRAY_LEN];
  PolicyState_t n_pe;

  if ((port->policy_state_ != PE_SNK_Ready) &&
      (port->policy_state_ != PE_SRC_Ready)) {
    return 1;
  }
  else {
    port->original_policy_state_ = port->policy_state_;
    n_pe = PE_DFP_VDM_Mode_Exit_Request;
    vdmh.SVDM.SVID = svid; /*  Use SVID specified upon function call */
    vdmh.SVDM.VDMType = STRUCTURED_VDM; /*  structured VDM Header */
    vdmh.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh.SVDM.ObjPos = mode_index; /*  select mode */
    vdmh.SVDM.CommandType = INITIATOR; /*  we are initiating mode entering */
    vdmh.SVDM.Command = EXIT_MODE; /*  Exit Mode command! */
    arr[0] = vdmh.object;
    SendVdmMessageWithTimeout(port, sop, arr, length, n_pe);
  }
  return 0;
}

/*  Receiving end VDM functionality */
FSC_S32 ProcessVdmMessage(struct Port *port, FSC_U32* arr_in, FSC_U32 length_in)
{
  doDataObject_t vdmh_in = {0};

  vdmh_in.object = arr_in[0];

  if (vdmh_in.SVDM.VDMType == STRUCTURED_VDM) {
    /* Check for VIF support - if not FALSE/FALSE, ACK or NAK as needed */
    if (vdmh_in.SVDM.CommandType == INITIATOR &&
        Responds_To_Discov_SOP_DFP == FALSE &&
        Responds_To_Discov_SOP_UFP == FALSE) {
      if (DPM_CurrentSpecRev(port, SOP_TYPE_SOP) == PDSpecRev3p0) {
        /* Send NS for PD 3.0 */
        PolicySendNotSupported(port);
      }
      else {
        /* Simply ignore for PD 2.0 */
      }

      return 0;
    }

    switch (vdmh_in.SVDM.Command) {
      case DISCOVER_IDENTITY:
        return ProcessDiscoverIdentity(port, port->protocol_msg_rx_sop_,
                                       arr_in, length_in);
      case DISCOVER_SVIDS:
        return ProcessDiscoverSvids(port, port->protocol_msg_rx_sop_,
                                    arr_in, length_in);
      case DISCOVER_MODES:
        return ProcessDiscoverModes(port, port->protocol_msg_rx_sop_,
                                    arr_in, length_in);
      case ENTER_MODE:
        return ProcessEnterMode(port, port->protocol_msg_rx_sop_, arr_in,
                                length_in);
      case EXIT_MODE:
        return ProcessExitMode(port, port->protocol_msg_rx_sop_, arr_in,
                               length_in);
      case ATTENTION:
        return ProcessAttention(port, port->protocol_msg_rx_sop_, arr_in,
                                length_in);
      default:
        /*  SVID-Specific commands go here */
        return ProcessSvidSpecific(port, port->protocol_msg_rx_sop_, arr_in,
                                   length_in);
    }
  }
  else {
      /* TODO: Unstructured messages */
      /* Unstructured VDM's not supported at this time */
      if (DPM_CurrentSpecRev(port, SOP_TYPE_SOP) == PDSpecRev3p0)
      {
          /* Not supported in PD3.0, ignored in PD2.0 */
          set_policy_state(port, port->policy_is_source_ == TRUE ?
              PE_SRC_Send_Not_Supported : PE_SNK_Send_Not_Supported);
      }

    return 1;
  }
}

/**
 * Determine message applicability or whether to a response is required
 */
FSC_BOOL evalResponseToSopVdm(struct Port *port, doDataObject_t vdm_hdr)
{
    FSC_BOOL response = TRUE;
    if (port->policy_is_dfp_ == TRUE && !Responds_To_Discov_SOP_DFP) {
      response = FALSE;
    }
    else if (port->policy_is_dfp_ == FALSE && !Responds_To_Discov_SOP_UFP) {
      response = FALSE;
    }
    else if (DPM_CurrentSpecRev(port, SOP_TYPE_SOP) < PDSpecRev3p0 &&
             port->policy_is_dfp_ == TRUE) {
      /* See message applicability */
      response = FALSE;
    }
    else if (!(port->policy_state_ == PE_SRC_Ready ||
               port->policy_state_ == PE_SNK_Ready)) {
        /* Neither sink ready or source ready state */
      response = FALSE;
    }
    return response;
}

/**
 * Determine message applicability or whether to a response is required
 */
FSC_BOOL evalResponseToCblVdm(struct Port *port, doDataObject_t vdm_hdr)
{
    FSC_BOOL response = TRUE;
    if (port->policy_state_ != PE_CBL_Ready) {
        response = FALSE;
    }
    return response;
}

FSC_S32 ProcessDiscoverIdentity(struct Port *port, SopType sop, FSC_U32* arr_in,
                                FSC_U32 length_in)
{
  doDataObject_t vdmh_in = {0};
  doDataObject_t vdmh_out = {0};
  IdHeader idh;
  CertStatVdo csvdo;
  Identity id;
  ProductVdo pvdo;
  FSC_U32 arr[7] = {0};
  FSC_U32 length;
  FSC_BOOL result;

  vdmh_in.object = arr_in[0];

  /*  Must NAK or not respond to Discover ID with wrong SVID */
  if (vdmh_in.SVDM.SVID != PD_SID) return -1;

  id.nack = TRUE;
  if (vdmh_in.SVDM.CommandType == INITIATOR) {
    port->original_policy_state_ = port->policy_state_;
    if (sop == SOP_TYPE_SOP) {
      if (evalResponseToSopVdm(port, vdmh_in)) {
        id = VdmRequestIdentityInfo(port, sop);
      }
      set_policy_state(port, PE_RESP_VDM_Get_Identity);
      set_policy_state(port, id.nack ? PE_RESP_VDM_Get_Identity_NAK :
                                       PE_RESP_VDM_Send_Identity);
    } else if (sop == SOP_TYPE_SOP1) {
      if(evalResponseToCblVdm(port, vdmh_in)) {
        id = VdmRequestIdentityInfo(port, sop);
      }
      set_policy_state(port, PE_RESP_VDM_Get_Identity);
      set_policy_state(port, id.nack ? PE_RESP_VDM_Get_Identity_NAK :
                                       PE_RESP_VDM_Send_Identity);
    }

    vdmh_out.SVDM.SVID = PD_SID; /* Use PS_SID for DiscId, even on response */
    vdmh_out.SVDM.VDMType = STRUCTURED_VDM; /* DiscId is Structured */
    vdmh_out.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh_out.SVDM.ObjPos = 0; /*  doesn't matter for Discover Identity */
    vdmh_out.SVDM.CommandType = id.nack ? RESPONDER_NAK : RESPONDER_ACK;
    vdmh_out.SVDM.Command = DISCOVER_IDENTITY; /*  Reply with same command */
    arr[0] = vdmh_out.object;
    length = 1;

    /* TODO: Optimize the nack and ack from here. */
    if (id.nack == FALSE) {
      /*  put capabilities into ID Header */
      idh = id.id_header;

      /*  put test ID into Cert Stat VDO Object */
      csvdo = id.cert_stat_vdo;

      arr[1] = getBitsForIdHeader(idh);
      length++;
      arr[2] = getBitsForCertStatVdo(csvdo);
      length++;

      /*  Product VDO should be sent for all */
      pvdo = id.product_vdo;
      arr[length] = getBitsForProductVdo(pvdo);
      length++;

      /* Cable VDO should be sent when we are a Passive Cable or Active Cable */
      if ((idh.product_type_ufp == PASSIVE_CABLE) ||
          (idh.product_type_ufp == ACTIVE_CABLE)) {
        CableVdo cvdo_out;
        cvdo_out = id.cable_vdo;
        arr[length] = getBitsForCableVdo(cvdo_out);
        length++;
      }

      /*  AMA VDO should be sent when we are an AMA! */
      if (idh.product_type_ufp == AMA) {
        AmaVdo amavdo_out;
        amavdo_out = id.ama_vdo;

        arr[length] = getBitsForAmaVdo(amavdo_out);
        length++;
      }
    }

    SendVdmMessage(port, sop, arr, length, port->original_policy_state_);
    return 0;
  }
  else {
    if ((port->policy_state_ != PE_INIT_PORT_VDM_Identity_Request) &&
        (port->policy_state_ != PE_SRC_VDM_Identity_Request)) {
      /* Prevent random discover identity ack/nak message through */
      return 0;
    }

    if (port->policy_state_ == PE_INIT_PORT_VDM_Identity_Request) {
      if (vdmh_in.SVDM.CommandType == RESPONDER_ACK) {
        set_policy_state(port, PE_INIT_PORT_VDM_Identity_ACKed);
      }
      else {
        set_policy_state(port, PE_INIT_PORT_VDM_Identity_NAKed);
        /* Discontinue auto vdm discovery if result is NAK */
        if (sop == SOP_TYPE_SOP) {
          port->vdm_auto_state_ = AUTO_VDM_DONE;
        }
      }
    }
    else if ((port->policy_state_ == PE_SRC_VDM_Identity_Request) &&
             ((sop == SOP_TYPE_SOP) || (sop == SOP_TYPE_SOP1))) {
      if (vdmh_in.SVDM.CommandType == RESPONDER_ACK) {
        set_policy_state(port, PE_SRC_VDM_Identity_ACKed);
      }
      else {
        set_policy_state(port, PE_SRC_VDM_Identity_NAKed);
      }
    }
    else {
      /*  TODO: something weird happened. */
    }

    if (sop == SOP_TYPE_SOP1)
    {
      /* Getting ACK/NAK here means that cable responded */
      port->vdm_cbl_present_ = TRUE;
      port->vdm_check_cbl_ = FALSE;
      /* Check the PD Revision for cable */
      port->dpm_pd_30_srccab_ = (vdmh_in.SVDM.Version == V2P0) ? TRUE : FALSE;
    }

    /* set to true when valid response received */
    result = FALSE;
    /*
     * Discover Identity responses should have at least VDM Header, ID Header,
     * and Cert Stat VDO
     */
    if (length_in >= MIN_DISC_ID_RESP_SIZE) {
      if (vdmh_in.SVDM.CommandType == RESPONDER_ACK) {
        id.id_header = getIdHeader(arr_in[1]);
        id.cert_stat_vdo = getCertStatVdo(arr_in[2]);

        if ((id.id_header.product_type_ufp == HUB)
            || (id.id_header.product_type_ufp == PERIPHERAL)
            || (id.id_header.product_type_ufp == PASSIVE_CABLE)
            || (id.id_header.product_type_ufp == ACTIVE_CABLE)
            || (id.id_header.product_type_ufp == AMA)) {
          id.has_product_vdo = TRUE;
          /*  !!! assuming it is before AMA VDO */
          id.product_vdo = getProductVdo(arr_in[3]);
        }

        if ((id.id_header.product_type_ufp == PASSIVE_CABLE)
            || (id.id_header.product_type_ufp == ACTIVE_CABLE)) {
          id.has_cable_vdo = TRUE;
          id.cable_vdo = getCableVdo(arr_in[4]);
        }

        if ((id.id_header.product_type_ufp == AMA)) {
          id.has_ama_vdo = TRUE;
          /*  !!! assuming it is after Product VDO */
          id.ama_vdo = getAmaVdo(arr_in[4]);
        }

        port->vdm_expecting_response_ = FALSE;
        TimerDisable(&port->vdm_timer_);
        result = TRUE;
      }
    }

    VdmInformIdentity(port, result, sop, id);
    set_policy_state(port, port->original_policy_state_);
    return 0;
  }
}

FSC_S32 ProcessDiscoverSvids(struct Port *port, SopType sop, FSC_U32* arr_in,
                             FSC_U32 length_in)
{
  doDataObject_t vdmh_in = {0};
  doDataObject_t vdmh_out = {0};
  SvidInfo svid_info;
  FSC_U32 i;
  FSC_U16 top16;
  FSC_U16 bottom16;
  FSC_U32 arr[7] = {0};
  FSC_U32 length;

  vdmh_in.object = arr_in[0];

  /* Must NAK or not respond to Discover SVIDs with wrong SVID */
  if (vdmh_in.SVDM.SVID != PD_SID) return -1;

  svid_info.nack = TRUE;
  if (vdmh_in.SVDM.CommandType == INITIATOR) {
    port->original_policy_state_ = port->policy_state_;
    if (sop == SOP_TYPE_SOP) {
      if (evalResponseToSopVdm(port, vdmh_in)) {
        /*  assuming that the splitting of SVID info is done outside this block */
        svid_info = VdmRequestSvidInfo(port);
      }
      set_policy_state(port, PE_RESP_VDM_Get_SVIDs);
      set_policy_state(port, svid_info.nack ? PE_RESP_VDM_Get_SVIDs_NAK :
                                              PE_RESP_VDM_Send_SVIDs);
    }
    else if (sop == SOP_TYPE_SOP1) {
      if (evalResponseToCblVdm(port, vdmh_in)) {
        svid_info = VdmRequestSvidInfo(port);
      }
      set_policy_state(port, PE_RESP_VDM_Get_SVIDs);
      set_policy_state(port, svid_info.nack ? PE_RESP_VDM_Get_SVIDs_NAK :
                                              PE_RESP_VDM_Send_SVIDs);
    }

    vdmh_out.SVDM.SVID = PD_SID; /* Use PS_SID for Disc SVIDs */
    vdmh_out.SVDM.VDMType = STRUCTURED_VDM; /*  Discovery SVIDs is Structured */
    vdmh_out.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh_out.SVDM.ObjPos = 0; /*  doesn't matter for Discover SVIDs */
    vdmh_out.SVDM.CommandType = svid_info.nack ? RESPONDER_NAK : RESPONDER_ACK;

    /*  Reply with same command, Discover SVIDs */
    vdmh_out.SVDM.Command = DISCOVER_SVIDS;
    length = 0;
    arr[length] = vdmh_out.object;
    length++;
    if (svid_info.nack == FALSE) {
      /*  prevent segfaults */
      if (svid_info.num_svids > MAX_NUM_SVIDS) {
        set_policy_state(port, port->original_policy_state_);
        return 1;
      }
      for (i = 0; i < svid_info.num_svids; i++) {
        /*  check if i is even */
        if (!(i & 0x1)) {
          length++;
          /*  setup new word to send */
          arr[length - 1] = 0;
          /*  if even, shift SVID up to the top 16 bits */
          arr[length - 1] |= svid_info.svids[i];
          arr[length - 1] <<= 16;
        }
        else {
          /*  if odd, fill out the bottom 16 bits */
          arr[length - 1] |= svid_info.svids[i];
        }
      }
    }
    SendVdmMessage(port, sop, arr, length, port->original_policy_state_);
    return 0;
  }
  else {
    /* Incoming responses, ACKs, NAKs, BUSYs */
    svid_info.num_svids = 0;

    if (port->policy_state_ != PE_INIT_VDM_SVIDs_Request) {
      return 1;
    }
    else if (vdmh_in.SVDM.CommandType == RESPONDER_ACK) {
      for (i = 1; i < length_in; i++) {
        top16 = (arr_in[i] >> 16) & 0xFFFF;
        bottom16 = (arr_in[i] >> 0) & 0xFFFF;

        /*  if top 16 bits are 0, we're done getting SVIDs */
        if (top16 == 0) {
          break;
        }
        else {
          svid_info.svids[2 * (i - 1)] = top16;
          svid_info.num_svids += 1;
        }
        /*  if bottom 16 bits are 0 we're done getting SVIDs */
        if (bottom16 == 0) {
          break;
        }
        else {
          svid_info.svids[2 * (i - 1) + 1] = bottom16;
          svid_info.num_svids += 1;
        }
      }
      set_policy_state(port, PE_INIT_VDM_SVIDs_ACKed);
    }
    else {
      set_policy_state(port, PE_INIT_VDM_SVIDs_NAKed);
    }
    VdmInformSvids(port,
                   port->policy_state_ == PE_INIT_VDM_SVIDs_ACKed ? TRUE:FALSE,
                   sop, svid_info);
    port->vdm_expecting_response_ = FALSE;
    TimerDisable(&port->vdm_timer_);
    set_policy_state(port, port->original_policy_state_);
    return 0;
  }
}

FSC_S32 ProcessDiscoverModes(struct Port *port, SopType sop, FSC_U32 *arr_in,
                             FSC_U32 length_in)
{
  doDataObject_t vdmh_in = {0};
  doDataObject_t vdmh_out = {0};
  ModesInfo modes_info;
  FSC_U32 i, j;
  FSC_U32 arr[7] = {0};
  FSC_U32 length;

  vdmh_in.object = arr_in[0];

  modes_info.nack = TRUE;
  if (vdmh_in.SVDM.CommandType == INITIATOR) {
    port->original_policy_state_ = port->policy_state_;
    if (sop == SOP_TYPE_SOP) {
      if (evalResponseToSopVdm(port, vdmh_in)) {
        modes_info = VdmRequestModesInfo(port, vdmh_in.SVDM.SVID);
      }
      set_policy_state(port, PE_RESP_VDM_Get_Modes);
      set_policy_state(port, modes_info.nack ? PE_RESP_VDM_Get_Modes_NAK :
                                               PE_RESP_VDM_Send_Modes);
    }

    else if (sop == SOP_TYPE_SOP1) {
      if (evalResponseToCblVdm(port, vdmh_in)) {
        modes_info = VdmRequestModesInfo(port, vdmh_in.SVDM.SVID);
      }
      set_policy_state(port, PE_RESP_VDM_Get_Modes);
      set_policy_state(port, modes_info.nack ? PE_RESP_VDM_Get_Modes_NAK :
                                               PE_RESP_VDM_Send_Modes);
    }

    vdmh_out.SVDM.SVID = vdmh_in.SVDM.SVID;
    vdmh_out.SVDM.VDMType = STRUCTURED_VDM;
    vdmh_out.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh_out.SVDM.ObjPos = 0;
    vdmh_out.SVDM.CommandType = modes_info.nack ? RESPONDER_NAK : RESPONDER_ACK;

    /*  Reply with same command, Discover Modes */
    vdmh_out.SVDM.Command = DISCOVER_MODES;
    length = 0;
    arr[length] = vdmh_out.object;
    length++;
    if (modes_info.nack == FALSE) {
      for (j = 0; j < modes_info.num_modes; j++) {
        arr[j + 1] = modes_info.modes[j];
        length++;
      }
    }
    SendVdmMessage(port, sop, arr, length, port->original_policy_state_);
    return 0;
  }
  else {
    /*  Incoming responses, ACKs, NAKs, BUSYs */
    if (port->policy_state_ != PE_INIT_VDM_Modes_Request) {
      return 1;
    }
    else {
      if (vdmh_in.SVDM.CommandType == RESPONDER_ACK) {
        modes_info.svid = vdmh_in.SVDM.SVID;
        modes_info.num_modes = length_in - 1;
        modes_info.nack = FALSE;
        for (i = 1; i < length_in; i++) {
          modes_info.modes[i - 1] = arr_in[i];
        }
        set_policy_state(port, PE_INIT_VDM_Modes_ACKed);
      }
      else {
        modes_info.nack = TRUE;
        set_policy_state(port, PE_INIT_VDM_Modes_NAKed);
      }
      VdmInformModes(port,
                 port->policy_state_ == PE_INIT_VDM_Modes_ACKed ? TRUE : FALSE,
                 sop, modes_info);
      port->vdm_expecting_response_ = FALSE;
      TimerDisable(&port->vdm_timer_);
      set_policy_state(port, port->original_policy_state_);
    }
    return 0;
  }
}

FSC_S32 ProcessEnterMode(struct Port *port, SopType sop, FSC_U32 *arr_in,
                         FSC_U32 length_in)
{
  doDataObject_t svdmh_in = {0};
  doDataObject_t svdmh_out = {0};
  FSC_BOOL mode_entered = FALSE;
  FSC_U32 arr_out[7] = {0};
  FSC_U32 length_out;

  svdmh_in.object = arr_in[0];

  if (svdmh_in.SVDM.CommandType == INITIATOR) {
    port->original_policy_state_ = port->policy_state_;
    if (sop == SOP_TYPE_SOP) {
      if (evalResponseToSopVdm(port, svdmh_in)) {
        mode_entered = VdmModeEntryRequest(port, svdmh_in.SVDM.SVID,
                                                 svdmh_in.SVDM.ObjPos);
      }
      set_policy_state(port, PE_UFP_VDM_Evaluate_Mode_Entry);
      set_policy_state(port, mode_entered ? PE_UFP_VDM_Mode_Entry_ACK :
                                            PE_UFP_VDM_Mode_Entry_NAK);
      svdmh_out.SVDM.CommandType = mode_entered ? RESPONDER_ACK :
                                                  RESPONDER_NAK;
    }
    else if (sop == SOP_TYPE_SOP1) {
      if (evalResponseToCblVdm(port, svdmh_in)) {
        mode_entered = VdmModeEntryRequest(port, svdmh_in.SVDM.SVID,
                                                 svdmh_in.SVDM.ObjPos);
      }
      set_policy_state(port, PE_CBL_Evaluate_Mode_Entry);
      set_policy_state(port, mode_entered ? PE_CBL_Mode_Entry_ACK :
                                            PE_CBL_Mode_Entry_NAK);
      svdmh_out.SVDM.CommandType = mode_entered ? RESPONDER_ACK :
                                                  RESPONDER_NAK;
    }

    /*  most of the message response will be the same whether we entered
     *  the mode or not */
    svdmh_out.SVDM.SVID = svdmh_in.SVDM.SVID;
    svdmh_out.SVDM.VDMType = STRUCTURED_VDM;
    svdmh_out.SVDM.Version = CurrentSVDMVersion(port, sop);
    svdmh_out.SVDM.ObjPos = svdmh_in.SVDM.ObjPos;
    svdmh_out.SVDM.Command = ENTER_MODE;
    arr_out[0] = svdmh_out.object;
    length_out = 1;
    SendVdmMessage(port, sop, arr_out, length_out,port->original_policy_state_);
    return 0;
  }
  else { /* Incoming responses, ACKs, NAKs, BUSYs */
    if (svdmh_in.SVDM.CommandType != RESPONDER_ACK) {
      set_policy_state(port, PE_DFP_VDM_Mode_Entry_NAKed);
      VdmEnterModeResult(port, FALSE, svdmh_in.SVDM.SVID,
                         svdmh_in.SVDM.ObjPos);
    }
    else {
      set_policy_state(port, PE_DFP_VDM_Mode_Entry_ACKed);
      VdmEnterModeResult(port, TRUE, svdmh_in.SVDM.SVID, svdmh_in.SVDM.ObjPos);
    }
    set_policy_state(port, port->original_policy_state_);
    port->vdm_expecting_response_ = FALSE;
    return 0;
  }
}

FSC_S32 ProcessExitMode(struct Port *port, SopType sop, FSC_U32 *arr_in,
                        FSC_U32 length_in)
{
  doDataObject_t vdmh_in = {0};
  doDataObject_t vdmh_out = {0};
  FSC_BOOL mode_exited;
  FSC_U32 arr[7] = {0};
  FSC_U32 length;

  vdmh_in.object = arr_in[0];

  mode_exited = FALSE;
  if (vdmh_in.SVDM.CommandType == INITIATOR) {
    port->original_policy_state_ = port->policy_state_;
    if (sop == SOP_TYPE_SOP) {
      if (evalResponseToSopVdm(port, vdmh_in)) {
        mode_exited = VdmModeExitRequest(port, vdmh_in.SVDM.SVID,
                                               vdmh_in.SVDM.ObjPos);
      }
      set_policy_state(port, PE_UFP_VDM_Mode_Exit);
      set_policy_state(port, mode_exited ? PE_UFP_VDM_Mode_Exit_ACK :
                                           PE_UFP_VDM_Mode_Exit_NAK);
      vdmh_out.SVDM.CommandType = mode_exited ? RESPONDER_ACK :
                                                RESPONDER_NAK ;
    }
    else if (sop == SOP_TYPE_SOP1) {
      if (evalResponseToCblVdm(port, vdmh_in)) {
        mode_exited = VdmModeExitRequest(port, vdmh_in.SVDM.SVID,
                                               vdmh_in.SVDM.ObjPos);
      }
      set_policy_state(port, PE_CBL_Mode_Exit);
      set_policy_state(port, mode_exited? PE_CBL_Mode_Exit_ACK :
                                          PE_CBL_Mode_Exit_NAK);
      vdmh_out.SVDM.CommandType = mode_exited ? RESPONDER_ACK : RESPONDER_NAK;
    }

    vdmh_out.SVDM.SVID = vdmh_in.SVDM.SVID;
    vdmh_out.SVDM.VDMType = STRUCTURED_VDM;
    vdmh_out.SVDM.Version = CurrentSVDMVersion(port, sop);
    vdmh_out.SVDM.ObjPos = vdmh_in.SVDM.ObjPos;
    vdmh_out.SVDM.Command = EXIT_MODE;
    arr[0] = vdmh_out.object;
    length = 1;
    SendVdmMessage(port, sop, arr, length, port->original_policy_state_);
    return 0;
  }
  else {
    if (vdmh_in.SVDM.CommandType != RESPONDER_ACK) {
      VdmExitModeResult(port, FALSE, vdmh_in.SVDM.SVID, vdmh_in.SVDM.ObjPos);
      /*  when exit mode not ACKed, go to hard reset state! */
      if (port->original_policy_state_ == PE_SRC_Ready) {
        set_policy_state(port, PE_SRC_Hard_Reset);
      }
      else if (port->original_policy_state_ == PE_SNK_Ready) {
        set_policy_state(port, PE_SNK_Hard_Reset);
      }
      else {
        /*  TODO: should never reach here, but you never know... */
      }
    }
    else {
      set_policy_state(port, PE_DFP_VDM_Mode_Exit_ACKed);
      VdmExitModeResult(port, TRUE, vdmh_in.SVDM.SVID, vdmh_in.SVDM.ObjPos);
      set_policy_state(port, port->original_policy_state_);
      TimerDisable(&port->vdm_timer_);
    }
    port->vdm_expecting_response_ = FALSE;
    return 0;
  }
}

FSC_S32 ProcessAttention(struct Port *port, SopType sop, FSC_U32* arr_in,
                         FSC_U32 length_in)
{
  doDataObject_t vdmh_in = {0};

  vdmh_in.object = arr_in[0];
  port->original_policy_state_ = port->policy_state_;
  set_policy_state(port, PE_RCV_VDM_Attention_Request);
  set_policy_state(port, port->original_policy_state_);

#ifdef FSC_HAVE_DP
  if (vdmh_in.SVDM.SVID == DP_SID)
  {
    DP_ProcessCommand(port, arr_in);
  }
  else
#endif /* FSC_HAVE_DP */
  {
    VdmInformAttention(port, vdmh_in.SVDM.SVID, vdmh_in.SVDM.ObjPos);
  }

  return 0;
}

FSC_S32 ProcessSvidSpecific(struct Port *port, SopType sop, FSC_U32 *arr_in,
                            FSC_U32 length_in)
{
  doDataObject_t vdmh_out = {0};
  doDataObject_t vdmh_in = {0};
  FSC_U32 arr[7] = {0};
  FSC_U32 length;

  vdmh_in.object = arr_in[0];

#ifdef FSC_HAVE_DP
  if (vdmh_in.SVDM.SVID == DP_SID) {
    if (!DP_ProcessCommand(port, arr_in)) {
      return 0; /* DP code will send response, so return */
    }
  }
#endif /*  FSC_HAVE_DP */
  /*  in this case the command is unrecognized. Reply with a NAK. */
  vdmh_out.SVDM.SVID = vdmh_in.SVDM.SVID;
  vdmh_out.SVDM.VDMType = STRUCTURED_VDM;
  vdmh_out.SVDM.Version = CurrentSVDMVersion(port, sop);
  vdmh_out.SVDM.ObjPos = 0;
  vdmh_out.SVDM.CommandType = RESPONDER_NAK;
  vdmh_out.SVDM.Command = vdmh_in.SVDM.Command;
  arr[0] = vdmh_out.object;
  length = 1;
  SendVdmMessage(port, sop, arr, length, port->original_policy_state_);
  return 0;
}

/*  Internal utility functions */
void SendVdmMessage(struct Port *port, SopType sop, FSC_U32 *arr,
                    FSC_U32 length, PolicyState_t next_ps) {
  FSC_U32 i;

  port->vdm_msg_length_ = length;
  port->vdm_next_ps_ = next_ps;
  for (i = 0; i < port->vdm_msg_length_; i++) {
    port->vdm_msg_obj_[i].object = arr[i];
  }
  port->vdm_msg_tx_sop_  = sop;
  port->vdm_sending_data_ = TRUE;
  TimerDisable(&port->vdm_timer_);
  set_policy_state(port, PE_GIVE_VDM);
  PolicyGiveVdm(port);
  port->idle_ = FALSE;
}

void SendVdmMessageWithTimeout(struct Port *port, SopType sop, FSC_U32 *arr,
                               FSC_U32 length, PolicyState_t n_pe)
{
  SendVdmMessage(port, sop, arr, length, n_pe);
  port->vdm_expecting_response_ = TRUE;
}

void StartVdmTimer(struct Port *port)
{
  /*  start the appropriate timer */
  switch (port->policy_state_) {
    case PE_INIT_PORT_VDM_Identity_Request:
    case PE_SRC_VDM_Identity_Request:
    case PE_INIT_VDM_SVIDs_Request:
    case PE_INIT_VDM_Modes_Request:
      TimerStart(&port->vdm_timer_, ktVDMSenderResponse);
      break;
    case PE_DFP_VDM_Mode_Entry_Request:
      TimerStart(&port->vdm_timer_, ktVDMWaitModeEntry);
      break;
    case PE_DFP_VDM_Mode_Exit_Request:
      TimerStart(&port->vdm_timer_, ktVDMWaitModeExit);
      break;
//    case peDpRequestStatus:
//      TimerStart(&port->vdm_timer_, ktVDMSenderResponse);
//      break;
    default:
      /*  Time out immediately */
      TimerStart(&port->vdm_timer_, ktVDMSenderResponse);
      return;
  }
}

void ResetPolicyState(struct Port *port, SopType sop) {
  /*  fake empty id, etc, Discover Identity for NAKs */
  Identity id = {0};
  SvidInfo svid_info = {0};
  ModesInfo modes_info = {0};

  port->vdm_expecting_response_ = FALSE;
  TimerDisable(&port->vdm_timer_);

  if (port->policy_state_ == PE_GIVE_VDM) {
    set_policy_state(port, port->vdm_next_ps_);
  }
  /* Reset from PE_GIVE_VDM is when message did not receive GoodCRC.
   * Reset from PE_VDM is when the message received GoodCRC. */
  else if (sop == SOP_TYPE_SOP1 &&
      port->vdm_cbl_present_ == FALSE &&
      (port->policy_state_ == PE_INIT_PORT_VDM_Identity_Request ||
      port->policy_state_ == PE_SRC_VDM_Identity_Request)) {
    /* Cable discover id was successfully sent but the cable did not respond
     * with VDM. Try switching to PD 2.0 */
    if (port->dpm_pd_30_srccab_ == TRUE) {
      /* Set PD 2.0 for cable */
      port->dpm_pd_30_srccab_ = FALSE;
      /* Reset auto VDM only when it is active. It will prevent GUI from
       * activating AUTO vdm discovery when sending SOP' discovery */
      port->vdm_check_cbl_ = TRUE;
    }
  }

  if (sop == SOP_TYPE_SOP &&
      port->vdm_auto_state_ != AUTO_VDM_DONE) {
    port->vdm_auto_state_ = AUTO_VDM_DONE;
  }

  switch (port->policy_state_) {
  case PE_INIT_PORT_VDM_Identity_Request:
    /* informing of a NAK */
    set_policy_state(port, PE_INIT_PORT_VDM_Identity_NAKed);
    VdmInformIdentity(port, FALSE, sop, id);
    set_policy_state(port, port->original_policy_state_);
    break;
  case PE_INIT_VDM_SVIDs_Request:
    set_policy_state(port, PE_INIT_VDM_SVIDs_NAKed);
    VdmInformSvids(port, FALSE, sop, svid_info);
    set_policy_state(port, port->original_policy_state_);
    break;
  case PE_INIT_VDM_Modes_Request:
    set_policy_state(port, PE_INIT_VDM_Modes_NAKed);
    VdmInformModes(port, FALSE, sop, modes_info);
    set_policy_state(port, port->original_policy_state_);
    break;
  case PE_DFP_VDM_Mode_Entry_Request:
    set_policy_state(port, PE_DFP_VDM_Mode_Entry_NAKed);
    VdmEnterModeResult(port, FALSE, 0, 0);
    set_policy_state(port, port->original_policy_state_);
    break;
  case PE_DFP_VDM_Mode_Exit_Request:
    VdmExitModeResult(port, FALSE, 0, 0);

    /*  if Mode Exit request is NAKed, go to hard reset state! */
    if (port->original_policy_state_ == PE_SNK_Ready) {
      set_policy_state(port, PE_SNK_Hard_Reset);
    }
    else if (port->original_policy_state_ == PE_SRC_Ready) {
      set_policy_state(port, PE_SRC_Hard_Reset);
    }
    else {
      /*  TODO: should never reach here, but... */
    }
    set_policy_state(port, port->original_policy_state_);
    return;
  case PE_SRC_VDM_Identity_Request:
    /*  informing of a NAK from cable */
    set_policy_state(port, PE_SRC_VDM_Identity_NAKed);
    VdmInformIdentity(port, FALSE, sop, id);
    set_policy_state(port, port->original_policy_state_);
    break;
  case PE_DFP_CBL_Send_Soft_Reset:
  case PE_DFP_CBL_Send_Cable_Reset:
    /* Allow PE to continue to reset states */
    break;
  default:
    set_policy_state(port, port->original_policy_state_);
    break;
  }
}

/*  VDM "Callback" functionality (TODO) */
/*  TODO: These are the "vdm callback" functions from the 30x */
Identity VdmRequestIdentityInfo(struct Port *port, SopType sop)
{
  Identity id = {0};
  if (port->mode_enable_ == TRUE &&
      port->svid_enable_ == TRUE)
  {
    id.id_header.modal_op_supported = TRUE;
  }
  else
  {
    id.id_header.modal_op_supported = FALSE;
  }
  id.nack = FALSE;
  id.id_header.usb_vid = USB_VID_SOP;
  id.id_header.product_type_ufp = Product_Type_UFP_SOP;
  if (DPM_CurrentSpecRev(port, sop) > PDSpecRev2p0)
  {
    id.id_header.product_type_dfp = Product_Type_DFP_SOP;
  }
  id.has_product_vdo = TRUE;
  id.cert_stat_vdo.test_id = XID_SOP;
  id.product_vdo.usb_product_id = PID_SOP;
  id.product_vdo.bcd_device = bcdDevice_SOP;

  return id;
}

SvidInfo VdmRequestSvidInfo(struct Port *port)
{
  SvidInfo svid_info = {0};
  if (port->svid_enable_ == TRUE)
  {
    svid_info.nack = FALSE;
    svid_info.num_svids = 1;
    svid_info.svids[0] = port->my_svid_;
  }
  else
  {
    svid_info.nack = TRUE;
    svid_info.num_svids = 0;
    svid_info.svids[0] = 0x0000;
  }
  return svid_info;
}

ModesInfo VdmRequestModesInfo(struct Port *port, FSC_U16 svid)
{
  ModesInfo modes_info = {0};
  if (port->svid_enable_ == TRUE &&
      port->mode_enable_ == TRUE &&
      svid == port->my_svid_) {
    modes_info.nack = FALSE;
    modes_info.svid = svid;
    modes_info.num_modes = 1;
#ifdef FSC_HAVE_DP
    if (svid == DP_SID)
    {
      modes_info.modes[0] = port->display_port_data_.DpCap.word;
    }
    else
#endif
    {
      modes_info.modes[0] = port->my_mode_;
    }
  }
  else {
    modes_info.nack = TRUE;
    modes_info.svid = svid;
    modes_info.num_modes = 0;
    modes_info.modes[0] = 0;
  }
  return modes_info;
}

FSC_BOOL VdmModeEntryRequest(struct Port *port, FSC_U16 svid,
                             FSC_U32 mode_index)
{
  if ((port->svid_enable_ == TRUE) &&
      (port->mode_enable_ == TRUE) &&
      (svid == port->my_svid_))
  {
    if (SVID1_mode1_enter_SOP && mode_index == 1)
    {
      port->mode_entered_ = TRUE;
#ifdef FSC_HAVE_DP
      if (port->my_svid_ == DP_SID)
      {
        port->display_port_data_.DpModeEntered = mode_index;
      }
#endif /*  FSC_HAVE_DP */
      notify_observers(EVENT_MODE_ENTER_SUCCESS, port->port_id_, 0);
      return TRUE;
    }
  }
  return FALSE;
}

FSC_BOOL VdmModeExitRequest(struct Port *port, FSC_U16 svid, FSC_U32 mode_index)
{
  if (port->mode_entered_ == TRUE &&
      svid == port->my_svid_ &&
      mode_index == 1) {
    port->mode_entered_ = FALSE;

#ifdef FSC_HAVE_DP
    if (port->display_port_data_.DpModeEntered &&
        (port->display_port_data_.DpModeEntered == mode_index) &&
        (svid == DP_SID))
    {
      port->display_port_data_.DpModeEntered = 0;
      port->display_port_data_.DpConfigured = FALSE;
      platform_dp_enable_pins(FALSE, 0);
    }
#endif /*  FSC_HAVE_DP */
    notify_observers(EVENT_MODE_EXIT_SUCCESS, port->port_id_, 0);
    return TRUE;
  }
  return FALSE;
}

FSC_BOOL VdmEnterModeResult(struct Port *port, FSC_BOOL success, FSC_U16 svid,
                            FSC_U32 mode_index)
{
  port->auto_mode_entry_pos_ = -1;
  port->mode_entered_ = FALSE;
//  port->display_port_data_.DpModeEntered = 0;
  if (success == TRUE)
  {
    port->mode_entered_ = TRUE;
#ifdef FSC_HAVE_DP
    if (svid == DP_SID)
    {
      port->display_port_data_.DpModeEntered = mode_index;
    }
#endif
    notify_observers(EVENT_MODE_ENTER_SUCCESS, port->port_id_, 0);
    return TRUE;
  }

  return FALSE;
}

void VdmExitModeResult(struct Port *port, FSC_BOOL success, FSC_U16 svid,
                       FSC_U32 mode_index)
{
#ifdef FSC_HAVE_DP
  if (svid == DP_SID &&
      port->display_port_data_.DpModeEntered == mode_index)
  {
    port->display_port_data_.DpModeEntered = 0;
  }
#endif /*  FSC_HAVE_DP */
  if (success == TRUE)
      notify_observers(EVENT_MODE_EXIT_SUCCESS, port->port_id_, 0);
}

void VdmInformIdentity(struct Port *port, FSC_BOOL success, SopType sop,
                       Identity id)
{
  if (sop == SOP_TYPE_SOP1) {
    if (success) {
      DPM_SetSOP1Details(port, success, port->policy_rx_header_.SpecRevision,
                         id.cable_vdo.vbus_current_handling_cap == VBUS_5A);
      notify_observers(EVENT_CBL_IDENTITY_RECEIVED, port->port_id_, 0);
    }
    else {
      DPM_SetSOP1Details(port, success, 0, 0);
    }
  }
  if (sop == SOP_TYPE_SOP)
  {
      if (success == TRUE)
          notify_observers(EVENT_IDENTITY_RECEIVED, port->port_id_, 0);
  }
}

void VdmInformSvids(struct Port *port, FSC_BOOL success, SopType sop,
                    SvidInfo svid_info)
{
  FSC_U32 i;
  /* Reset the known index */
  port->svid_discv_idx_ = -1;
  /* Assume we are are going to be done */
  port->svid_discvry_done_ = TRUE;

  if (success == TRUE)
  {
    port->core_svid_info_.num_svids = svid_info.num_svids;
    for (i = 0; (i < svid_info.num_svids) && (i < MAX_NUM_SVIDS); i++)
    {
      port->core_svid_info_.svids[i] = svid_info.svids[i];
      if (port->core_svid_info_.svids[i] == SVID1_SOP)
      {
        port->svid_discv_idx_ = i;
        break;
      }
    }

    if (port->svid_discv_idx_ < 0 &&
        port->core_svid_info_.num_svids >= MAX_NUM_SVIDS)
    {
      /* Continue discovery as no known svid is found and there are more svids */
      port->svid_discvry_done_ = FALSE;
    }

    notify_observers(EVENT_SVID_RECEIVED, port->port_id_, 0);
  }

  /* If multiple request to svids are required then reset the vdm auto
   * discovery state to request svids here. */
}

void VdmInformModes(struct Port *port, FSC_BOOL success, SopType sop,
                    ModesInfo modes_info)
{
  FSC_U8 i;
  if (success == TRUE && modes_info.nack == FALSE)
  {
#ifdef FSC_HAVE_DP
    /* Evaluate DP mode first if defined */
    if (modes_info.svid == DP_SID)
    {
      for (i = 0; i < modes_info.num_modes; i++)
      {
        if (DP_EvaluateSinkCapability(port, modes_info.modes[i]))
        {
          port->auto_mode_entry_pos_ = i + 1;
          break;
        }
      }
    }
    else
#endif /* FSC_HAVE_DP */
    if (modes_info.svid == SVID_AUTO_ENTRY)
    {
      for (i = 0; i < modes_info.num_modes; i++)
      {
        if (MODE_AUTO_ENTRY == modes_info.modes[i] &&
            port->auto_mode_entry_enabled_ == TRUE)
        {
          port->auto_mode_entry_pos_ = i + 1;
          break;
        }
      }
    }
    notify_observers(EVENT_MODES_RECEIVED, port->port_id_, 0);
  }
}

void VdmInformAttention(struct Port *port, FSC_U16 svid, FSC_U8 mode_index)
{
/*  TODO */
    notify_observers(EVENT_MODE_VDM_ATTENTION, port->port_id_, 0);
}

void VdmInitDpm(struct Port *port)
{
  port->svid_enable_ = (Num_SVIDs_min_SOP > 0) ? TRUE : FALSE;
  port->mode_enable_ = Modal_Operation_Supported_SOP;
  port->my_svid_ = SVID_DEFAULT;
  port->my_mode_  = MODE_DEFAULT;
  port->mode_entered_ = FALSE;
}

/*
 * VdmBitTranslator implementation
 * Functions that convert bits into internal header representations.
 */
VdmType getVdmTypeOf(FSC_U32 in)
{
  UnstructuredVdmHeader vdm_header = getUnstructuredVdmHeader(in);
  return vdm_header.vdm_type;
}

UnstructuredVdmHeader getUnstructuredVdmHeader(FSC_U32 in)
{
  UnstructuredVdmHeader ret;
  ret.svid = (Svid)((in >> 16) & 0xFFFF);
  ret.vdm_type = (VdmType)((in >> 15) & 0x1);
  ret.info = (in >> 0) & 0x7FFF;
  return ret;
}

StructuredVdmHeader getStructuredVdmHeader(FSC_U32 in)
{
  StructuredVdmHeader ret;
  ret.svid = (Svid)((in >> 16) & 0xFFFF);
  ret.vdm_type = (VdmType)((in >> 15) & 0x1);
  ret.svdm_version = (SvdmVersion)((in >> 13) & 0x3);
  ret.obj_pos = (ObjPos)((in >> 8) & 0x7);
  ret.cmd_type = (CmdType)((in >> 6) & 0x3);
  ret.command = (Command)((in >> 0) & 0x1F);
  return ret;
}

IdHeader getIdHeader(FSC_U32 in)
{
  IdHeader ret;
  ret.usb_host_data_capable = (FSC_BOOL)((in >> 31) & 0x1);
  ret.usb_device_data_capable = (FSC_BOOL)((in >> 30) & 0x1);
  ret.product_type_ufp = (ProductType)((in >> 27) & 0x7);
  ret.modal_op_supported = (FSC_BOOL)((in >> 26) & 0x1);
  ret.product_type_dfp = (ProductType)((in >> 23));
  ret.usb_vid = (FSC_U16)((in >> 0) & 0xFFFF);
  return ret;
}

FSC_U32 getBitsForUnstructuredVdmHeader(UnstructuredVdmHeader in)
{
  FSC_U32 ret;
  FSC_U32 tmp;
  ret = 0;
  tmp = in.svid;
  ret |= (tmp << 16);
  tmp = in.vdm_type;
  ret |= (tmp << 15);
  tmp = (in.info << 0);
  ret |= tmp;
  return ret;
}

FSC_U32 getBitsForStructuredVdmHeader(StructuredVdmHeader in)
{
  FSC_U32 ret = 0;
  FSC_U32 tmp = 0;
  tmp = in.svid;
  ret |= (tmp << 16);
  tmp = in.vdm_type;
  ret |= (tmp << 15);
  tmp = in.svdm_version;
  ret |= (tmp << 13);
  tmp = in.obj_pos;
  ret |= (tmp << 8);
  tmp = in.cmd_type;
  ret |= (tmp << 6);
  tmp = in.command;
  ret |= (tmp << 0);
  return ret;
}

FSC_U32 getBitsForIdHeader(IdHeader in)
{
  FSC_U32 ret = 0;
  FSC_U32 tmp = 0;
  tmp = in.usb_host_data_capable;
  ret |= (tmp << 31);
  tmp = in.usb_device_data_capable;
  ret |= (tmp << 30);
  tmp = in.product_type_ufp;
  ret |= (tmp << 27);
  tmp = in.modal_op_supported;
  ret |= (tmp << 26);
  tmp = in.product_type_dfp;
  ret |= (tmp << 23);
  tmp = in.usb_vid;
  ret |= (tmp << 0);
  return ret;
}

/*  Functions that convert bits into internal VDO representations... */
CertStatVdo getCertStatVdo(FSC_U32 in)
{
  CertStatVdo ret;
  ret.test_id = (in >> 0) & 0xFFFFF;
  return ret;
}

ProductVdo getProductVdo(FSC_U32 in)
{
  ProductVdo ret;
  ret.usb_product_id = (in >> 16) & 0xFFFF;
  ret.bcd_device = (in >> 0) & 0xFFFF;
  return ret;
}

CableVdo getCableVdo(FSC_U32 in)
{
  CableVdo ret;
  ret.cable_hw_version = (in >> 28) & 0xF;
  ret.cable_fw_version = (in >> 24) & 0xF;
  ret.cable_to_type = (CableToType)((in >> 18) & 0x3);
  ret.cable_to_pr = (CableToPr)((in >> 17) & 0x1);
  ret.cable_latency = (CableLatency)((in >> 13) & 0xF);
  ret.cable_term = (CableTermType)((in >> 11) & 0x3);
  ret.sstx1_dir_supp = (SsDirectionality)((in >> 10) & 0x1);
  ret.sstx2_dir_supp = (SsDirectionality)((in >> 9) & 0x1);
  ret.ssrx1_dir_supp = (SsDirectionality)((in >> 8) & 0x1);
  ret.ssrx2_dir_supp = (SsDirectionality)((in >> 7) & 0x1);
  ret.vbus_current_handling_cap =
      (VbusCurrentHandlingCapability)((in >> 5) & 0x3);
  ret.vbus_thru_cable = (VbusThruCable)((in >> 4) & 0x1);
  ret.sop2_presence = (Sop2Presence)((in >> 3) & 0x1);
  ret.usb_ss_supp = (UsbSsSupport)((in >> 0) & 0x7);
  return ret;
}

AmaVdo getAmaVdo(FSC_U32 in)
{
  AmaVdo ret;
  ret.cable_hw_version = (in >> 28) & 0xF;
  ret.cable_fw_version = (in >> 24) & 0xF;
  ret.sstx1_dir_supp = (SsDirectionality)((in >> 11) & 0x1);
  ret.sstx2_dir_supp = (SsDirectionality)((in >> 10) & 0x1);
  ret.ssrx1_dir_supp = (SsDirectionality)((in >> 9) & 0x1);
  ret.ssrx2_dir_supp = (SsDirectionality)((in >> 8) & 0x1);
  ret.vconn_full_power = (VConnFullPower)((in >> 5) & 0x7);
  ret.vconn_requirement = (VConnRequirement)((in >> 4) & 0x1);
  ret.vbus_requirement = (VBusRequirement)((in >> 3) & 0x1);
  ret.usb_ss_supp = (AmaUsbSsSupport)((in >> 0) & 0x7);
  return ret;
}

/*  Functions that convert internal VDO representations into bits. */
FSC_U32 getBitsForProductVdo(ProductVdo in)
{
  FSC_U32 ret = 0;
  FSC_U32 tmp = 0;
  tmp = in.usb_product_id;
  ret |= (tmp << 16);
  tmp = in.bcd_device;
  ret |= (tmp << 0);
  return ret;
}

FSC_U32 getBitsForCertStatVdo(CertStatVdo in)
{
  FSC_U32 ret = 0;
  FSC_U32 tmp = 0;
  tmp = in.test_id;
  ret |= (tmp << 0);
  return ret;
}

FSC_U32 getBitsForCableVdo(CableVdo in)
{
  FSC_U32 ret = 0;
  FSC_U32 tmp = 0;
  tmp = in.cable_hw_version;
  ret |= (tmp << 28);
  tmp = in.cable_fw_version;
  ret |= (tmp << 24);
  tmp = in.cable_to_type;
  ret |= (tmp << 18);
  tmp = in.cable_to_pr;
  ret |= (tmp << 17);
  tmp = in.cable_latency;
  ret |= (tmp << 13);
  tmp = in.cable_term;
  ret |= (tmp << 11);
  tmp = in.sstx1_dir_supp;
  ret |= (tmp << 10);
  tmp = in.sstx2_dir_supp;
  ret |= (tmp << 9);
  tmp = in.ssrx1_dir_supp;
  ret |= (tmp << 8);
  tmp = in.ssrx2_dir_supp;
  ret |= (tmp << 7);
  tmp = in.vbus_current_handling_cap;
  ret |= (tmp << 5);
  tmp = in.vbus_thru_cable;
  ret |= (tmp << 4);
  tmp = in.sop2_presence;
  ret |= (tmp << 3);
  tmp = in.usb_ss_supp;
  ret |= (tmp << 0);
  return ret;
}

FSC_U32 getBitsForAmaVdo(AmaVdo in)
{
  FSC_U32 ret = 0;
  FSC_U32 tmp = 0;
  tmp = in.cable_hw_version;
  ret |= (tmp << 28);
  tmp = in.cable_fw_version;
  ret |= (tmp << 24);
  tmp = in.sstx1_dir_supp;
  ret |= (tmp << 11);
  tmp = in.sstx2_dir_supp;
  ret |= (tmp << 10);
  tmp = in.ssrx1_dir_supp;
  ret |= (tmp << 9);
  tmp = in.ssrx2_dir_supp;
  ret |= (tmp << 8);
  tmp = in.vconn_full_power;
  ret |= (tmp << 5);
  tmp = in.vconn_requirement;
  ret |= (tmp << 4);
  tmp = in.vbus_requirement;
  ret |= (tmp << 3);
  tmp = in.usb_ss_supp;
  ret |= (tmp << 0);
  return ret;
}

#endif /*  FSC_HAVE_VDM */
