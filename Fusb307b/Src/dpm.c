/*******************************************************************************
 * @file     dpm.c
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
 * dpm.c
 *
 * Implements the Device Policy Manager state machine functions.
 */

#include "dpm.h"

#include "port.h"
#include "timer.h"
#include "typec.h"
#include "vdm.h"
#include "vendor_info.h"

void DPM_Initialize(struct Port *port)
{
  port->dpm_active_ = FALSE;

  DPM_Reset(port);
}

void DPM_Reset(struct Port *port)
{
    port->dpm_supply_ready_ = FALSE;
    port->dpm_alert_ = FALSE;
    port->dpm_reject_count_ = 0;
    port->dpm_initial_connected_ = TRUE;
    port->dpm_5a_possible_ = FALSE;
    port->dpm_5a_capsent_ = FALSE;
    port->dpm_disc_id_returned_ = FALSE;
    port->dpm_disc_id_done_ = FALSE;
    port->dpm_disc_id_count_ = 0;
    port->dpm_first_attach_ = TRUE;
    port->dpm_pd_30_ = (port->pd_preferred_rev_ == PDSpecRev3p0) ? TRUE : FALSE;
    port->dpm_pd_30_srccab_ = (port->pd_preferred_rev_ == PDSpecRev3p0) ? TRUE : FALSE;;
    port->dpm_pd_20_cabchk_ = FALSE;
#ifdef FSC_HAVE_VDM
    port->vdm_cbl_present_ = FALSE;
    port->vdm_check_cbl_ = DPM_IsSOPPAllowed(port);
#endif /* FSC_HAVE_VDM */
}

void DPM_PrepareSrcCaps(struct Port *port)
{
  FSC_U8 i;

  for (i = 0; i < port->caps_header_source_.NumDataObjects; ++i) {
    /* 3A or 5A adjustments to FPDO objects only */
    if (port->caps_source_[i].PDO.SupplyType == pdoTypeFixed)
    {
      if (port->dpm_5a_possible_)
      {
          /* Cap the max current to that supported by cable */
          if (port->caps_source_[i].FPDOSupply.MaxCurrent > 500)
          {
              port->caps_source_[i].FPDOSupply.MaxCurrent = 500;  /* 5A */
          }
      }
      else
      {
          /* Cap the max current to that supported by cable */
          if (port->caps_source_[i].FPDOSupply.MaxCurrent > 300)
          {
              port->caps_source_[i].FPDOSupply.MaxCurrent = 300;  /* 3A */
          }
      }
    }
    if (port->dpm_pd_30_) {
      /* TODO Add PPS caps if available */
    }
  }

  port->dpm_src_caps_ready_ = TRUE;
}

FSC_BOOL DPM_EvaluateRequest(struct Port *port)
{
  FSC_BOOL reqOK = FALSE;
  FSC_U16 iRequest, iSupply, vSupplyMax, vSupplyMin, vRequest;
  FSC_U8 obj_position = 0;

  if (port->dpm_first_attach_ == TRUE) {
    /* Update partner spec revision here */
    DPM_SetSOPVersion(port, port->policy_rx_header_.SpecRevision);
  }

  port->source_is_apdo_ = FALSE;

  obj_position = port->policy_rx_data_obj_[0].FVRDO.ObjectPosition;

  if ((obj_position > 0) &&
      (obj_position <= port->caps_header_source_.NumDataObjects)) {
    switch (port->caps_source_[obj_position - 1].FPDOSupply.SupplyType) {
    case pdoTypeFixed:
      if (port->policy_rx_data_obj_[0].FVRDO.OpCurrent <=
          port->caps_source_[obj_position - 1].FPDOSupply.MaxCurrent) {
        reqOK = TRUE;
      }
      break;
    case pdoTypeAugmented:
      switch (port->caps_source_[obj_position-1].APDO.APDOType) {
      case apdoTypePPS:
        iRequest = port->policy_rx_data_obj_[0].PPSRDO.OpCurrent; /* 50mA */
        vRequest = port->policy_rx_data_obj_[0].PPSRDO.OpVoltage; /* 20mV */

        /* 50mA units */
        iSupply = port->caps_source_[obj_position - 1].PPSAPDO.MaxCurrent;

        /* Convert caps voltage (100mV) to 20mV */
        vSupplyMax =
            port->caps_source_[obj_position - 1].PPSAPDO.MaxVoltage * 5;
        vSupplyMin =
            port->caps_source_[obj_position - 1].PPSAPDO.MinVoltage * 5;

        if ((iRequest <= iSupply) &&
            (vRequest <= vSupplyMax) &&
            (vRequest >= vSupplyMin)) {
          reqOK = TRUE;
          port->source_is_apdo_ = TRUE;
          port->stored_apdo_.object =
              port->caps_source_[obj_position - 1].object;
        }
        break;
      /* Add appropriate cases here for other APDO's */
      default:
        break;
      } /* switch APDOType */
    } /* switch SupplyType */
  } /* if valid position */

  // TODO - ALSO CHECK CURRENT CONTRACT AND UPDATE IS_CONTRACT_VALID_

  return reqOK;
}

void DPM_Status(struct Port *port)
{
  // TODO - UPDATE STATUS MSG DATA
  set_policy_state(port, PE_SRC_Give_Source_Status);
}

void DPM_PPSStatus(struct Port *port)
{
  // TODO - UPDATE PPS STATUS MSG DATA
  set_policy_state(port, PE_SRC_Give_PPS_Status);
}

void DPM_AlertStatus(struct Port *port)
{
  // TODO - UPDATE ALERT MSG STATUS
  set_policy_state(port, PE_SRC_Send_Source_Alert);
  port->dpm_alert_ = FALSE;
}

void DPM_ExtendedCaps(struct Port *port)
{
  // TODO - UPDATE EXT CAPS BITS
  set_policy_state(port, PE_SRC_Give_Source_Cap_Ext);
}

void DPM_DiscoverVersion(struct Port *port)
{
  /* Only one case where we need to update a revision value. */
  if (port->dpm_pd_30_ == FALSE && port->dpm_pd_30_srccab_ == TRUE) {
    port->dpm_pd_30_srccab_ = FALSE;
  }

  port->dpm_first_attach_ = FALSE;
  set_dpm_state(port, dpmPDConnected);
}

void set_dpm_state(struct Port *port, DPMState_t state)
{
}

void DPM_PrepareStatus(struct Port *port)
{

}

void DPM_PreparePPSStatus(struct Port *port)
{

}

void DPM_PrepareExtendedCaps(struct Port *port)
{

}

FSC_BOOL DPM_VCS_Allowed(struct Port *port)
{
  return port->is_vconn_source_;
}

void DPM_SetSOPVersion(struct Port *port, FSC_U8 ver)
{
  /* Catch a compliance trick where they send 4p0... */
  if (ver > PDSpecRev3p0) {
      ver = PDSpecRev3p0;
  }

  if (ver != PDSpecRev3p0) {
    port->dpm_pd_30_ = FALSE;
    port->dpm_pd_30_srccab_ = FALSE;
  }
}

void DPM_SetSOP1Details(struct Port *port,
                        FSC_BOOL ack, FSC_U8 ver, FSC_BOOL is_5a_capable)
{
  port->dpm_disc_id_returned_ = TRUE;
  port->dpm_disc_id_result_ = ack;

  if (ack) {
    if (ver != PDSpecRev3p0)
      port->dpm_pd_30_srccab_ = FALSE;

    port->dpm_5a_possible_ = is_5a_capable;
  }
}

FSC_U8 DPM_CurrentSpecRev(struct Port *port, SopType sop)
{
    if (sop == SOP_TYPE_SOP) {
      if (port->dpm_pd_30_) {
        return PDSpecRev3p0;
      }
      else {
          return PDSpecRev2p0;
      }
    }
    else if (sop == SOP_TYPE_SOP1 || sop == SOP_TYPE_SOP2) {
      if (port->dpm_pd_30_srccab_) {
          return PDSpecRev3p0;
      }
      else {
          return PDSpecRev2p0;
      }
    }
    else {
        return PDSpecRev2p0;
    }
}
/**
 * @brief Returns true if allowed to talk to cable
 */
FSC_BOOL DPM_IsSOPPAllowed(struct Port *port)
{
  if (SOP_P_Capable == FALSE) {
      return FALSE;
  }

  if (DPM_CurrentSpecRev(port, SOP_TYPE_SOP) == PDSpecRev3p0 &&
        port->is_vconn_source_) {
    /* PD 3.0 requires only vconn source to communicate with sop' and sop''*/
      return TRUE;
    }
    else if (DPM_CurrentSpecRev(port, SOP_TYPE_SOP) == PDSpecRev2p0 &&
        port->policy_is_dfp_) {
      /* PD 2.0 dfp communicates with SOP' and SOP'' */
      return TRUE;
    }

  return FALSE;
}

void DPM_ReConfigureRxDetect(struct Port *port)
{

  if (DPM_IsSOPPAllowed(port)) {
    set_sop_p_detect(port, TRUE);
#ifdef FSC_HAVE_VDM
    port->discover_id_counter_ = 0;
#endif /* FSC_HAVE_VDM */
  }
  else {
    set_sop_p_detect(port, FALSE);
  }
}

FSC_U8 DPM_Retries(struct Port *port, SopType sop)
{
  FSC_U8 rev = DPM_CurrentSpecRev(port, sop);

    return (rev == PDSpecRev3p0) ? 2 : 3;
}
