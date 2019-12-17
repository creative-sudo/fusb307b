/*******************************************************************************
 * @file     hostcomm.c
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
#ifdef FSC_HAVE_USBHID

#include "version.h"
#include "hostcomm.h"

#include "local_platform.h"
#include "port.h"
#include "typec.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_i2c.h"
#include "dpm.h"

#ifdef FSC_HAVE_VDM
#include "vdm.h"
#ifdef FSC_HAVE_DP
#include "display_port.h"
#endif /* FSC_HAVE_DP */
#endif /* FSC_HAVE_VDM */

/*******************************************************************************
 * Macros for HostCom
 ******************************************************************************/
#define PTR_DIFF(ptr1, ptr2)                (((FSC_U8*)ptr1) - ((FSC_U8*)ptr2))
#define SET_DEFAULT_VAL(expr, val, def)     (expr == val) ? (def) : (expr)

/* Sets the value to 1 if expr is zero */
#define ONE_IF_ZERO(expr)                   SET_DEFAULT_VAL(expr, 0, 1)

/* Wraps index if it goes outside limit */
#define SAFE_INDEX(val, limit)              (val % limit)

#define WRITE_INT(start, val)\
    ({\
        ((FSC_U8*)(start))[0] = val & 0xFF;\
        ((FSC_U8*)(start))[1] = (val & 0xFF00) >> 8;\
        ((FSC_U8*)(start))[2] = (val & 0xFF0000) >> 16;\
        ((FSC_U8*)(start))[3] = (val & 0xFF000000) >> 24;\
        sizeof(FSC_U32);\
    })

#define WRITE_SHORT(start, val)\
    ({\
        ((FSC_U8*)(start))[0] = val & 0xFF;\
        ((FSC_U8*)(start))[1] = (val & 0xFF00) >> 8;\
        sizeof(FSC_U16);\
    })

#define READ_WORD(start)\
    ({\
        FSC_U32 x;\
        x = ((FSC_U8*)start)[0] | ((FSC_U8*)start)[1] << 8 \
        | ((FSC_U8*)start)[2] << 16 | ((FSC_U8*)start)[3] << 24;\
        x;\
     })

#define READ_SHORT(start)\
    ({\
        FSC_U16 x;\
        x = ((FSC_U8*)start)[0] | ((FSC_U8*)start)[1] << 8;\
        x;\
     })

#define HCOM_MEM_FILL(ptr, val, len)\
    ({\
       FSC_U32 i = 0; \
       FSC_U8 *p = (FSC_U8 *)ptr;\
       while (i++ < len) { *p++ = 0; }\
    })

/**
 * Some structure for easier reading of pd request and response
 */
struct pd_buf {
    FSC_U8 id;
    FSC_U8 val;
};

static int ReadSinkCapabilities(FSC_U8 *data, FSC_S32 bufLen, struct Port *port)
{
    FSC_U32 i = 0, j = 0;
    FSC_U32 index = 0;
    int len = port->caps_header_sink_.NumDataObjects * 4;

    if (bufLen < len)
    {
        return 0;
    }

    data[index++] = port->caps_header_sink_.byte[0];
    data[index++] = port->caps_header_sink_.byte[1];

    for (i = 0; i < port->caps_header_sink_.NumDataObjects; i++)
    {
        for (j = 0; j < 4; j++)
        {
            data[index++] = port->caps_sink_[i].byte[j];
        }
    }

    return index;
}

static int ReadSourceCapabilities(FSC_U8 *data, FSC_S32 bufLen, struct Port *port)
{
    FSC_U32 i = 0, j = 0;
    FSC_U32 index = 0;
    int len = port->caps_header_source_.NumDataObjects * 4;

    if (bufLen < len)
    {
        return 0;
    }

    data[index++] = port->caps_header_source_.byte[0];
    data[index++] = port->caps_header_source_.byte[1];

    for (i = 0; i < port->caps_header_source_.NumDataObjects; i++)
    {
        for (j = 0; j < 4; j++)
        {
            data[index++] = port->caps_source_[i].byte[j];
        }
    }

    return index;
}

static int ReadReceivedCaps(FSC_U8 *data, FSC_S32 bufLen, struct Port *port)
{
    FSC_U32 i = 0, j = 0;
    FSC_U32 index = 0;
    int len = port->caps_header_received_.NumDataObjects * 4;

    if (bufLen < len) {
        return 0;
    }

    data[index++] = port->caps_header_received_.byte[0];
    data[index++] = port->caps_header_received_.byte[1];

    for (i = 0; i < port->caps_header_received_.NumDataObjects; i++)
    {
        for (j = 0; j < 4; j++)
        {
            data[index++] = port->caps_received_[i].byte[j];
        }
    }

    return index;
}

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
static void WriteSourceCapabilities(FSC_U8 *data, struct Port *port)
{
    FSC_U32 i = 0, j = 0;
    sopMainHeader_t header = { 0 };

    header.byte[0] = *data++;
    header.byte[1] = *data++;

    /* Only do anything if we decoded a source capabilities message */
    if ((header.NumDataObjects > 0)
            && (header.MessageType == DMTSourceCapabilities))
    {
        port->caps_header_source_.word = header.word;

        for (i = 0; i < port->caps_header_source_.NumDataObjects; i++)
        {
            for (j = 0; j < 4; j++)
            {
                port->caps_source_[i].byte[j] = *data++;
            }
        }

        if (port->policy_is_source_)
        {
            port->pd_transmit_header_.word = port->caps_header_source_.word;
            port->pd_tx_flag_ = TRUE;
            port->source_caps_updated_ = TRUE;
            /* Wake up the port if idle */
            port->idle_ = FALSE;
        }
    }
}

#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
static void WriteSinkCapabilities(FSC_U8 *data, struct Port *port)
{
    FSC_U32 i = 0, j = 0;
    sopMainHeader_t header = { 0 };

    header.byte[0] = *data++;
    header.byte[1] = *data++;

    /* Only do anything if we decoded a sink capabilities message */
    if ((header.NumDataObjects > 0)
            && (header.MessageType == DMTSinkCapabilities))
    {
        port->caps_header_sink_.word = header.word;
        for (i = 0; i < port->caps_header_sink_.NumDataObjects; i++)
        {
            for (j = 0; j < 4; j++)
            {
                port->caps_sink_[i].byte[j] = *data++;
            }
        }
        /* We could also trigger sending the caps or re-evaluating,
         * but we don't do anything with this info here...
         */
    }
}
#endif

static void SendUSBPDMessage(FSC_U8 *data, struct Port *port)
{
    FSC_U32 i = 0, j = 0;

    /* First byte is sop */
    port->policy_msg_tx_sop_ = *data++;

    /* 2 header bytes */
    port->pd_transmit_header_.byte[0] = *data++;
    port->pd_transmit_header_.byte[1] = *data++;

    /* Data objects */
    for (i = 0; i < port->pd_transmit_header_.NumDataObjects; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            port->pd_transmit_objects_[i].byte[j] = *data++;
        }
    }

    port->pd_tx_flag_ = TRUE;
    /* Wake up the port if idle */
    port->idle_ = FALSE;
}

#ifdef FSC_HAVE_VDM
/* Svid 1 buffer layout */
struct svid1_buf
{
    FSC_U8 svid[2];
    FSC_U8 num_modes;
    FSC_U8 modes[4];
};

struct vdm_mode_buf
{
    FSC_U8 num_svid;
    struct svid1_buf svid1;
/* Add more if needed */
};

static FSC_U8 ReadVdmModes(struct Port *port, FSC_U8 *buf, FSC_U32 len)
{
    struct vdm_mode_buf *vdm_obj = (struct vdm_mode_buf*) buf;
    /* Check if buffer has enough size */
    if (len < (sizeof(struct vdm_mode_buf)))
    {
        return 0;
    }
    /* Assuming that number of svids and modes is 1. If not change
     * the following code to write all values in buffer and use
     * constants from vendor_info. Incorrect number might cause
     * GUI to crash as it tries to read all the svids.
     * Num_SVIDs_max_SOP = 1
     */
    vdm_obj->num_svid = 1;
    /* Iterate through each svid if needed. Assuming we only have one */
    WRITE_SHORT(vdm_obj->svid1.svid, port->my_svid_);
    vdm_obj->svid1.num_modes = 1;
    WRITE_INT(vdm_obj->svid1.modes, port->my_mode_);
    return sizeof(struct vdm_mode_buf);
}

static void WriteVdmModes(struct Port *port, FSC_U8 *buf)
{
    struct vdm_mode_buf *vdm_obj = (struct vdm_mode_buf*) buf;
    if (vdm_obj->num_svid > 0)
    {
        port->my_svid_ = READ_SHORT(vdm_obj->svid1.svid);
        port->my_mode_ = READ_WORD(vdm_obj->svid1.modes);
    }
}
#endif /* FSC_HAVE_VDM */

static void ProcessTCSetState(TypeCState state, struct Port *port)
{
    switch (state)
    {
    case (Disabled):
        SetStateDisabled(port);
        break;
    case (ErrorRecovery):
        SetStateErrorRecovery(port);
        break;
    case (Unattached):
        SetStateUnattached(port);
        break;
#ifdef FSC_HAVE_SNK
    case (AttachWaitSink):
        SetStateAttachWaitSink(port);
        break;
    case (AttachedSink):
        SetStateAttachedSink(port);
        break;
#ifdef FSC_HAVE_DRP
    case (TryWaitSink):
        SetStateTryWaitSink(port);
        break;
    case (TrySink):
        SetStateTrySink(port);
        break;
#endif /* FSC_HAVE_DRP */
#endif /* FSC_HAVE_SNK */
#ifdef FSC_HAVE_SRC
    case(AttachWaitSource):
      SetStateAttachWaitSource(port);
      break;
    case(AttachedSource):
      SetStateAttachedSource(port);
      break;
    case(UnattachedWaitSource):
      SetStateUnattachedWaitSource(port);
      break;
#ifdef FSC_HAVE_DRP
    case (TrySource):
        SetStateTrySource(port);
        break;
    case (TryWaitSource):
        SetStateTryWaitSource(port);
        break;
#endif /* FSC_HAVE_DRP */
#endif /* FSC_HAVE_SRC */
#ifdef FSC_HAVE_ACC
    case(AudioAccessory):
      SetStateAudioAccessory(port);
      break;
#ifdef FSC_HAVE_SRC
    case(UnorientedDebugAccessorySource):
      SetStateUnorientedDebugAccessorySource(port);
      break;
    case(OrientedDebugAccessorySource):
      SetStateOrientedDebugAccessorySource(port);
      break;
#endif /* FSC_HAVE_SRC */
    case(DebugAccessorySink):
      SetStateDebugAccessorySink(port);
      break;
    case(AttachWaitAccessory):
      SetStateAttachWaitAccessory(port);
      break;
    case(PoweredAccessory):
      SetStatePoweredAccessory(port);
      break;
    case(UnsupportedAccessory):
      SetStateUnsupportedAccessory(port);
      break;
#endif /* FSC_HAVE_ACC */
    default:
        SetStateUnattached(port);
        break;
    }
}

static FSC_BOOL ProcessTCWrite(FSC_U8 *subCmds, struct Port *port)
{
    FSC_BOOL setUnattached = FALSE;
    FSC_U32 i;
    FSC_S32 len;
    struct pd_buf *pCmd = (struct pd_buf*) subCmds;

    for (i = 0; i < HCMD_PAYLOAD_SIZE / 2; i++)
    {
        len = PTR_DIFF(&subCmds[HCMD_PAYLOAD_SIZE], pCmd);
        if (len < sizeof(struct pd_buf))
        {
            goto end_of_cmd;
        }
        len -= sizeof(struct pd_buf);
        switch (pCmd->id)
        {
        case TYPEC_ENABLE:
            if (pCmd->val != port->tc_enabled_)
                port->tc_enabled_ = (pCmd->val) ? TRUE : FALSE;
            break;
        case TYPEC_PORT_TYPE:
            if (pCmd->val < USBTypeC_UNDEFINED
                    && pCmd->val != port->port_type_) {
                port->port_type_ = pCmd->val;
                port->caps_source_[0].FPDOSupply.DualRolePower =
                    (port->port_type_ == USBTypeC_DRP) ? TRUE : FALSE;
                setUnattached = TRUE;
            }
            break;
        case TYPEC_ACC_SUPPORT:
            if (pCmd->val != port->acc_support_) {
                port->acc_support_ = (pCmd->val) ? TRUE : FALSE;
                setUnattached = TRUE;
            }
            break;
        case TYPEC_SRC_PREF:
            if (pCmd->val != port->src_preferred_) {
                port->src_preferred_ = (pCmd->val) ? TRUE : FALSE;
                setUnattached = TRUE;
            }
            break;
        case TYPEC_SNK_PREF:
            if (pCmd->val != port->snk_preferred_) {
                port->snk_preferred_ = (pCmd->val) ? TRUE : FALSE;
                setUnattached = TRUE;
            }
            break;
        case TYPEC_STATE:
            ProcessTCSetState(pCmd->val, port);
            break;
        case TYPEC_DFP_CURRENT_AD:
            if (pCmd->val != port->src_current_ && pCmd->val < utccInvalid) {
                port->src_current_ = pCmd->val;
                UpdateSourceCurrent(port, port->src_current_);
            }
            break;
#ifdef FSC_HAVE_FRSWAP
        case TYPEC_FRS_MODE:
            if (pCmd->val == FRS_Hub_Sink) {
                EnableFRS_HubInitialSink(port);
            }
            else if (pCmd->val == FRS_Hub_Source) {
                EnableFRS_HubInitialSource(port);
            }
            else if (pCmd->val == FRS_Host_Sink) {
                EnableFRS_HostInitialSink(port);
            }
            break;
#endif /* FSC_HAVE_FRSWAP */
        case TYPEC_EOP:
        default:
            goto end_of_cmd;
            break;
        }

        pCmd++;
    }

end_of_cmd:
  /** set unattached if command requires for transition */
  if (setUnattached) {
      SetStateUnattached(port);
      setUnattached = FALSE;
  }

  return TRUE;
}

static FSC_BOOL ProcessTCRead(FSC_U8 *subCmds, FSC_U8 *outBuf, struct Port *port)
{
    FSC_U32 i;
    FSC_S32 len;

    struct pd_buf *pRsp = (struct pd_buf*) outBuf;

    /* limit it to 30 commands because the output requires
     * twice the size for each command */
    for (i = 0; i < HCMD_PAYLOAD_SIZE / 2; i++)
    {
        len = PTR_DIFF(&outBuf[HCMD_PAYLOAD_SIZE], pRsp);
        if (len < sizeof(struct pd_buf))
        {
            if (subCmds[i] == TYPEC_EOP)
            {
                goto end_of_cmd;
            }
            /* at least 2 bytes needed */
            goto out_buf_full;
        }
        pRsp->id = subCmds[i];
        /* Bytes that additional info can use */
        len -= sizeof(struct pd_buf);
        switch (pRsp->id)
        {
        case TYPEC_ENABLE:
            pRsp->val = port->tc_enabled_;
            break;
        case TYPEC_PORT_TYPE:
            pRsp->val = port->port_type_;
            break;
        case TYPEC_ACC_SUPPORT:
            pRsp->val = port->acc_support_;
            break;
        case TYPEC_SRC_PREF:
            pRsp->val = port->src_preferred_;
            break;
        case TYPEC_SNK_PREF:
            pRsp->val = port->snk_preferred_;
            break;
        case TYPEC_STATE:
            pRsp->val = port->tc_state_;
            break;
        case TYPEC_SUBSTATE:
            pRsp->val = port->tc_substate_;
            break;
        case TYPEC_CC_ORIENT:
            pRsp->val = port->cc_pin_;
            break;
        case TYPEC_CC_TERM:
            pRsp->val = port->cc_term_;
            break;
        case TYPEC_VCON_TERM:
            pRsp->val = port->vconn_term_;
            break;
        case TYPEC_DFP_CURRENT_AD:
            pRsp->val = port->src_current_;
            break;
        case TYPEC_UFP_CURRENT:
            pRsp->val = port->snk_current_;
            break;
        case TYPEC_STATE_LOG:
            #ifdef FSC_LOGGING
            pRsp->val = ReadTCLog(&port->log_, &pRsp->val + 1, len);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            #endif
            break;
        case TYPEC_FRS_MODE:
            pRsp->val = FRS_None;
            break;
        case TYPEC_EOP:
        default:
            pRsp->id = TYPEC_EOP;
            goto end_of_cmd;
            break;
        }
        pRsp++;
    }

end_of_cmd:
    return TRUE;
out_buf_full:
    return FALSE;
}

static void ProcessTCStatus(HostCmd_t *inCmd, HostCmd_t *outMsg, struct Port *port)
{
    int read = (inCmd->typeC.cmd.req.rw == 0) ? TRUE : FALSE;
    int status;

    if (read)
    {
        status = ProcessTCRead(inCmd->typeC.cmd.req.payload,
                outMsg->typeC.cmd.rsp.payload, port);
    } else
    {
        status = ProcessTCWrite(inCmd->typeC.cmd.req.payload, port);
    }

    if (status == TRUE)
    {
        outMsg->typeC.cmd.rsp.error = HCMD_STATUS_SUCCESS;
    } else {
        outMsg->typeC.cmd.rsp.error = HCMD_STATUS_FAILED;
    }
}

static FSC_BOOL ProcessPDRead(FSC_U8 *subCmds, FSC_U8 *outBuf, struct Port *port)
{
    FSC_S32 len;
    FSC_U32 i;

    struct pd_buf *pRsp = (struct pd_buf*) outBuf;

      for (i = 0; i < HCMD_PAYLOAD_SIZE / 2; i++)
      {
          len = PTR_DIFF(&outBuf[HCMD_PAYLOAD_SIZE], pRsp);
          if (len < sizeof(struct pd_buf))
          {
              /* Buffer is full */
              if (subCmds[i] == USBPD_EOP)
              {
                /* Done with commands, so full buffer is OK */
                  goto end_of_cmd;
              }
              else
              {
                /* At least 2 bytes needed to continue */
                  goto out_buf_full;
              }
        }
        pRsp->id = subCmds[i];
        switch (pRsp->id) {
        case PD_ENABLE:
            pRsp->val = port->pd_enabled_;
            break;
        case PD_ACTIVE:
            pRsp->val = port->pd_active_;
            break;
        case PD_HAS_CONTRACT:
            pRsp->val = port->policy_has_contract_;
            break;
        case PD_SPEC_REV:
            pRsp->val = port->pd_preferred_rev_;
            break;
        case PD_PWR_ROLE:
            pRsp->val = port->policy_is_source_;
            break;
        case PD_DATA_ROLE:
            pRsp->val = port->policy_is_dfp_;
            break;
        case PD_VCON_SRC:
            pRsp->val = port->is_vconn_source_;
            break;
        case PD_STATE:
            pRsp->val = port->policy_state_;
            break;
        case PD_SUB_STATE:
            pRsp->val = port->policy_subindex_;
            break;
        case PD_PROTOCOL_STATE:
            pRsp->val = port->protocol_state_;
            break;
        case PD_PROTOCOL_SUB_STATE:
            pRsp->val = port->policy_subindex_;
            break;
        case PD_TX_STATUS:
            pRsp->val = port->pd_tx_status_;
            break;
        case PD_CAPS_CHANGED:
            pRsp->val = port->source_caps_updated_;
            port->source_caps_updated_ = FALSE;
            break;
        case PD_GOTOMIN_COMPAT:
            pRsp->val =port->sink_goto_min_compatible_;
            break;
        case PD_USB_SUSPEND:
            pRsp->val = port->sink_usb_suspend_compatible_;
            break;
        case PD_COM_CAPABLE:
            pRsp->val = port->sink_usb_comm_capable_;
            break;
#ifdef FSC_HAVE_SRC
        case PD_SRC_CAP:
            pRsp->val = ReadSourceCapabilities(&pRsp->val + 1, len, port);
            pRsp = (struct pd_buf*) ((FSC_U8*) pRsp + pRsp->val);
            break;
#endif ///< FSC_HAVE_SRC
#ifdef FSC_HAVE_SNK
        case PD_SNK_CAP:
            pRsp->val = ReadSinkCapabilities(&pRsp->val + 1, len, port);
            pRsp = (struct pd_buf*) ((FSC_U8*) pRsp + pRsp->val);
            break;
#endif ///< FSC_HAVE_SNK
#ifdef FSC_LOGGING
        case PD_PD_LOG:
            pRsp->val = ReadPDLog(&port->log_, &pRsp->val + 1, len);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        case PD_PE_LOG:
            pRsp->val = ReadPELog(&port->log_, &pRsp->val + 1, len);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
#endif	///< FSC_LOGGING
        case PD_MAX_VOLTAGE:
            pRsp->val = WRITE_INT((&pRsp->val + 1), port->sink_request_max_voltage_ / 10);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        case PD_OP_POWER:
            pRsp->val = WRITE_INT((&pRsp->val + 1), port->sink_request_op_power_ / 10);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        case PD_MAX_POWER:
            pRsp->val = WRITE_INT((&pRsp->val + 1), port->sink_request_max_power_ / 10);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        case PD_SNK_TX_STATUS:
            break;
        case PD_PPR_SRC_CAP:
        case PD_PPR_SINK_CAP:
            pRsp->val = ReadReceivedCaps(&pRsp->val + 1, len, port);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
#ifdef FSC_HAVE_VDM
        case PD_VDM_MODES:
        {
            pRsp->val = ReadVdmModes(port, &pRsp->val + 1, len);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        }
        case PD_SVID_ENABLE:
            pRsp->val = port->svid_enable_;
            break;
        case PD_MODE_ENABLE:
            pRsp->val = port->mode_enable_;
            break;
        case PD_SVID_AUTO_ENTRY:
            pRsp->val = port->auto_mode_entry_enabled_;
            /* May not be required */
            break;
#endif /* FSC_HAVE_VDM */
        case USBPD_EOP:
        default:
            pRsp->id = USBPD_EOP;
            goto end_of_cmd;
            break;
        }
        pRsp++;
    }

end_of_cmd:
    return TRUE;
out_buf_full:
    return FALSE;
}

static FSC_BOOL ProcessPDWrite(FSC_U8 *subCmds, struct Port *port)
{

    FSC_BOOL setUnattached = FALSE;
    struct pd_buf *pCmd = (struct pd_buf*) subCmds;
    FSC_U32 i;
    FSC_S32 len;

    for (i = 0; i < HCMD_PAYLOAD_SIZE / 2; i++)
    {
        len = PTR_DIFF(&subCmds[HCMD_PAYLOAD_SIZE], pCmd);
        if (len < sizeof(struct pd_buf))
        {
            goto end_of_cmd;
        }
        len -= sizeof(struct pd_buf);
        switch (pCmd->id)
        {
        case PD_ENABLE:
            if (pCmd->val != port->pd_enabled_)
            {
                port->pd_enabled_ = (pCmd->val == 0) ? FALSE : TRUE;
            }
            break;
        case PD_SPEC_REV:
            if (port->pd_preferred_rev_ != pCmd->val)
            {
              port->pd_preferred_rev_ = pCmd->val;
              setUnattached = TRUE;
            }
            break;
        case PD_GOTOMIN_COMPAT:
            port->sink_goto_min_compatible_ = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case PD_USB_SUSPEND:
            port->sink_usb_suspend_compatible_ = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case PD_COM_CAPABLE:
            port->sink_usb_comm_capable_ = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case PD_MAX_VOLTAGE:
            port->sink_request_max_voltage_ = READ_WORD((&pCmd->val + 1)) * 10;
            pCmd = (struct pd_buf*)((FSC_U8*)pCmd + pCmd->val);
            break;
        case PD_OP_POWER:
            port->sink_request_op_power_ = READ_WORD((&pCmd->val + 1)) * 10;
            pCmd = (struct pd_buf*)((FSC_U8*)pCmd + pCmd->val);
            break;
        case PD_MAX_POWER:
            port->sink_request_max_power_ = READ_WORD((&pCmd->val + 1)) * 10;
            pCmd = (struct pd_buf*)((FSC_U8*)pCmd + pCmd->val);
            break;
        case PD_HARD_RESET:
           /* Wake up the port */
           port->idle_ = FALSE;
           if (port->policy_is_source_)
                set_policy_state(port, PE_SRC_Hard_Reset);
            else
                set_policy_state(port, PE_SNK_Hard_Reset);
            break;
#ifdef FSC_HAVE_SRC
        case PD_SRC_CAP:
            WriteSourceCapabilities(&pCmd->val+1, port);
            pCmd = (struct pd_buf*)((FSC_U8*)pCmd + pCmd->val);
            break;
#endif ///< FSC_HAVE_SRC
#ifdef FSC_HAVE_SNK
        case PD_SNK_CAP:
            WriteSinkCapabilities(&pCmd->val+1, port);
            pCmd = (struct pd_buf*)((FSC_U8*)pCmd + pCmd->val);
            break;
#endif ///< FSC_HAVE SNK
        case PD_MSG_WRITE:
            SendUSBPDMessage(&pCmd->val+1, port);
            pCmd = (struct pd_buf*)((FSC_U8*)pCmd + pCmd->val);
            break;
#ifdef FSC_HAVE_VDM
        case PD_SVID_ENABLE:
            port->svid_enable_ = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case PD_MODE_ENABLE:
            port->mode_enable_ = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case PD_SVID_AUTO_ENTRY:
            port->auto_mode_entry_enabled_  = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case PD_VDM_MODES:
            if (len < sizeof(struct vdm_mode_buf)) { goto end_of_cmd; }
            WriteVdmModes(port, &pCmd->val + 1);
            pCmd = (struct pd_buf*) ((FSC_U8*) pCmd + pCmd->val);
            break;
        case PD_CABLE_RESET:
            port->cbl_rst_state_ = CBL_RST_START;
            port->idle_ = FALSE;
            break;
#endif  /* FSC_HAVE_VDM */
        case USBPD_EOP:
        default:
            goto end_of_cmd;
            break;
        }
        pCmd++;
    }

end_of_cmd:
    /** set unattached if command requires for transition */
    if (setUnattached)
    {
        SetStateUnattached(port);
        setUnattached = FALSE;
    }
    return TRUE;
}

static void ProcessPDStatus(HostCmd_t *inCmd, HostCmd_t *outMsg, struct Port *port)
{
    int read = (inCmd->pd.cmd.req.rw == 0) ? TRUE : FALSE;
    int status;

    if (read == TRUE)
    {
        status = ProcessPDRead(inCmd->pd.cmd.req.payload,
                outMsg->pd.cmd.rsp.payload, port);
    } else
    {
        status = ProcessPDWrite(inCmd->pd.cmd.req.payload, port);
    }

    if (status == TRUE)
    {
        outMsg->typeC.cmd.rsp.error = HCMD_STATUS_SUCCESS;
    } else {
        outMsg->typeC.cmd.rsp.error = HCMD_STATUS_FAILED;
    }
}

#ifdef FSC_HAVE_DP
static FSC_BOOL ProcessDPWrite(FSC_U8 *subCmds, struct Port *port)
{
    FSC_U32 i, len;
    struct pd_buf *pCmd = (struct pd_buf*) subCmds;

    for (i = 0; i < HCMD_PAYLOAD_SIZE / 2; i++)
    {
        len = PTR_DIFF(&subCmds[HCMD_PAYLOAD_SIZE], pCmd);
        if (len < sizeof(struct pd_buf))
        {
            goto end_of_cmd;
        }
        len -= sizeof(struct pd_buf);
        switch (pCmd->id)
        {
        case DP_ENABLE:
            port->display_port_data_.DpEnabled = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case DP_AUTO_MODE_ENTRY:
            port->display_port_data_.DpAutoModeEntryEnabled = (pCmd->val == 0) ? FALSE : TRUE;
            break;
        case DP_SEND_STATUS:
          if (port->policy_is_dfp_ == TRUE &&
              port->policy_state_ == PE_SRC_Ready)
          {
            DP_RequestPartnerStatus(port);
          }
          else if (port->policy_is_dfp_ == FALSE &&
                   port->policy_state_ == PE_SNK_Ready)
          {
            DP_SendAttention(port);
          }
          break;
        case DP_CAP:
            if (len < sizeof(FSC_U32)) { goto end_of_cmd; }
            port->display_port_data_.DpCap.word = READ_WORD((&pCmd->val + 1));
            pCmd = (struct pd_buf*) ((FSC_U8*) pCmd + pCmd->val);
            break;
        case DP_STATUS:
            if (len < sizeof(FSC_U32)) { goto end_of_cmd; }
            port->display_port_data_.DpStatus.word = READ_WORD((&pCmd->val + 1));
            pCmd = (struct pd_buf*) ((FSC_U8*) pCmd + pCmd->val);
            break;
        case DP_EOP:
        default:
            goto end_of_cmd;
            break;
        }
        pCmd++;
    }

end_of_cmd:
    return TRUE;
}

static FSC_BOOL ProcessDPRead(FSC_U8 *subCmds, FSC_U8 *outBuf,
                              struct Port *port)
{
    FSC_U32 i;
    FSC_U32 len;
    struct pd_buf *pRsp = (struct pd_buf*) outBuf;

    for (i = 0; i < HCMD_PAYLOAD_SIZE / 2; i++)
    {
        len = PTR_DIFF(&outBuf[HCMD_PAYLOAD_SIZE], pRsp);
        if (len < sizeof(struct pd_buf))
        {
            /* at least 2 bytes needed */
            goto out_buf_full;

        }
        pRsp->id = subCmds[i];
        len -= sizeof(struct pd_buf);
        switch (pRsp->id)
        {
        case DP_ENABLE:
            pRsp->val = port->display_port_data_.DpEnabled;
            break;
        case DP_AUTO_MODE_ENTRY:
            pRsp->val = port->display_port_data_.DpAutoModeEntryEnabled;
            break;
        case DP_CAP:
            if (len < sizeof(FSC_U32)) break;
            pRsp->val = WRITE_INT((&pRsp->val + 1),
                                    port->display_port_data_.DpCap.word);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        case DP_STATUS:
            if (len < sizeof(FSC_U32)) break;
            pRsp->val = WRITE_INT((&pRsp->val + 1),
                    port->display_port_data_.DpStatus.word);
            pRsp = (struct pd_buf*)((FSC_U8*)pRsp + pRsp->val);
            break;
        case DP_EOP:
        default:
            pRsp->id = DP_EOP;
            goto end_of_cmd;
            break;
        }
        pRsp++;
    }

out_buf_full:
    return FALSE;
end_of_cmd:
    return TRUE;
}

static void ProcessDpCommands(HostCmd_t *inCmd, HostCmd_t *outMsg,
                            struct Port *port)
{
    FSC_BOOL read = (inCmd->dp.cmd.req.rw == 0) ? TRUE : FALSE;
    FSC_BOOL status = FALSE;
    if (read == TRUE)
    {
        status = ProcessDPRead(inCmd->dp.cmd.req.payload,
                               outMsg->dp.cmd.rsp.payload, port);
    }
    else
    {
        status = ProcessDPWrite(inCmd->dp.cmd.req.payload, port);
    }

    if (status == TRUE)
    {
        outMsg->dp.cmd.rsp.error = HCMD_STATUS_SUCCESS;
    }
    else
    {
        outMsg->dp.cmd.rsp.error = HCMD_STATUS_FAILED;
    }
}
#endif /* FSC_HAVE_DP */

static void ProcessUserClassCmd(HostCmd_t *inCmd, HostCmd_t *outMsg, struct Port *port)
{
    outMsg->userClass.cmd.rsp.id = inCmd->userClass.cmd.req.id;
    switch (inCmd->userClass.cmd.req.id)
    {
    case 0:
        outMsg->userClass.cmd.rsp.payload[0] =
                platform_get_device_irq_state(port->port_id_) ? 0x00 : 0xFF;
        outMsg->userClass.cmd.rsp.error = HCMD_STATUS_SUCCESS;
        break;
    case 1:
        port->i2c_addr_ = inCmd->userClass.cmd.req.payload[0];
        outMsg->userClass.cmd.rsp.error = HCMD_STATUS_SUCCESS;
        break;
    default:
        outMsg->userClass.cmd.rsp.error = HCMD_STATUS_NOT_IMPLEMENTED;
        break;
    }
}

static void ProcessDeviceInfo(HostCmd_t *outMsg, struct Port *port)
{
    outMsg->deviceInfo.cmd.rsp.error = HCMD_STATUS_SUCCESS;
    outMsg->deviceInfo.cmd.rsp.mcu = MY_MCU;
    outMsg->deviceInfo.cmd.rsp.device = MY_DEV_TYPE;
    outMsg->deviceInfo.cmd.rsp.hostcom[0] = HOSTCOM_REV_LOW;
    outMsg->deviceInfo.cmd.rsp.hostcom[1] = HOSTCOM_REV_HIGH;
    outMsg->deviceInfo.cmd.rsp.config[0] = 0x00;
    outMsg->deviceInfo.cmd.rsp.config[1] = 0x00;
    outMsg->deviceInfo.cmd.rsp.fw[0] = FSC_TYPEC_CORE_FW_REV_LOWER;
    outMsg->deviceInfo.cmd.rsp.fw[1] = FSC_TYPEC_CORE_FW_REV_MIDDLE;
    outMsg->deviceInfo.cmd.rsp.fw[2] = FSC_TYPEC_CORE_FW_REV_UPPER;

}

static void ProcessWriteI2CFCSDevice(HostCmd_t* inCmd, HostCmd_t *outMsg,
                              struct Port *port)
{
    HAL_StatusTypeDef result;
    FSC_U8 reg_addr = inCmd->wrI2CDev.cmd.req.reg[0];
    FSC_U8 slave_addr = inCmd->wrI2CDev.cmd.req.addr;
    FSC_U8 *buf  = inCmd->wrI2CDev.cmd.req.data;
    FSC_U8 size = inCmd->wrI2CDev.cmd.req.dlen;

    I2C_HandleTypeDef i2chandle = { .Instance = I2C2, .State =
            HAL_I2C_STATE_READY };
    result = HAL_I2C_Mem_Write(&i2chandle, slave_addr, reg_addr, 1, buf, size,
            0xFF);

    if (result == HAL_OK)
    {
        outMsg->rdI2CDev.cmd.rsp.error = HCMD_STATUS_SUCCESS;
    } else
    {
        outMsg->rdI2CDev.cmd.rsp.error = HCMD_STATUS_FAILED;
    }
}

static void ProcessReadI2CFCSDevice(HostCmd_t* inCmd, HostCmd_t* outMsg,
                             struct Port *port)
{
    HAL_StatusTypeDef result;
    FSC_U8 slave_addr = inCmd->rdI2CDev.cmd.req.addr;
    FSC_U8 reg_addr = inCmd->rdI2CDev.cmd.req.reg[0];
    FSC_U8 *buf = outMsg->rdI2CDev.cmd.rsp.data;
    FSC_U8 size = inCmd->rdI2CDev.cmd.req.dlen;

    I2C_HandleTypeDef i2chandle = { .Instance = I2C2, .State =
            HAL_I2C_STATE_READY };
    result = HAL_I2C_Mem_Read(&i2chandle, slave_addr, reg_addr, 1, buf, size,
            0xFF);

    if (result == HAL_OK)
    {
        outMsg->rdI2CDev.cmd.rsp.error = HCMD_STATUS_SUCCESS;
    } else
    {
        outMsg->rdI2CDev.cmd.rsp.error = HCMD_STATUS_FAILED;
    }
}

void ProcessMsg(FSC_U8 *inMsgBuffer, FSC_U8 *outMsgBuffer, struct Port *port)
{
    HostCmd_t *inCmd = (HostCmd_t*) inMsgBuffer;
    HostCmd_t *outMsg = (HostCmd_t*) outMsgBuffer;

    HCOM_MEM_FILL(outMsg, 0, sizeof(HostCmd_t));

    /* Set the common response here */
    outMsg->request.opcode = inCmd->request.opcode;
    outMsg->request.cmd.rsp.status = HCMD_STATUS_SUCCESS;

    switch (inCmd->request.opcode)
    {
    case HCMD_GET_DEVICE_INFO:
        ProcessDeviceInfo(outMsg, port);
        break;
    case HCMD_READ_I2C_FCS_DEV:
        ProcessReadI2CFCSDevice(inCmd, outMsg, port);
        break;
    case HCMD_WRITE_I2C_FCS_DEV:
        ProcessWriteI2CFCSDevice(inCmd, outMsg, port);
        break;
    case HCMD_USER_CLASS:
        ProcessUserClassCmd(inCmd, outMsg, port);
        break;
    case HCMD_TYPEC_CLASS:
        ProcessTCStatus(inCmd, outMsg, port);
        break;
    case HCMD_PD_CLASS:
        ProcessPDStatus(inCmd, outMsg, port);
        break;
#ifdef FSC_HAVE_DP
    case HCMD_DP_CLASS:
        ProcessDpCommands(inCmd, outMsg, port);
        break;
#endif /* FSC_HAVE_DP */
    default:
        /* Return that the request is not implemented */
        outMsg->request.cmd.rsp.status = HCMD_STATUS_FAILED;
        break;
    }
} /* ProcessMsg */

#endif /* FSC_HAVE_USBHID */

