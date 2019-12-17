/*******************************************************************************
 * @file     port.c
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
 * port.cpp
 *
 * Implements the port interface for the port manager.
 * ************************************************************************** */

#include "port.h"
#include "dpm.h"
#include "vendor_info.h"

#ifdef FSC_HAVE_DP
#include "display_port.h"
#endif /* FSC_HAVE_DP */

/* Initialize
 *
 * Initializes the port data object's members, populates the register map with
 * I2C read data from the device, configures capability objects, and writes
 * initial configuration values to the device.
 */

void InitializeVars(struct Port *port, FSC_U8 id, FSC_U8 i2c_addr)
{
  FSC_U32 i = 0;

  port->port_id_ = id;
  port->i2c_addr_ = i2c_addr;
  port->idle_ = FALSE;
  port->initialized_ = FALSE;
  port->port_type_ = USBTypeC_UNDEFINED;
  port->source_or_sink_ = Source;
  port->tc_enabled_ = TRUE;
  port->tc_state_ = Disabled;
  port->tc_substate_ = 0;
  port->src_preferred_ = Type_C_Implements_Try_SRC;
  port->snk_preferred_ = Type_C_Implements_Try_SNK;
  port->acc_support_ = (Type_C_Supports_VCONN_Powered_Accessory ||
                        Type_C_Supports_Audio_Accessory) ? TRUE : FALSE;
  port->snk_current_ = utccOpen;

  port->src_current_ = Rp_Value + 1;
  if (port->src_current_ < utccDefault || port->src_current_ > utcc3p0A)
  {
    port->src_current_ = utccDefault;
  }

  port->cc_pin_ = NONE;
  port->cc_term_ = CCTypeUndefined;
  ResetDebounceVariables(port);
  port->is_hard_reset_ = FALSE;
#ifdef FSC_HAVE_FRSWAP
  port->is_fr_swap_ = FALSE;
#endif /* FSC_HAVE_FRSWAP */
  port->is_pr_swap_ = FALSE;
  port->is_vconn_swap_ = FALSE;
  port->unattach_loop_counter_ = 0;
  port->vbus_transition_time_ = 20 * kMSTimeFactor;
  port->is_dead_battery_ = FALSE;
  port->have_sink_path_ = FALSE;
  port->have_HV_path_ = FALSE;
  port->pd_active_ = FALSE;
  port->pd_enabled_ = USB_PD_Support;
  port->protocol_state_ = PRLDisabled;
  port->pd_tx_status_ = txIdle;
  port->pd_tx_flag_ = FALSE;
  port->policy_msg_tx_sop_ = SOP_TYPE_SOP;
  port->protocol_msg_rx_ = FALSE;
  port->protocol_msg_rx_sop_ = SOP_TYPE_SOP;
  port->protocol_msg_tx_sop_ = SOP_TYPE_SOP;
  port->protocol_retries_ = RETRIES_PD30;
  port->protocol_use_sinktx_ = FALSE;
  port->waiting_on_hr_ = FALSE;
#ifdef FSC_HAVE_EXTENDED
  port->protocol_ext_num_bytes_ = 0;
  port->protocol_ext_chunk_number_ = 0;
  port->protocol_ext_request_chunk_ = 0;
  port->protocol_ext_send_chunk_ = FALSE;
  port->protocol_ext_request_cmd_ = 0;
  port->protocol_chunking_supported_ = TRUE;
#ifdef FSC_HAVE_SRC
  port->policy_src_cap_ext_.VID = Manufacturer_Info_VID_Port;
  port->policy_src_cap_ext_.PID = Manufacturer_Info_PID_Port;
  port->policy_src_cap_ext_.SrcPDP = PD_Power_as_Source / 1000;
#endif /* FSC_HAVE_SRC */
#endif /* FSC_HAVE_EXTENDED */
  port->policy_state_ = PE_SRC_Disabled;
  port->policy_subindex_ = 0;
  port->policy_is_ams_ = FALSE;
  port->policy_sinktx_state_ = SinkTxNG;

  port->policy_is_source_ = TRUE;
  port->policy_is_dfp_ = TRUE;
  port->is_contract_valid_ = FALSE;
  port->is_vconn_source_ = FALSE;
  port->collision_counter_ = 0;
  port->hard_reset_counter_ = 0;
  port->caps_counter_ = 0;
  port->policy_has_contract_ = FALSE;
  port->needs_goto_min_ = FALSE;
  port->renegotiate_ = FALSE;
  port->policy_wait_on_sink_caps_ = FALSE;
  port->sink_selected_voltage_ = FSC_VBUS_05_V;
  port->sink_transition_up_ = FALSE;
  port->sink_request_max_voltage_ = 5000;  /* 12V */
  port->sink_request_max_power_ = 1000;      /* 0.1A */
  port->sink_request_op_power_ = 100;       /* 0.1A */
  port->sink_partner_max_power_ = 0;
  port->sink_request_low_power_ = FALSE;
  port->sink_goto_min_compatible_ = FALSE;
  port->sink_usb_suspend_compatible_ = FALSE;
  port->sink_usb_comm_capable_ = FALSE;
  port->partner_caps_.object = 0;
  port->partner_caps_available_ = FALSE;
  port->pd_HV_option_ = FSC_VBUS_09_V;
  port->source_is_apdo_ = FALSE;
  port->pd_preferred_rev_ = PD_Specification_Revision;

#if defined(FSC_DEBUG) || defined(FSC_HAVE_USBHID)
  port->source_caps_updated_ = FALSE;
#endif /* FSC_DEBUG || FSC_HAVE_USBHID */

  TimerDisable(&port->tc_state_timer_);
  TimerDisable(&port->policy_state_timer_);
  TimerDisable(&port->policy_sinktx_timer_);
  TimerDisable(&port->cc_debounce_timer_);
  TimerDisable(&port->pd_debounce_timer_);
  TimerDisable(&port->no_response_timer_);
  TimerDisable(&port->swap_source_start_timer_);
  TimerDisable(&port->pps_timer_);
  TimerDisable(&port->dpm_timer_);
  TimerDisable(&port->protocol_timer_);

  /*
   * Initialize SOP-related arrays.
   * NOTE: Update this loop condition if supporting additional SOP types!
   */
  for (i = 0; i < NUM_SOP_SUPPORTED; ++i) {
    port->message_id_counter_[i] = 0;
    port->message_id_[i] = 0xFF;
  }

  for (i = 0; i < 7; ++i) {
    port->policy_rx_data_obj_[i].object = 0;
    port->policy_tx_data_obj_[i].object = 0;
    port->pd_transmit_objects_[i].object = 0;
    port->caps_sink_[i].object = 0;
    port->caps_source_[i].object = 0;
    port->caps_received_[i].object = 0;
  }

  /* Need to check PD_Port_Type if supporting producer/consumer or
   * consumer/producer here */
  switch(Type_C_State_Machine)
  {
  case 0:
      port->port_type_ = USBTypeC_Source;
      break;
  case 1:
      port->port_type_ = USBTypeC_Sink;
      break;
  case 2:
  default:
      port->port_type_ = USBTypeC_DRP;
      break;
  }

  /* Set up the capabilities objects */
#ifdef FSC_HAVE_SNK
  port->caps_header_sink_.NumDataObjects = Num_Snk_PDOs;
  port->caps_header_sink_.PortDataRole = 0;                                /* UFP */
  port->caps_header_sink_.PortPowerRole = 0;                               /* Sink */
  port->caps_header_sink_.SpecRevision = port->pd_preferred_rev_;        /* Spec rev */
  VIF_InitializeSnkCaps(port->caps_sink_);
#endif /* FSC_HAVE_SNK */

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
  port->caps_header_source_.NumDataObjects = Num_Src_PDOs;
  port->caps_header_source_.PortDataRole = 0;                                       /* UFP */
  port->caps_header_source_.PortPowerRole = 1;                                      /* Source */
  port->caps_header_source_.SpecRevision = port->pd_preferred_rev_;               /* Spec rev */
  VIF_InitializeSrcCaps(port->caps_source_);
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_VDM
  TimerDisable(&port->vdm_timer_);

  port->vdm_next_ps_ = PE_SRC_Disabled;
  port->original_policy_state_ = PE_SRC_Disabled;
  port->vdm_expecting_response_ = FALSE;
  port->vdm_sending_data_ = FALSE;
  port->vdm_auto_state_ = AUTO_VDM_INIT;
  port->vdm_msg_length_ = 0;
  port->vdm_msg_tx_sop_ = SOP_TYPE_SOP;
  port->svid_discv_idx_ = -1;
  port->svid_discvry_done_ = FALSE;
  port->svid_enable_ = (Num_SVIDs_min_SOP > 0 &&
                       Modal_Operation_Supported_SOP) ? TRUE : FALSE;
  port->mode_enable_ = Modal_Operation_Supported_SOP;
  port->my_svid_ = SVID_DEFAULT;
  port->my_mode_ = MODE_DEFAULT;
  port->mode_entered_ = FALSE;
  port->discover_id_counter_ = 0;

  for (i = 0; i < 7; ++i) {
    port->vdm_msg_obj_[i].object = 0;
  }
  port->auto_mode_entry_pos_ = -1;
  port->auto_mode_entry_enabled_ = TRUE;

#ifdef FSC_HAVE_DP
  DP_Initialize(port);
  port->display_port_data_.DpAutoModeEntryEnabled = DisplayPort_Auto_Mode_Entry;
  port->display_port_data_.DpEnabled = DisplayPort_Enabled;
#endif /* FSC_HAVE_DP */
#endif /* FSC_HAVE_VDM */

  /* Initialize DPM before returning */
  DPM_Initialize(port);
}

void InitializePort(struct Port *port)
{
  /* Read all of the register values to update our cache */
  ReadAllRegisters(port);

  /* Clear VD Masks */
  /* NOTE - This is a chip bug - AlertMskH.M_VD_ALERT doesn't work */
  port->registers_.AlertVDMsk.byte = 0;
  WriteRegister(port, regALERT_VD_MSK);

  /* Clear reset flag */
  ClearInterrupt(port, regFAULTSTAT, MSK_ALL_REGS_RESET);

  /* Set our snk/src path options */
  port->have_sink_path_ = TRUE;
  port->have_HV_path_ = TRUE;

  /* Set SDAC hysteresis to 85mv */
  port->registers_.Slice.SDAC_HYS = SDAC_HYS_DEFAULT;
  port->registers_.Slice.SDAC = SDAC_DEFAULT;
  WriteRegister(port, regSLICE);

  /* Disable automatic debug accessory while firmware is running */
  port->registers_.TcpcCtrl.DEBUG_ACC_CTRL = 1;
  WriteRegister(port, regTCPC_CTRL);

  /* Set GPIO1 (3695 Control) Enabled and High (active low) at startup */
  port->registers_.Gpio1Cfg.GPO1_EN = 1;
  port->registers_.Gpio1Cfg.GPO1_VAL = 1;
  WriteRegister(port, regGPIO1_CFG);

  /* Initially mask all interrupts - unmask/remask as needed */
  port->registers_.AlertMskL.byte = 0;
  WriteRegister(port, regALERTMSKL);
  port->registers_.AlertMskH.byte = 0;
  WriteRegister(port, regALERTMSKH);

  /* Dead Battery Handling */
  /* For now (until silicon fix) we assume that VBUS on init -> DB */
#if 0 /* Review this later - what silicon fix was needed? */
  if (port->registers_.PwrStat.VBUS_VAL) {
    /* Workaround for register cleared on DB startup */
    port->registers_.RoleCtrl.DRP = 0;
    port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
    port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
    WriteRegister(port, regROLECTRL);

    port->port_type_ = USBTypeC_Sink;
    port->source_or_sink_ = Sink;
    port->is_dead_battery_ = TRUE;
  }
#endif /* 0 */

  port->initialized_ = TRUE;
}

/* Register Update Functions */
FSC_BOOL ReadRegister(struct Port *port, enum RegAddress regaddress)
{
  return platform_i2c_read(port->i2c_addr_, (FSC_U8)regaddress, 1,
                           AddressToRegister(&port->registers_, regaddress));
}

FSC_BOOL ReadRegisters(struct Port *port, enum RegAddress regaddr, FSC_U8 cnt)
{
  return platform_i2c_read(port->i2c_addr_, (FSC_U8)regaddr, cnt,
                           AddressToRegister(&port->registers_, regaddr));
}

/*
 * ReadStatusRegisters
 *
 * Updates register map with the device's interrupt and status register data.
 */
void ReadStatusRegisters(struct Port *port)
{
  /* Read interrupts */
  ReadRegisters(port, regALERTL, 2);
  ReadRegister(port, regALERT_VD);

  /* Read statuses */
  ReadRegisters(port, regCCSTAT, 3);
  ReadRegister(port, regVD_STAT);
}

void ReadAllRegisters(struct Port *port)
{
  ReadRegister(port, regVENDIDL);
  ReadRegister(port, regVENDIDH);
  ReadRegister(port, regPRODIDL);
  ReadRegister(port, regPRODIDH);
  ReadRegister(port, regDEVIDL);
  ReadRegister(port, regDEVIDH);
  ReadRegister(port, regTYPECREVL);
  ReadRegister(port, regTYPECREVH);
  ReadRegister(port, regUSBPDVER);
  ReadRegister(port, regUSBPDREV);
  ReadRegister(port, regPDIFREVL);
  ReadRegister(port, regPDIFREVH);
  ReadRegister(port, regALERTL);
  ReadRegister(port, regALERTH);
  ReadRegister(port, regALERTMSKL);
  ReadRegister(port, regALERTMSKH);
  ReadRegister(port, regPWRSTATMSK);
  ReadRegister(port, regFAULTSTATMSK);
  ReadRegister(port, regSTD_OUT_CFG);
  ReadRegister(port, regTCPC_CTRL);
  ReadRegister(port, regROLECTRL);
  ReadRegister(port, regFAULTCTRL);
  ReadRegister(port, regPWRCTRL);
  ReadRegister(port, regCCSTAT);
  ReadRegister(port, regPWRSTAT);
  ReadRegister(port, regFAULTSTAT);
  ReadRegister(port, regCOMMAND);
  ReadRegister(port, regDEVCAP1L);
  ReadRegister(port, regDEVCAP1H);
  ReadRegister(port, regDEVCAP2L);
  ReadRegister(port, regSTD_OUT_CAP);
  ReadRegister(port, regMSGHEADR);
  ReadRegister(port, regRXDETECT);
  ReadRegister(port, regRXBYTECNT);
  ReadRegister(port, regRXSTAT);
  ReadRegister(port, regTRANSMIT);
  ReadRegister(port, regTXBYTECNT);
  ReadRegister(port, regVBUS_VOLTAGE_L);
  ReadRegister(port, regVBUS_VOLTAGE_H);
  ReadRegister(port, regVBUS_SNK_DISCL);
  ReadRegister(port, regVBUS_SNK_DISCH);
  ReadRegister(port, regVBUS_STOP_DISCL);
  ReadRegister(port, regVBUS_STOP_DISCH);
  ReadRegister(port, regVALARMHCFGL);
  ReadRegister(port, regVALARMHCFGH);
  ReadRegister(port, regVALARMLCFGL);
  ReadRegister(port, regVALARMLCFGH);
  ReadRegister(port, regVCONN_OCP);
  ReadRegister(port, regSLICE);
  ReadRegister(port, regRESET);
  ReadRegister(port, regVD_STAT);
  ReadRegister(port, regGPIO1_CFG);
  ReadRegister(port, regGPIO2_CFG);
  ReadRegister(port, regGPIO_STAT);
  ReadRegister(port, regDRPTOGGLE);
  ReadRegister(port, regSINK_TRANSMIT);
  ReadRegister(port, regSRC_FRSWAP);
  ReadRegister(port, regSNK_FRSWAP);
  ReadRegister(port, regALERT_VD);
  ReadRegister(port, regALERT_VD_MSK);
  ReadRegister(port, regRPVAL_OVERRIDE);
}

void ReadRxRegisters(struct Port *port, FSC_U8 numbytes)
{
  /* Check length limit */
  if (numbytes > COMM_BUFFER_LENGTH) numbytes = COMM_BUFFER_LENGTH;

  platform_i2c_read(port->i2c_addr_, regRXDATA_00,
                    numbytes, port->registers_.RxData);
}

void WriteRegister(struct Port *port, enum RegAddress regaddress)
{
  platform_i2c_write(port->i2c_addr_, (FSC_U8)regaddress, 1,
                     AddressToRegister(&port->registers_, regaddress));
}

void WriteRegisters(struct Port *port, enum RegAddress regaddr, FSC_U8 cnt)
{
  platform_i2c_write(port->i2c_addr_, (FSC_U8)regaddr, cnt,
                     AddressToRegister(&port->registers_, regaddr));
}

void WriteTxRegisters(struct Port *port, FSC_U8 numbytes)
{
  /* Check length limit */
  if (numbytes > COMM_BUFFER_LENGTH) numbytes = COMM_BUFFER_LENGTH;

  platform_i2c_write(port->i2c_addr_, regTXDATA_00,
                     numbytes, port->registers_.TxData);
}

/*
 * Sets bits indicated by mask in interrupt register at address. This has the
 * effect of clearing the specified interrupt(s).
 */
void ClearInterrupt(struct Port *port, enum RegAddress address, FSC_U8 mask)
{
  FSC_U8 data = mask;
  platform_i2c_write(port->i2c_addr_, (FSC_U8)address, 1, &data);
  RegClearBits(&(port->registers_), address, mask);
}

/*
 * SendCommand
 *
 * Sets the port's command register to cmd and writes it to the device.
 */
void SendCommand(struct Port *port, enum DeviceCommand cmd)
{
  /* Make sure the command is supported */
  if ((cmd == SinkVbus || cmd == DisableSinkVbus) && !port->have_sink_path_)
    return;

  if (cmd == SourceVbusHighV) {
    if (!port->have_HV_path_) {
      return;
    }
    else {
      /* GPIO workaround for HV path */
      platform_setHVSwitch(TRUE);

      platform_delay(2000);

      port->registers_.Command = DisableSourceVbus;
      WriteRegister(port, regCOMMAND);
    }
  }
  else {
    port->registers_.Command = cmd;
    WriteRegister(port, regCOMMAND);
  }

  if (cmd == SourceVbusDefaultV) {
    port->sink_selected_voltage_ = FSC_VBUS_05_V;
  }

  /* Check the 3695 GPIO enable bit if needed */
  if (cmd == SourceVbusDefaultV || cmd == SinkVbus) {
    port->registers_.Gpio1Cfg.GPO1_VAL = 0; /* ON - Active Low */
    WriteRegister(port, regGPIO1_CFG);
  }
  else {
    port->registers_.Gpio1Cfg.GPO1_VAL = 1; /* OFF - Active Low */
    WriteRegister(port, regGPIO1_CFG);
  }

  if (cmd != SourceVbusHighV) {
      platform_setHVSwitch(FALSE);
      platform_setPPSVoltage(port->port_id_, 0);
  }
}

void SetRpValue(struct Port *port, USBTypeCCurrent currentVal)
{
  switch (currentVal) {
    case utccDefault: /* Rp Default */
      port->registers_.RoleCtrl.RP_VAL = 0b00;
      break;
    case utcc1p5A:  /* Rp 1.5A */
      port->registers_.RoleCtrl.RP_VAL = 0b01;
      break;
    case utcc3p0A:  /* Rp 3.0A*/
      port->registers_.RoleCtrl.RP_VAL = 0b10;
      break;
    default:        /* Go to default */
      port->registers_.RoleCtrl.RP_VAL = 0b00;
      break;
    }
}

/* Type-C Functionality */
void UpdateSourceCurrent(struct Port *port, USBTypeCCurrent currentVal)
{
  SetRpValue(port, currentVal);
  WriteRegister(port, regROLECTRL);
}

void UpdateSinkCurrent(struct Port *port)
{
  /* As a sink, one CC pin will be open, and we want to check the other one */
  port->snk_current_ =
    (USBTypeCCurrent)((port->registers_.CCStat.CC2_STAT == CCTermSnkOpen) ?
                       port->registers_.CCStat.CC1_STAT :
                       port->registers_.CCStat.CC2_STAT);
}

/* Returns the VBus voltage, multiplied by VBUS_SCALE */
FSC_U16 GetVBusVoltage(struct Port *port)
{
  /* Max scaled voltage is 0xFFC, min is 0 */
  FSC_U16 voltage = 0;

  /* Read the current register values */
  ReadRegisters(port, regVBUS_VOLTAGE_L, 2);

  /* Combine value bytes */
  voltage = ((FSC_U16)port->registers_.VBusVoltageH.VBUS_V_HI) << 8;
  voltage |= port->registers_.VBusVoltageL.VBUS_V_LO;

  /* Scale value as needed */
  switch (port->registers_.VBusVoltageH.VBUS_SCALE) {
    case 0b01: /* Scaled by 2 */
      voltage *= 2;
      break;
    case 0b10: /* Scaled by 4 */
      voltage *= 4;
      break;
    case 0: /* No scaling, fall through */
    default:
      break;
  }

  /* Voltage measurement in millivolts */
  return voltage * 25;
}

FSC_BOOL IsVbusInRange(struct Port *port, FSC_U16 mv)
{
  /* Returns true when voltage (mv) is within 5% */
  FSC_U16 measurement = GetVBusVoltage(port);
  return ((measurement > (mv - mv / 20)) &&
          (measurement < (mv + mv / 20))) ? TRUE : FALSE;
}

FSC_BOOL IsVbusVSafe0V(struct Port *port)
{
  /* Returns true when voltage is < 0.8V */
  FSC_U16 voltage = GetVBusVoltage(port);
  return (voltage < FSC_VBUS_VSAFE0_V) ? TRUE : FALSE;
}

FSC_BOOL IsVbusVSafe5V(struct Port *port)
{
  /* Returns true when voltage is within 4.75V - 5.5V */
  FSC_U16 measurement = GetVBusVoltage(port);
  return ((measurement > 4750) && (measurement < 5500)) ? TRUE : FALSE;
}

FSC_BOOL IsVbusOverVoltage(struct Port *port, FSC_U16 mv)
{
  /* Returns true when voltage is > the voltage argument */
  FSC_U16 measurement = GetVBusVoltage(port);
  return (measurement > mv) ? TRUE : FALSE;
}

void SetVBusSnkDisc(struct Port *port, FSC_U16 level)
{
  port->registers_.VBusSnkDiscL.byte = level & 0x00FF;
  port->registers_.VBusSnkDiscH.byte = (level & 0x0300) >> 8;
  WriteRegisters(port, regVBUS_SNK_DISCL, 2);
}

void SetVBusStopDisc(struct Port *port, FSC_U16 level)
{
  port->registers_.VBusStopDiscL.byte = level & 0x00FF;
  port->registers_.VBusStopDiscH.byte = (level & 0x0300) >> 8;
  WriteRegisters(port, regVBUS_STOP_DISCL, 2);
}

void SetVBusAlarm(struct Port *port, FSC_U16 levelL, FSC_U16 levelH)
{
  port->registers_.VAlarmLCfgL.byte = levelL & 0x00FF;
  port->registers_.VAlarmLCfgH.byte = (levelL & 0x0300) >> 8;
  WriteRegisters(port, regVALARMLCFGL, 2);

  port->registers_.VAlarmHCfgL.byte = levelH & 0x00FF;
  port->registers_.VAlarmHCfgH.byte = (levelH & 0x0300) >> 8;
  WriteRegisters(port, regVALARMHCFGL, 2);
}

/*
 * DecodeCCTermination
 *
 * Returns the termination seen by the CC line.
 * If the caller sets (port->cc_pin_ == CC1), then this checks the CC1
 * line. Otherwise, this checks the CC2 line.
 */
CCTermType DecodeCCTermination(struct Port *port)
{
  /* Check the undefined cases */
  if (port->cc_pin_ == NONE ||
      (port->cc_pin_ == CC1 &&
       (port->registers_.RoleCtrl.CC1_TERM == CCRoleOpen ||
        port->registers_.RoleCtrl.CC1_TERM == CCRoleRa)) ||
      (port->cc_pin_ == CC2 &&
       (port->registers_.RoleCtrl.CC2_TERM == CCRoleOpen ||
        port->registers_.RoleCtrl.CC2_TERM == CCRoleRa))) {
    return CCTypeUndefined;
  }

  if (port->source_or_sink_ == Source) {
#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
    return DecodeCCTerminationSource(port);
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACC) */
  }
  else {
#ifdef FSC_HAVE_SNK
    return DecodeCCTerminationSink(port);
#endif /* FSC_HAVE_SNK */
  }

  /* Shouldn't get here, but call it undefined */
  return CCTypeUndefined;
}

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
CCTermType DecodeCCTerminationSource(struct Port *port)
{
  CCTermType term = CCTypeUndefined;
  switch ((port->cc_pin_ == CC1) ?
          port->registers_.CCStat.CC1_STAT :
          port->registers_.CCStat.CC2_STAT) {
  case CCTermSrcOpen:
    term = CCTypeOpen;
    break;
  case CCTermSrcRa:
    term = CCTypeRa;
    break;
  case CCTermSrcRd:
    term = CCTypeRdUSB;
    break;
  case CCTermSrcUndefined: /* Fall through */
  default:
    break;
  }
  return term;
}
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACC) */

#ifdef FSC_HAVE_SNK
CCTermType DecodeCCTerminationSink(struct Port *port)
{
  CCTermType term = CCTypeUndefined;
  switch ((port->cc_pin_ == CC1) ?
          port->registers_.CCStat.CC1_STAT :
          port->registers_.CCStat.CC2_STAT) {
  case CCTermSnkOpen:
    term = CCTypeOpen;
    break;
  case CCTermSnkDefault:
    term = CCTypeRdUSB;
    break;
  case CCTermSnkRp1p5:
    term = CCTypeRd1p5;
    break;
  case CCTermSnkRp3p0:
    term = CCTypeRd3p0;
    break;
  default:
    break;
  }
  return term;
}
#endif /* FSC_HAVE_SNK */

void ResetDebounceVariables(struct Port *port)
{
  port->cc_term_previous_ = CCTypeUndefined;
  port->cc_term_cc_debounce_ = CCTypeUndefined;
  port->cc_term_pd_debounce_ = CCTypeUndefined;
  port->cc_term_pd_debounce_prev_ = CCTypeUndefined;
}

void DebounceCC(struct Port *port)
{
  /* PD Debounce (filter out PD traffic that could look like CC changes) */
  CCTermType cctermcurrent = DecodeCCTermination(port);
  if (port->cc_term_previous_ != cctermcurrent) {
    /* If our latest value has changed, update and reset timers */
    port->cc_term_previous_ = cctermcurrent;
    /* Might be better to call restart than disable & enable */
    TimerDisable(&port->pd_debounce_timer_);
    TimerStart(&port->pd_debounce_timer_, ktPDDebounce);
  }

  /* If our debounce timer has expired, record the latest values */
  if (TimerExpired(&port->pd_debounce_timer_)) {
    port->cc_term_pd_debounce_ = port->cc_term_previous_;
    TimerDisable(&port->pd_debounce_timer_);
  }

  /* CC Debounce (debounce the cc lines once PD traffic has been filtered) */
  if (port->cc_term_pd_debounce_prev_ != port->cc_term_pd_debounce_) {
    /* If our latest value has changed, update and reset timers */
    port->cc_term_pd_debounce_prev_ = port->cc_term_pd_debounce_;
    port->cc_term_cc_debounce_ = CCTypeUndefined;
    /* Timer was enabled above */
    TimerDisable(&port->cc_debounce_timer_);
    TimerStart(&port->cc_debounce_timer_, ktCCDebounce - ktPDDebounce);
  }

  /* If our debounce timer has expired, record the latest values */
  if (TimerExpired(&port->cc_debounce_timer_)) {
    port->cc_term_cc_debounce_ = port->cc_term_pd_debounce_prev_;
    TimerDisable(&port->cc_debounce_timer_);
  }
}

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
/*
 * @brief Sets the Source variables and enables VBUS monitor and Alarm high.
 * It disables the Alarm Interrupt so all source states only need
 * to un mask VBUS Alarm.
 */
void SetStateSource(struct Port *port)
{
  port->source_or_sink_ = Source;
  port->snk_current_ = utccOpen;
  port->tc_substate_ = 0;
  ResetDebounceVariables(port);

  /* Disable the VBUS Alarm Interrupt only */
  port->registers_.AlertMskL.M_PORT_PWR = 0;
  port->registers_.AlertMskL.M_VBUS_ALRM_HI = 0;
  WriteRegister(port, regALERTMSKL);
  port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
  WriteRegister(port, regALERTMSKH);
  /* Enable the VBUS monitor and enable the Alarm system
   * so next states need to only unmask Alarm interrupt */
  port->registers_.PwrCtrl.DIS_VBUS_MON = 0;
  port->registers_.PwrCtrl.DIS_VALARM = 0;
  WriteRegister(port, regPWRCTRL);

#ifdef FSC_LOGGING
  LogTCState(port);
#endif /* FSC_LOGGING */
}
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACC) */

void SetVConn(struct Port *port, FSC_BOOL enable)
{
  port->registers_.PwrCtrl.EN_VCONN = (enable ? 1 : 0);
  WriteRegister(port, regPWRCTRL);

  /* PD 3.0 requires to enable sop' and sop'' comm. for vconn source*/
  DPM_ReConfigureRxDetect(port);
}
FSC_BOOL GetVConn(struct Port *port)
{
  return (port->registers_.PwrCtrl.EN_VCONN == 1) ? TRUE : FALSE;
}

#ifdef FSC_HAVE_SNK
/*
 * @brief Sets the Sink variables and enables VBUS monitor and Alarm low.
 * It disables the Alarm Interrupt so all sink states only need
 * to un-mask VBUS Alarm.
 */
void SetStateSink(struct Port *port)
{
  port->source_or_sink_ = Sink;
  port->tc_substate_ = 0;
  ResetDebounceVariables(port);

  /* Disable the VBUS Alarm until required. All Sink state
   * can just enable the mask to enable VBUS ALARM High */
  port->registers_.AlertMskL.M_PORT_PWR = 0;
  port->registers_.AlertMskL.M_VBUS_ALRM_HI = 0;
  WriteRegister(port, regALERTMSKL);
  port->registers_.AlertMskH.M_VBUS_ALRM_LO = 0;
  WriteRegister(port, regALERTMSKH);

  /* Enable the device's auto-discharge and VBus measure features */
  port->registers_.PwrCtrl.DIS_VALARM = 0;
  port->registers_.PwrCtrl.DIS_VBUS_MON = 0;
  WriteRegister(port, regPWRCTRL);

#ifdef FSC_LOGGING
  LogTCState(port);
#endif /* FSC_LOGGING */
}
#endif /* FSC_HAVE_SNK */

/*
 * DetectCCPin
 *
 * Called to discover which pin is CC.
 * This sets port->cc_pin_ accordingly, and sets port->cc_term_
 * according to the termination found on the CC line.
 */
void DetectCCPin(struct Port *port)
{
  /* Decode CC1 termination */
  port->cc_pin_ = CC1;
  port->cc_term_ = DecodeCCTermination(port);

  if ((port->cc_term_ != CCTypeRdUSB) &&
      (port->cc_term_ != CCTypeRd1p5) &&
      (port->cc_term_ != CCTypeRd3p0)) {
    /* CC1 is not CC, so decode CC2 termination */
    port->cc_pin_ = CC2;
    port->cc_term_ = DecodeCCTermination(port);

    if ((port->cc_term_ != CCTypeRdUSB) &&
        (port->cc_term_ != CCTypeRd1p5) &&
        (port->cc_term_ != CCTypeRd3p0)) {

      /* ... Or maybe RaRa? */
      if (port->cc_term_ == CCTypeRa) {
        /* Call it CC1 for now */
        port->cc_pin_ = CC1;
      }
      else {
        /* Neither pin is CC */
        port->cc_pin_ = NONE;

        port->cc_term_ = CCTypeUndefined;
      }
    }
  }
}

/*
 * UpdateVConnTermination
 *
 * Measures and saves the termination seen on the pin that is not configured as
 * the CC line.
 */
void UpdateVConnTermination(struct Port *port)
{
  /* Flip our CC indicators so we can measure the VConn line */
  port->cc_pin_ = port->cc_pin_ == CC1 ? CC2 : CC1;

  port->vconn_term_ = DecodeCCTermination(port);

  /* Restore our CC indicators */
  port->cc_pin_ = port->cc_pin_ == CC1 ? CC2 : CC1;
}

/*
 * UpdateOrientation
 *
 * Updates the device's Type-C orientation according to which pin is currently
 * configured as the CC pin.
 *
 * This function does nothing if neither or both pins are configured as CC.
 */
void UpdateOrientation(struct Port *port)
{
  if (port->cc_pin_ == CC1) {
    port->registers_.TcpcCtrl.ORIENT = 0;
    port->registers_.StdOutCfg.ORIENT = 0;

    if (port->source_or_sink_ == Source) {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleRp;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleOpen;
    }
    else {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleOpen;
    }
  }
  else if (port->cc_pin_ == CC2) {
    port->registers_.TcpcCtrl.ORIENT = 1;
    port->registers_.StdOutCfg.ORIENT = 1;

    if (port->source_or_sink_ == Source) {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleOpen;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleRp;
    }
    else {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleOpen;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
    }
  }
  else {
    /* Set both Rp or Rd - used for handling Try.* states */
    port->registers_.TcpcCtrl.ORIENT = 0;
    port->registers_.StdOutCfg.ORIENT = 0;

    if (port->source_or_sink_ == Source) {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleRp;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleRp;
    }
    else {
      port->registers_.RoleCtrl.CC1_TERM = CCRoleRd;
      port->registers_.RoleCtrl.CC2_TERM = CCRoleRd;
    }
  }

  port->registers_.RoleCtrl.DRP = 0;

  WriteRegister(port, regTCPC_CTRL);
  WriteRegister(port, regSTD_OUT_CFG);
  WriteRegister(port, regROLECTRL);
}

void ClearState(struct Port *port)
{
  PDDisable(port);

  /* Disable VBus and VBus detection */
  SendCommand(port, DisableSourceVbus);
  SendCommand(port, DisableSinkVbus);

  /* Disable VConn, etc. */
  port->registers_.PwrCtrl.EN_VCONN = 0;
  port->registers_.PwrCtrl.DIS_VBUS_MON = 1;
  port->registers_.PwrCtrl.DIS_VALARM = 1;
  port->registers_.PwrCtrl.AUTO_DISCH = 0;
  port->registers_.PwrCtrl.EN_BLEED_DISCH = 1;
  port->registers_.PwrCtrl.FORCE_DISCH = 0;
  WriteRegister(port, regPWRCTRL);

  port->is_vconn_source_ = FALSE;

  ResetDebounceVariables(port);
  port->cc_pin_ = NONE;

  port->cc_term_ = CCTypeUndefined;
  port->vconn_term_ = CCTypeUndefined;

  port->tc_substate_ = 0;

  TimerDisable(&port->pd_debounce_timer_);

#ifdef FSC_LOGGING
  LogTCState(port);
#endif /* FSC_LOGGING */
}

#ifdef FSC_HAVE_ACC
FSC_BOOL CheckForAccessory(struct Port *port)
{
  UpdateVConnTermination(port);

  return TRUE;
}
#endif /* FSC_HAVE_ACC */

void set_policy_msg_tx_sop(struct Port *port, SopType sop)
{
  if ((sop == SOP_TYPE_SOP) || (sop == SOP_TYPE_SOP1))
    port->policy_msg_tx_sop_ = sop;
}

SopType DecodeSopFromPdMsg(FSC_U8 msg)
{
  switch (msg & 0b111) {
  case SOP_TYPE_SOP2: /* SOP'' */
    return SOP_TYPE_SOP2;
  case SOP_TYPE_SOP1: /* SOP' */
    return SOP_TYPE_SOP1;
  case SOP_TYPE_SOP:  /* SOP */
  default:
    return SOP_TYPE_SOP;
  }
}

void set_msg_id_count(struct Port *port, SopType sop, FSC_U32 count)
{
  if ((sop == SOP_TYPE_SOP) || (sop == SOP_TYPE_SOP1))
    port->message_id_counter_[sop] = count;
}

void set_message_id(struct Port *port, SopType sop, FSC_U32 id)
{
  if ((sop == SOP_TYPE_SOP) || (sop == SOP_TYPE_SOP1))
    port->message_id_[sop] = id;
}

static void ResetMessageIDs(struct Port *port, SopType sop)
{
  port->message_id_counter_[sop] = 0;
  port->message_id_[sop] = 0xFF;
  port->message_id_counter_[sop] = 0;
  port->message_id_[sop] = 0xFF;
}

void ResetProtocolLayer(struct Port *port, SopType sop)
{
  FSC_U8 i = 0;
  port->protocol_state_ = PRLIdle;
  port->pd_tx_status_ = txIdle;

  ResetMessageIDs(port, sop);

  if (sop == SOP_TYPE_SOP)
  {
#ifdef FSC_HAVE_VDM
  TimerDisable(&port->vdm_timer_);
#endif /* FSC_HAVE_VDM */

    port->protocol_msg_rx_ = FALSE;
    port->protocol_msg_rx_sop_ = SOP_TYPE_SOP;
    port->protocol_msg_tx_sop_ = SOP_TYPE_SOP;
    port->pd_tx_flag_ = FALSE;
    port->policy_has_contract_ = FALSE;

    port->waiting_on_hr_ = FALSE;

#ifdef FSC_HAVE_EXTENDED
    port->protocol_ext_num_bytes_ = 0;
#endif /* FSC_HAVE_EXTENDED */

#ifdef FSC_HAVE_USBHID
    /* Set the source caps updated flag to trigger an update of the GUI */
    port->source_caps_updated_ = TRUE;
#endif /* FSC_HAVE_USBHID */

    port->usb_pd_contract_.object = 0;
    port->caps_header_received_.word = 0;

    for (i = 0; i < 7; i++) {
      port->caps_received_[i].object = 0;
    }
  }

  DPM_ReConfigureRxDetect(port);
}

void PDEnable(struct Port *port, FSC_BOOL is_source)
{
  FSC_U8 i;
  port->is_hard_reset_ = FALSE;
  port->is_pr_swap_ = FALSE;
  port->hard_reset_counter_ = 0;
  port->renegotiate_ = FALSE;
  port->needs_goto_min_ = FALSE;
  port->policy_is_ams_ = FALSE;
  port->req_dr_swap_to_dfp_as_sink_ = Attempt_DR_Swap_to_Dfp_As_Sink;
  port->req_dr_swap_To_ufp_as_src_ = Attempt_DR_Swap_to_Ufp_As_Src;
  port->req_vconn_swap_to_on_as_sink_ = Attempt_Vconn_Swap_to_On_As_Sink;
  port->req_vconn_swap_to_off_as_src_ = Attempt_Vconn_Swap_to_Off_As_Src;
  port->req_pr_swap_as_src_ = Requests_PR_Swap_As_Src;
  port->req_pr_swap_as_snk_ = Requests_PR_Swap_As_Snk;
#ifdef FSC_HAVE_EXTENDED
  port->protocol_ext_request_chunk_ = FALSE;
  port->protocol_ext_send_chunk_ = FALSE;
  port->protocol_ext_state_active_ = FALSE;
#endif /* FSC_HAVE_EXTENDED */
  if (port->pd_enabled_ == TRUE) {
    if (port->cc_pin_ != NONE) {
      /* If we know what pin the CC signal is... */
      port->pd_active_ = TRUE;

      TimerDisable(&port->no_response_timer_);
      TimerDisable(&port->policy_state_timer_);
      TimerDisable(&port->policy_sinktx_timer_);
      TimerDisable(&port->swap_source_start_timer_);

      port->idle_ = FALSE;
      port->policy_is_source_ = is_source;
      port->policy_is_dfp_ = is_source;
      port->policy_subindex_ = 0;

      DPM_Initialize(port);
      /* Reset the protocol layer */
      for (i = SOP_TYPE_SOP; i < NUM_SOP_SUPPORTED; i++) {
        ResetProtocolLayer(port, i);
      }

      ClearInterrupt(port, regALERTL, MSK_I_TXSUCC | MSK_I_TXDISC |
          MSK_I_TXFAIL | MSK_I_RXHRDRST | MSK_I_RXSTAT);

      /* Set the initial data port direction */
      if (port->policy_is_source_) {
        set_policy_state(port, PE_SRC_Startup);

        /* Initialize as a source-DFP */
        port->registers_.MsgHeadr.POWER_ROLE = 1;
        port->registers_.MsgHeadr.DATA_ROLE = 1;
      }
      else {
        /* Policy is sink */
        set_policy_state(port, PE_SNK_Startup);

        /* Initialize as a sink-UFP */
        port->registers_.MsgHeadr.POWER_ROLE = 0;
        port->registers_.MsgHeadr.DATA_ROLE = 0;
      }

      WriteRegister(port, regMSGHEADR);

#ifdef FSC_LOGGING
      WritePDToken(&port->log_, TRUE, pdtAttach);
#endif /* FSC_LOGGING */
    }
#ifdef FSC_HAVE_DP
    DP_Initialize(port);
#endif /* FSC_HAVE_DP */
  }
}

void PDDisable(struct Port *port)
{
#ifdef FSC_LOGGING
  /* If we were previously active, store the PD detach token */
  if (port->pd_active_ == TRUE)
    WritePDToken(&port->log_, TRUE, pdtDetach);
#endif /* FSC_LOGGING */

#ifdef FSC_HAVE_USBHID
  /* Set the source caps updated flag to trigger an update of the GUI */
  port->source_caps_updated_ = TRUE;
#endif /* FSC_HAVE_USBHID */

  port->is_hard_reset_ = FALSE;
  port->pd_active_ = FALSE;
  port->protocol_state_ = PRLDisabled;
  set_policy_state(port, PE_SRC_Disabled);
  port->pd_tx_status_ = txIdle;
  port->policy_is_source_ = TRUE;
  port->policy_is_dfp_ = TRUE;
  port->policy_has_contract_ = FALSE;
  port->is_contract_valid_ = FALSE;

  TimerDisable(&port->policy_state_timer_);
  TimerDisable(&port->policy_sinktx_timer_);
  TimerDisable(&port->no_response_timer_);

  /* Disable PD in the device */
  port->registers_.RxDetect.byte = 0;
  WriteRegister(port, regRXDETECT);
#ifdef FSC_HAVE_DP
  platform_dp_enable_pins(FALSE, 0);
#endif /* FSC_HAVE_DP */
}

void set_sop_p_detect(struct Port *port, FSC_BOOL enable)
{
  /* Clear before write is needed for cable reset */
  port->registers_.RxDetect.byte = 0;
  WriteRegister(port, regRXDETECT);

  /* Enable PD messaging */
  port->registers_.RxDetect.EN_SOP = SOP_Capable;
  port->registers_.RxDetect.EN_HRD_RST = 1;
  if (enable == FALSE)
  {
    port->registers_.RxDetect.EN_SOP1 = 0;
    port->registers_.RxDetect.EN_SOP2 = 0;
    port->registers_.RxDetect.EN_SOP1_DBG = 0;
    port->registers_.RxDetect.EN_SOP2_DBG = 0;
  }
  else
  {
    port->registers_.RxDetect.EN_SOP1 = (SOP_P_Capable) ? 1 : 0;
    port->registers_.RxDetect.EN_SOP2 = (SOP_PP_Capable) ? 1 : 0;
    port->registers_.RxDetect.EN_SOP1_DBG = (SOP_P_Debug_Capable) ? 1 : 0;
    port->registers_.RxDetect.EN_SOP2_DBG = (SOP_PP_Debug_Capable) ? 1 : 0;
  }

  WriteRegister(port, regRXDETECT);
}

void set_policy_state(struct Port *port, PolicyState_t state)
{
  if (port->policy_state_ == state) return;

  if (port->dpm_pd_30_ && port->policy_is_ams_ &&
      (state == PE_SRC_Ready || state == PE_SNK_Ready)) {
    /* Should indicate that we are done with an AMS */
    port->policy_is_ams_ = FALSE;
    SetSinkTx(port, SinkTxOK);
  }

  port->policy_state_ = state;
  port->policy_subindex_ = 0;

  port->waiting_on_hr_ = FALSE;
  port->policy_wait_on_sink_caps_ = FALSE;

  platform_printf(port->port_id_, "PE SS ", state);

  if (state == PE_ErrorRecovery) port->idle_ = FALSE;

#ifdef FSC_LOGGING
  LogPEState(port);
#endif /* FSC_LOGGING */
}

void SetSinkTx(struct Port *port, SinkTxState_t state)
{
  port->policy_sinktx_state_ = state;

  UpdateSourceCurrent(port, (state == SinkTxOK) ? utcc3p0A : utcc1p5A);
}

#ifdef FSC_HAVE_FRSWAP
void EnableFRS_HubInitialSink(struct Port *port)
{
  /* FRS enable for the Hub port that is initially sinking power for the hub
   * and is then disconnected.
   */
  port->registers_.Gpio2Cfg.FR_SWAP_EN = 1;
  port->registers_.Gpio2Cfg.GPI2_EN = 0;
  port->registers_.Gpio2Cfg.GPO2_EN = 1;
  port->registers_.Gpio2Cfg.GPO2_VAL = 1;
  WriteRegister(port, regGPIO2_CFG);
}

void EnableFRS_HubInitialSource(struct Port *port)
{
  /* FRS enable for the Hub port that is the initial source for the FRS pair.
   * This is the port that initiates the CC-low signaling.
   */
  port->registers_.Gpio2Cfg.FR_SWAP_EN = 1;
  port->registers_.Gpio2Cfg.GPI2_EN = 1;
  port->registers_.Gpio2Cfg.GPO2_EN = 0;
  WriteRegister(port, regGPIO2_CFG);

  port->registers_.AlertVDMsk.M_SWAP_TX = 1;
  WriteRegister(port, regALERT_VD_MSK);
  ClearInterrupt(port, regALERT_VD, MSK_I_SWAP_TX);

  port->registers_.AlertMskH.M_VD_ALERT = 1;
  WriteRegister(port, regALERTMSKH);
  ClearInterrupt(port, regALERTH, MSK_I_VD_ALERT);
}

void EnableFRS_HostInitialSink(struct Port *port)
{
  /* FRS enable for the Hub port that is the initial sink for the FRS pair.
   * This is the port that detects the CC-low signaling.
   */
  port->registers_.SnkFRSwap.EN_FRSWAP_DTCT = 1;
  WriteRegister(port, regSNK_FRSWAP);

  port->registers_.AlertVDMsk.M_SWAP_RX = 1;
  WriteRegister(port, regALERT_VD_MSK);
  ClearInterrupt(port, regALERT_VD, MSK_I_SWAP_RX);

  port->registers_.AlertMskH.M_VD_ALERT = 1;
  port->registers_.AlertMskH.M_VBUS_SNK_DISC = 0;
  WriteRegister(port, regALERTMSKH);
  ClearInterrupt(port, regALERTH, MSK_I_VD_ALERT);
  ClearInterrupt(port, regALERTH, MSK_I_VBUS_SNK_DISC);

  /* Disable Auto-Discharge to prevent auto sink disconnect when VBUS drops */
  port->registers_.PwrCtrl.AUTO_DISCH = 0;
  WriteRegister(port, regPWRCTRL);

  /* ... and disable the sink disconnect threshold */
  SetVBusSnkDisc(port, 0);
}
#endif /* FSC_HAVE_FRSWAP */

#ifdef FSC_HAVE_VDM
/* VDM-specific functionality */
void set_vdm_msg_tx_sop(struct Port *port, SopType sop)
{
  if ((sop == SOP_TYPE_SOP) || (sop == SOP_TYPE_SOP1))
    port->vdm_msg_tx_sop_ = sop;
}
#endif /* FSC_HAVE_VDM */

void LogTCState(struct Port *port)
{
#ifdef FSC_LOGGING
  WriteTCState(&port->log_, platform_timestamp(), port->tc_state_);
#endif /* FSC_LOGGING */
}

void LogPEState(struct Port *port)
{
#ifdef FSC_LOGGING
  WritePEState(&port->log_, platform_timestamp(), port->policy_state_);
#endif /* FSC_LOGGING */
}

void PortPDReset(struct Port *port)
{
  ReadRegister(port, regRESET);
  ReadRegister(port, regMSGHEADR);
  ReadRegister(port, regRXDETECT);

  /* Before going back to PE_SNK_Ready reset the PD state machine. */
  port->registers_.Reset.byte = 0x2;

  port->registers_.MsgHeadr.POWER_ROLE = 0;
  port->registers_.MsgHeadr.DATA_ROLE = 0;

  /* Enable PD messaging */
  port->registers_.RxDetect.EN_SOP = 1;
  port->registers_.RxDetect.EN_HRD_RST = 1;

  /* Commit the configuration to the device */
  WriteRegister(port, regRESET);
  WriteRegister(port, regMSGHEADR);
  WriteRegister(port, regRXDETECT);
}

