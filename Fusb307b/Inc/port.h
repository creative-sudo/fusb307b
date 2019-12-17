/*******************************************************************************
 * @file     port.h
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
 * Defines the port implementation.
 * ***************************************************************************/
#ifndef FSCPM_PORT_H_
#define FSCPM_PORT_H_

#include "platform.h"
#include "TypeCTypes.h"
#include "PDTypes.h"
#include "DPMTypes.h"
#include "log.h"
#include "registers.h"
#include "timer.h"

#ifdef FSC_HAVE_VDM
#include "vdm_types.h"
#endif /* FSC_HAVE_VDM */

#ifdef FSC_HAVE_DP
#include "display_port_types.h"
#endif /* FSC_HAVE_DP */

/* Voltage levels given in 25mV resolution for vSafe0V and vSafe5V levels */
#define FSC_VSAFE0V       32    /* 0.8V  : vSafe0V */
#define FSC_VSAFE0V_DISCH 28    /* 0.7V  : Discharge slightly below vSafe5V */

#define FSC_VSAFE5V_DISC  146   /* 3.67V : Disconnect level */
#define FSC_VSAFE5V_L     190   /* 4.75V : vSafe5V - 5% */
#define FSC_VSAFE5V       200   /* 5.0V  : vSafe5V */
#define FSC_VSAFE5V_H     220   /* 5.5V  : vSafe5V + 10% */

/* Macro definitions of the disconnect voltage (80% of V) and the high/low
 * range (+/- 5%) representing an acceptable value for a given VBus level.
 * volts is expected to be an FSC_U16 or FSC_U32 type.
 * Result value is given in 25mV resolution to match hardware-reported values.
 */
/* TODO test this to verify values and make sure this is how we want to
 * represent these values in the code.
 */
#define FSC_VBUS_VSAFE0_V          800    /* Values in mV */
#define FSC_VBUS_03_V             3000
#define FSC_VBUS_05_V             5000
#define FSC_VBUS_09_V             9000
#define FSC_VBUS_12_V             12000
#define FSC_VBUS_15_V             15000
#define FSC_VBUS_20_V             20000

#define FSC_VBUS_LVL_DISC(mVolts)  ((mVolts - (mVolts / 5)) / 25)
#define FSC_VBUS_LVL_L(mVolts)     ((mVolts - (mVolts / 20)) / 25)
#define FSC_VBUS_LVL(mVolts)       (mVolts / 25)
#define FSC_VBUS_LVL_H(mVolts)     ((mVolts + (mVolts / 20)) / 25)

/* This one is used as an alarm high val when we only care about the low end */
#define FSC_VBUS_LVL_HIGHEST      (FSC_VBUS_LVL_H(FSC_VBUS_20_V))

/* This one returns the voltage in 50mV resolution for PD */
#define FSC_VBUS_LVL_PD(volts)    (volts / 50)
/*
 * The Port struct contains all port-related data and state information,
 * timer references, register map, etc.
 */
struct Port {
  FSC_U8 port_id_;                  /* Each port has an "ID", one indexed */
  FSC_U8 i2c_addr_;                 /* Assigned hardware I2C address */
  DeviceReg_t registers_;           /* Chip register object */
  FSC_BOOL idle_;                   /* If true, may give up processor */
  FSC_BOOL initialized_;            /* False until the INIT INT allows config */

  /* *** Timer Objects */
  struct TimerObj tc_state_timer_;
  struct TimerObj policy_state_timer_;
  struct TimerObj policy_sinktx_timer_;
  struct TimerObj cc_debounce_timer_;
  struct TimerObj pd_debounce_timer_;
  struct TimerObj no_response_timer_;
  struct TimerObj swap_source_start_timer_;
  struct TimerObj dpm_timer_;
  struct TimerObj pps_timer_;
  struct TimerObj protocol_timer_;

  /* *** Type-C (TC) port items */
  USBTypeCPort port_type_;           /* Src/Snk/DRP as config'd in the device */
  SourceOrSink source_or_sink_;      /* Src/Snk as current connection state */
  FSC_BOOL tc_enabled_;              /* TC state machine enabled */
  TypeCState tc_state_;              /* TC state machine current state */
  FSC_U8 tc_substate_;               /* TC state machine current sub-state */
  FSC_BOOL src_preferred_;           /* DRP, Src preferred */
  FSC_BOOL snk_preferred_;           /* DRP, Snk preferred */
  FSC_BOOL acc_support_;             /* Accessory support */
  USBTypeCCurrent snk_current_;      /* Current capability received */
  USBTypeCCurrent src_current_;      /* Current capability broadcasting */
  CCOrientation cc_pin_;             /* CC detected on CC1 or CC2 */
  CCTermType cc_term_;               /* Termination on CC pin */
  CCTermType vconn_term_;            /* Termination on VConn pin */
  CCTermType cc_term_previous_;      /* CC debounce status items */
  CCTermType cc_term_cc_debounce_;
  CCTermType cc_term_pd_debounce_;
  CCTermType cc_term_pd_debounce_prev_;
  FSC_BOOL is_hard_reset_;
#ifdef FSC_HAVE_FRSWAP
  FSC_BOOL is_fr_swap_;
#endif /* FSC_HAVE_FRSWAP */
  FSC_BOOL is_pr_swap_;
  FSC_BOOL is_vconn_swap_;
  FSC_U8 unattach_loop_counter_;
  FSC_U32 vbus_transition_time_;

  FSC_BOOL is_dead_battery_;

  FSC_BOOL have_sink_path_;           /* Sink path control */
  FSC_BOOL have_HV_path_;             /* HV Src path control */

  /* *** PD Protocol port items */
  FSC_BOOL pd_active_;                /* PD active during valid connection */
  FSC_BOOL pd_enabled_;               /* PD state machine enabled state */
  ProtocolState_t protocol_state_;
  PDTxStatus_t pd_tx_status_;
  FSC_BOOL pd_tx_flag_;
  SopType policy_msg_tx_sop_;
  FSC_BOOL req_dr_swap_to_dfp_as_sink_;        /* Request DR swap as sink */
  FSC_BOOL req_dr_swap_To_ufp_as_src_;         /* Request DR swap as source */
  FSC_BOOL req_vconn_swap_to_on_as_sink_;      /* Request Vconn swap */
  FSC_BOOL req_vconn_swap_to_off_as_src_;      /* Request Vconn swap */
  FSC_BOOL req_pr_swap_as_src_;               /* Request PR Swap as source */
  FSC_BOOL req_pr_swap_as_snk_;               /* Request PR Swap as sink */

  /*
   * message_id_ and message_id_counter_ to handle SOP/SOP'.
   * +1 to offset 0-indexed enum.
   * NOTE - If adding additional SOP types, make sure to update the array's
   * initializer loop condition to match!
   */
  FSC_U32 message_id_counter_[NUM_SOP_SUPPORTED];
  FSC_U32 message_id_[NUM_SOP_SUPPORTED];
  FSC_BOOL protocol_msg_rx_;
  SopType protocol_msg_rx_sop_;
  SopType protocol_msg_tx_sop_;
  FSC_U8 protocol_retries_;
  FSC_BOOL protocol_use_sinktx_;
  FSC_BOOL waiting_on_hr_;               /* Flag for hard reset shortcut */

#ifdef FSC_HAVE_EXTENDED
  FSC_U8 protocol_ext_buffer_[MAX_EXT_MSG_LEN];
  FSC_U16 protocol_ext_num_bytes_;
  FSC_U8 protocol_ext_chunk_number_;
  FSC_U8 protocol_ext_request_chunk_;
  FSC_U8 protocol_ext_state_active_;
  FSC_BOOL protocol_ext_send_chunk_;
  FSC_U8 protocol_ext_request_cmd_;
  FSC_BOOL protocol_chunking_supported_; /* Allow chunked messages */
#ifdef FSC_HAVE_SRC
  ExtSrcCapBlock_t policy_src_cap_ext_;
#endif /* FSC_HAVE_SRC */
#else
  FSC_BOOL wait_for_not_supported_;
  sopExtendedHeader_t protocol_ext_header_;
#endif /* FSC_HAVE_EXTENDED */

  /* *** PD Policy port items */
  PolicyState_t policy_state_;           /* Policy SM */
  FSC_U8 policy_subindex_;               /* Policy SM */
  FSC_BOOL policy_is_ams_;               /* Indicates start of Source AMS */
  SinkTxState_t policy_sinktx_state_;    /* Current SinkTx state */

  FSC_BOOL policy_is_source_;            /* Power Source or Sink */
  FSC_BOOL policy_is_dfp_;               /* Data Role DFP or UFP */
  FSC_BOOL is_contract_valid_;           /* Is PD Contract valid? */
  FSC_BOOL is_vconn_source_;             /* Sourcing VConn? */
  FSC_U8 collision_counter_;
  FSC_U8 hard_reset_counter_;            /* Track how many hard resets sent */
  FSC_U8 caps_counter_;                  /* Track how many caps messages sent */
  FSC_BOOL policy_has_contract_;         /* Contract in place? */
  FSC_BOOL renegotiate_;                 /* Signal to re-negotiate contract */
  FSC_BOOL policy_wait_on_sink_caps_;    /* Flag to req sink caps after delay */
  FSC_BOOL needs_goto_min_;              /* DPM requested goto min */
  sopMainHeader_t policy_rx_header_;     /* Header for PD messages received */
  sopMainHeader_t policy_tx_header_;     /* Header for PD messages to send */
  doDataObject_t policy_rx_data_obj_[7]; /* Buffer for data objects received */
  doDataObject_t policy_tx_data_obj_[7]; /* Buffer for data objects to send */
  sopMainHeader_t pd_transmit_header_;   /* PD packet to send */
  sopMainHeader_t caps_header_sink_;     /* Sink caps header */
  sopMainHeader_t caps_header_source_;   /* Source caps header */
  sopMainHeader_t caps_header_received_; /* Last capabilities header received */
  doDataObject_t pd_transmit_objects_[7];/* Data objects to send */
  doDataObject_t caps_sink_[7];          /* Power object defs of the snk caps */
  doDataObject_t caps_source_[7];        /* Power object defs of the src caps */
  doDataObject_t caps_received_[7];      /* Last power objects received */
  doDataObject_t usb_pd_contract_;       /* Current USB PD contract (req obj) */
  doDataObject_t sink_request_;          /* Sink request message */
  FSC_U32 sink_selected_voltage_;        /* Sink request voltage in mv */
  FSC_BOOL sink_transition_up_;          /* Voltage requested up or down */
  FSC_U32 sink_request_max_voltage_;     /* Max mv the sink will request */
  FSC_U32 sink_request_max_power_;       /* Max mw the sink will request */
  FSC_U32 sink_request_op_power_;        /* Op mw the snk will request */
  FSC_U32 sink_partner_max_power_;       /* Max mw advert'd by src partner */
  FSC_BOOL sink_request_low_power_;      /* Select the lower power PDO? */
  FSC_BOOL sink_goto_min_compatible_;    /* GotoMin command supported */
  FSC_BOOL sink_usb_suspend_compatible_; /* Sink suspend */
                                         /* operation during USB suspend */
  FSC_BOOL sink_usb_comm_capable_;       /* USB coms capable */
  doDataObject_t partner_caps_;          /* Partner's Capabilities */
  FSC_BOOL partner_caps_available_;

  FSC_U32 pd_HV_option_;                 /* Supported high voltage value (mv) */
  FSC_U8  pd_preferred_rev_;             /* Preferred PD revision to use */

  FSC_BOOL source_is_apdo_;
  doDataObject_t stored_apdo_;

  /* *** Device Policy Manager (DPM) items */
  FSC_BOOL dpm_active_;                  /* DPM initialized and active */
  FSC_U8 dpm_reject_count_;              /* DPM request reject counter */
  FSC_BOOL dpm_initial_connected_;       /* DPM first entry to dpmPDConnected */
  FSC_BOOL dpm_src_caps_ready_;          /* DPM updated source caps ready */
  FSC_BOOL dpm_request_valid_;           /* DPM requested PDO is valid */
  FSC_BOOL dpm_5a_possible_;             /* DPM Cable 5A capable */
  FSC_BOOL dpm_5a_capsent_;              /* DPM 5A caps sent, if available */
  FSC_BOOL dpm_disc_id_returned_;        /* DPM result of DiscID available */
  FSC_BOOL dpm_disc_id_result_;          /* DPM true == ACK */
  FSC_BOOL dpm_disc_id_done_;            /* DPM Cable DiscID process complete */
  FSC_U8 dpm_disc_id_count_;             /* DPM DiscID attempt counter */
  FSC_BOOL dpm_first_attach_;            /* DPM Gate for setting compat vers */
  FSC_BOOL dpm_pd_30_;                   /* DPM Src <-> Partner Spec Rev */
  FSC_BOOL dpm_pd_30_srccab_;            /* DPM Src <-> Cab Spec Rev */
  FSC_BOOL dpm_pd_20_cabchk_;            /* DPM Check cable for spec 2.0 */
  FSC_BOOL dpm_supply_ready_;            /* DPM state of supply */
  FSC_BOOL dpm_alert_;                   /* DPM alert flag */
  FSC_BOOL dpm_transition_0V_;           /* DPM signal for -> 0V -> 5V */

#ifdef FSC_HAVE_VDM
  /* VDM-specific data members */
  struct TimerObj vdm_timer_;            /* VDM-specific timer */
  PolicyState_t vdm_next_ps_;            /* Next VDM policy state */
  PolicyState_t original_policy_state_;
  FSC_BOOL vdm_expecting_response_;      /* True if expecting a VDM response */
  FSC_BOOL vdm_sending_data_;
  VdmDiscoveryState_t vdm_auto_state_;
  FSC_U32 vdm_msg_length_;
  doDataObject_t vdm_msg_obj_[7];
  SopType vdm_msg_tx_sop_;
  FSC_BOOL vdm_cbl_present_;             /* Avoid resets if querying SOP' id */
  FSC_BOOL vdm_check_cbl_;
  FSC_U16  discover_id_counter_;

  /* VDM handling data used by VDM handling example */
  FSC_BOOL svid_enable_;
  FSC_BOOL mode_enable_;
  FSC_U16 my_svid_;
  FSC_U32 my_mode_;
  FSC_BOOL mode_entered_;
  FSC_BOOL svid_discvry_done_;         /* Set when known svid is found */
  FSC_S16 svid_discv_idx_;             /* Index of svid to ask for modes */
  SvidInfo core_svid_info_;            /* Contains the most recent svid list */
  FSC_S8 auto_mode_entry_pos_;
  FSC_BOOL auto_mode_entry_enabled_;

#ifdef FSC_HAVE_DP
  DisplayPortData_t display_port_data_; /* struct containing relevant DP data */
#endif /* FSC_HAVE_DP */
#endif /* FSC_HAVE_VDM */
  CableResetState_t cbl_rst_state_;

#ifdef FSC_LOGGING
  /* Log object */
  struct Log log_;
#endif /* FSC_LOGGING */

#if defined(FSC_DEBUG) || defined(FSC_HAVE_USBHID)
  FSC_BOOL source_caps_updated_;  /* Signal GUI that source caps have changed */
#endif /* FSC_DEBUG || FSC_HAVE_USBHID */
}; /* struct Port */

/* Initialize the port and hardware interface. */
/* Note: Must be called after hardware setup is complete (including I2C coms) */
void InitializeVars(struct Port *port, FSC_U8 id, FSC_U8 i2c_addr_);
void InitializePort(struct Port *port);

/* Register Update Functions */
FSC_BOOL ReadRegister(struct Port *port, enum RegAddress regaddress);
FSC_BOOL ReadRegisters(struct Port *port, enum RegAddress regaddr, FSC_U8 cnt);
void ReadStatusRegisters(struct Port *port);
void ReadAllRegisters(struct Port *port);
void ReadRxRegisters(struct Port *port, FSC_U8 numbytes);
void WriteRegister(struct Port *port, enum RegAddress regaddress);
void WriteRegisters(struct Port *port, enum RegAddress regaddr, FSC_U8 cnt);
void WriteTxRegisters(struct Port *port, FSC_U8 numbytes);
void ClearInterrupt(struct Port *port, enum RegAddress address, FSC_U8 mask);
void SendCommand(struct Port *port, enum DeviceCommand cmd);

/* *** Type-C Functionality */
void SetRpValue(struct Port *port, USBTypeCCurrent currentVal);
void UpdateSourceCurrent(struct Port *port, USBTypeCCurrent currentVal);
void UpdateSinkCurrent(struct Port *port);

/* Return VBus voltage in millivolts */
FSC_U16 GetVBusVoltage(struct Port *port);
FSC_BOOL IsVbusInRange(struct Port *port, FSC_U16 mv);
FSC_BOOL IsVbusVSafe0V(struct Port *port);
FSC_BOOL IsVbusVSafe5V(struct Port *port);
FSC_BOOL IsVbusOverVoltage(struct Port *port, FSC_U16 mv);

/* Helper functions to set the level registers
 * level is in 25mV steps - call using macros defined above
 * e.g. SetVBusSnkDisc(port,FSC_VBUS_LVL_DISC(FSC_VBUS_05_V));
 */
void SetVBusSnkDisc(struct Port *port, FSC_U16 level);
void SetVBusStopDisc(struct Port *port, FSC_U16 level);
void SetVBusAlarm(struct Port *port, FSC_U16 levelL, FSC_U16 levelH);

#ifdef FSC_HAVE_SNK
FSC_BOOL IsVbusUnder5V(struct Port *port);
#endif /* FSC_HAVE_SNK */

CCTermType DecodeCCTermination(struct Port *port);

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
CCTermType DecodeCCTerminationSource(struct Port *port);
void SetStateSource(struct Port *port);
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACC) */

void SetVConn(struct Port *port, FSC_BOOL enable);
FSC_BOOL GetVConn(struct Port *port);

#ifdef FSC_HAVE_SNK
CCTermType DecodeCCTerminationSink(struct Port *port);
void SetStateSink(struct Port *port);
#endif /* FSC_HAVE_SNK */

void ResetDebounceVariables(struct Port *port);
void DebounceCC(struct Port *port);
void DetectCCPin(struct Port *port);
void UpdateVConnTermination(struct Port *port);
void UpdateOrientation(struct Port *port);
void ClearState(struct Port *port);

#ifdef FSC_HAVE_ACC
FSC_BOOL CheckForAccessory(struct Port *port);
#endif /* FSC_HAVE_ACC */

void set_policy_msg_tx_sop(struct Port *port, SopType sop);
SopType DecodeSopFromPdMsg(FSC_U8 msg);

void set_msg_id_count(struct Port *port, SopType sop, FSC_U32 count);
void set_message_id(struct Port *port, SopType sop, FSC_U32 id);

void PDDisable(struct Port *port);
void PDEnable(struct Port *port, FSC_BOOL is_source);

void ResetProtocolLayer(struct Port *port, SopType sop);
/* Enable or disable sop' and sop'' communication */
void set_sop_p_detect(struct Port *port, FSC_BOOL enable);

/* Policy helpers */
void set_policy_state(struct Port *port, PolicyState_t state);

void SetSinkTx(struct Port *port, SinkTxState_t state);

#ifdef FSC_HAVE_FRSWAP
void EnableFRS_HubInitialSink(struct Port *port);
void EnableFRS_HubInitialSource(struct Port *port);
void EnableFRS_HostInitialSink(struct Port *port);
#endif /* FSC_HAVE_FRSWAP */

/* DPM helpers */
void set_dpm_state(struct Port *port, DPMState_t state);

#ifdef FSC_HAVE_VDM
/* VDM-specific functionality */
void set_vdm_msg_tx_sop(struct Port *port, SopType sop);
#endif /* FSC_HAVE_VDM */

/* Logging */
void LogTCState(struct Port *port);
void LogPEState(struct Port *port);
void PortPDReset(struct Port *port);

#endif /* FSCPM_REGISTERS_H_ */
