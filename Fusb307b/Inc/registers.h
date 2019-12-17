/*******************************************************************************
 * @file     registers.h
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
 * Defines register mappings for the FUSB307.
 * ************************************************************************** */
#ifndef FSCPM_REGISTERS_H_
#define FSCPM_REGISTERS_H_

#include "platform.h"

/* Total number of registers in the device */
#define TOTAL_REGISTER_CNT 123

/* Rx/Tx buffer length in bytes */
#define COMM_BUFFER_LENGTH 28

/* Product ID definitions used for register PRODIDL */
enum ProductID {
  ProdID_None = 0x00,
  ProdID_307  = 0x33
};

/* Interrupt bit masks, organized by register */
/* Interrupt register masks: ALERTL */
#define MSK_I_CCSTAT          1
#define MSK_I_PORT_PWR        (1 << 1)
#define MSK_I_RXSTAT          (1 << 2)
#define MSK_I_RXHRDRST        (1 << 3)
#define MSK_I_TXFAIL          (1 << 4)
#define MSK_I_TXDISC          (1 << 5)
#define MSK_I_TXSUCC          (1 << 6)
#define MSK_I_VBUS_ALRM_HI    (1 << 7)
#define MSK_I_ALARM_LO_ALL    0xFF

/* Interrupt register masks: ALERTH */
#define MSK_I_VBUS_ALRM_LO    1
#define MSK_I_FAULT           (1 << 1)
#define MSK_I_RX_FULL         (1 << 2)
#define MSK_I_VBUS_SNK_DISC   (1 << 3)
#define MSK_I_VD_ALERT        (1 << 7)
#define MSK_I_ALARM_HI_ALL    0x8F

/* Interrupt register masks: ALERT_VD */
#define MSK_I_SWAP_RX         1
#define MSK_I_SWAP_TX         (1 << 1)
#define MSK_I_OTP             (1 << 2)
#define MSK_I_VDD_DTCT        (1 << 3)
#define MSK_I_GPI1            (1 << 4)
#define MSK_I_GPI2            (1 << 5)
#define MSK_I_DISCH_SUCC      (1 << 6)
#define MSK_I_ALERT_VD_ALL    0x7F

/* FaultStat register masks: FAULTSTAT */
#define MSK_I2C_ERROR         1
#define MSK_VCONN_OCP         (1 << 1)
#define MSK_FORCE_DISCH_FAIL  (1 << 4)
#define MSK_AUTO_DISCH_FAIL   (1 << 5)
#define MSK_ALL_REGS_RESET    (1 << 7)
#define MSK_FAULTSTAT_ALL     0x33

/* Measurement values */
#define SDAC_DEFAULT          0x20
#define SDAC_HYS_DEFAULT      0x01  /* 85mV */

/* List of valid values to write to regCOMMAND */
enum DeviceCommand {
  WakeI2C             = 0b00010001,
  DisableVbusDetect   = 0b00100010,
  EnableVbusDetect    = 0b00110011,
  DisableSinkVbus     = 0b01000100,
  SinkVbus            = 0b01010101,
  DisableSourceVbus   = 0b01100110,
  SourceVbusDefaultV  = 0b01110111,
  SourceVbusHighV     = 0b10001000,
  Look4Con            = 0b10011001,
  RxOneMore           = 0b10101010,
  I2CIdle             = 0b11111111
};

/* TRANSMIT register command values */
#define TRANSMIT_HARDRESET      0b101
#define TRANSMIT_CABLERESET     0b110
#define TRANSMIT_BIST_CM2       0b111

/* Register map for the FUSB307 */
enum RegAddress {
  regVENDIDL                = 0x00,
  regVENDIDH                = 0x01,
  regPRODIDL                = 0x02,
  regPRODIDH                = 0x03,
  regDEVIDL                 = 0x04,
  regDEVIDH                 = 0x05,
  regTYPECREVL              = 0x06,
  regTYPECREVH              = 0x07,
  regUSBPDVER               = 0x08,
  regUSBPDREV               = 0x09,
  regPDIFREVL               = 0x0A,
  regPDIFREVH               = 0x0B,
  /* 0x0C - 0x0F Reserved */

  regALERTL                 = 0x10,
  regALERTH                 = 0x11,
  regALERTMSKL              = 0x12,
  regALERTMSKH              = 0x13,
  regPWRSTATMSK             = 0x14,
  regFAULTSTATMSK           = 0x15,
  /* 0x16 - 0x17 Reserved */
  regSTD_OUT_CFG            = 0x18,
  regTCPC_CTRL              = 0x19,
  regROLECTRL               = 0x1A,
  regFAULTCTRL              = 0x1B,
  regPWRCTRL                = 0x1C,
  regCCSTAT                 = 0x1D,
  regPWRSTAT                = 0x1E,
  regFAULTSTAT              = 0x1F,

  /* 0x20 - 0x22 Reserved */
  regCOMMAND                = 0x23,
  regDEVCAP1L               = 0x24,
  regDEVCAP1H               = 0x25,
  regDEVCAP2L               = 0x26,
  regDEVCAP2H               = 0x27, /* Reserved */
  regSTD_IN_CAP             = 0x28, /* Reserved */
  regSTD_OUT_CAP            = 0x29,
  /* 0x2A - 0x2D Reserved */
  regMSGHEADR               = 0x2E,
  regRXDETECT               = 0x2F,

  regRXBYTECNT              = 0x30,
  regRXSTAT                 = 0x31,
  regRXHEADL                = 0x32,
  regRXHEADH                = 0x33,
  regRXDATA_00              = 0x34,
  regRXDATA_01              = 0x35,
  regRXDATA_02              = 0x36,
  regRXDATA_03              = 0x37,
  regRXDATA_04              = 0x38,
  regRXDATA_05              = 0x39,
  regRXDATA_06              = 0x3A,
  regRXDATA_07              = 0x3B,
  regRXDATA_08              = 0x3C,
  regRXDATA_09              = 0x3D,
  regRXDATA_10              = 0x3E,
  regRXDATA_11              = 0x3F,

  regRXDATA_12              = 0x40,
  regRXDATA_13              = 0x41,
  regRXDATA_14              = 0x42,
  regRXDATA_15              = 0x43,
  regRXDATA_16              = 0x44,
  regRXDATA_17              = 0x45,
  regRXDATA_18              = 0x46,
  regRXDATA_19              = 0x47,
  regRXDATA_20              = 0x48,
  regRXDATA_21              = 0x49,
  regRXDATA_22              = 0x4A,
  regRXDATA_23              = 0x4B,
  regRXDATA_24              = 0x4C,
  regRXDATA_25              = 0x4D,
  regRXDATA_26              = 0x4E,
  regRXDATA_27              = 0x4F,

  regTRANSMIT               = 0x50,
  regTXBYTECNT              = 0x51,
  regTXHEADL                = 0x52,
  regTXHEADH                = 0x53,
  regTXDATA_00              = 0x54,
  regTXDATA_01              = 0x55,
  regTXDATA_02              = 0x56,
  regTXDATA_03              = 0x57,
  regTXDATA_04              = 0x58,
  regTXDATA_05              = 0x59,
  regTXDATA_06              = 0x5A,
  regTXDATA_07              = 0x5B,
  regTXDATA_08              = 0x5C,
  regTXDATA_09              = 0x5D,
  regTXDATA_10              = 0x5E,
  regTXDATA_11              = 0x5F,

  regTXDATA_12              = 0x60,
  regTXDATA_13              = 0x61,
  regTXDATA_14              = 0x62,
  regTXDATA_15              = 0x63,
  regTXDATA_16              = 0x64,
  regTXDATA_17              = 0x65,
  regTXDATA_18              = 0x66,
  regTXDATA_19              = 0x67,
  regTXDATA_20              = 0x68,
  regTXDATA_21              = 0x69,
  regTXDATA_22              = 0x6A,
  regTXDATA_23              = 0x6B,
  regTXDATA_24              = 0x6C,
  regTXDATA_25              = 0x6D,
  regTXDATA_26              = 0x6E,
  regTXDATA_27              = 0x6F,

  regVBUS_VOLTAGE_L         = 0x70,
  regVBUS_VOLTAGE_H         = 0x71,
  regVBUS_SNK_DISCL         = 0x72,
  regVBUS_SNK_DISCH         = 0x73,
  regVBUS_STOP_DISCL        = 0x74,
  regVBUS_STOP_DISCH        = 0x75,
  regVALARMHCFGL            = 0x76,
  regVALARMHCFGH            = 0x77,
  regVALARMLCFGL            = 0x78,
  regVALARMLCFGH            = 0x79,
  /* 0x7A - 0x7F Reserved */

  regVCONN_OCP              = 0xA0,
  regSLICE                  = 0xA1,
  regRESET                  = 0xA2,
  regVD_STAT                = 0xA3,
  regGPIO1_CFG              = 0xA4,
  regGPIO2_CFG              = 0xA5,
  regGPIO_STAT              = 0xA6,
  regDRPTOGGLE              = 0xA7,
  regTOGGLE_SM              = 0xA8,
  /* 0xA9 - 0xAF Reserved */

  regSINK_TRANSMIT          = 0xB0,
  regSRC_FRSWAP             = 0xB1,
  regSNK_FRSWAP             = 0xB2,
  regALERT_VD               = 0xB3,
  regALERT_VD_MSK           = 0xB4,
  regRPVAL_OVERRIDE         = 0xB5
};

/* Register unions */
typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 I_CCSTAT         : 1;
    FSC_U8 I_PORT_PWR       : 1;
    FSC_U8 I_RXSTAT         : 1;
    FSC_U8 I_RXHRDRST       : 1;
    FSC_U8 I_TXFAIL         : 1;
    FSC_U8 I_TXDISC         : 1;
    FSC_U8 I_TXSUCC         : 1;
    FSC_U8 I_VBUS_ALRM_HI   : 1;
  };
} regAlertL_t; /* R/WC */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 I_VBUS_ALRM_LO   : 1;
    FSC_U8 I_FAULT          : 1;
    FSC_U8 I_RX_FULL        : 1;
    FSC_U8 I_VBUS_SNK_DISC  : 1;
    FSC_U8 Reserved         : 3;
    FSC_U8 I_VD_ALERT       : 1;
  };
} regAlertH_t; /* R/WC */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 M_CCSTAT         : 1;
    FSC_U8 M_PORT_PWR       : 1;
    FSC_U8 M_RXSTAT         : 1;
    FSC_U8 M_RXHRDRST       : 1;
    FSC_U8 M_TXFAIL         : 1;
    FSC_U8 M_TX_DISC        : 1;
    FSC_U8 M_TXSUCC         : 1;
    FSC_U8 M_VBUS_ALRM_HI   : 1;
  };
} regAlertMskL_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 M_VBUS_ALRM_LO   : 1;
    FSC_U8 M_FAULT          : 1;
    FSC_U8 M_RX_FULL        : 1;
    FSC_U8 M_VBUS_SNK_DISC  : 1;
    FSC_U8 Reserved         : 3;
    FSC_U8 M_VD_ALERT       : 1;
  };
} regAlertMskH_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 M_SNKVBUS        : 1;
    FSC_U8 M_VCONN_VAL      : 1;
    FSC_U8 M_VBUS_VAL       : 1;
    FSC_U8 M_VBUS_VAL_EN    : 1;
    FSC_U8 M_SRC_VBUS       : 1;
    FSC_U8 M_SRC_HV         : 1;
    FSC_U8 M_INIT           : 1;
    FSC_U8 M_DEBUG_ACC      : 1;
  };
} regPwrStatMsk_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 M_I2C_ERR          : 1;
    FSC_U8 M_VCONN_OCP        : 1;
    FSC_U8 Reserved0          : 2;
    FSC_U8 M_FORCE_DISCH_FAIL : 1;
    FSC_U8 M_AUTO_DISCH_FAIL  : 1;
    FSC_U8 Reserved1          : 1;
    FSC_U8 M_ALL_REGS_RESET   : 1;
  };
} regFaultStatMsk_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 ORIENT             : 1;
    FSC_U8 Reserved_0         : 1;
    FSC_U8 MUX_CTRL           : 2;
    FSC_U8 Reserved_1         : 2;
    FSC_U8 DEBUG_ACC          : 1;
    FSC_U8 TRI_STATE          : 1;
  };
} regStdOutCfg_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 ORIENT             : 1;
    FSC_U8 BIST_TMODE         : 1;
    FSC_U8 I2C_CLK_STRETCH    : 2;  /* Read-only */
    FSC_U8 DEBUG_ACC_CTRL     : 1;
    FSC_U8 EN_WATCHDOG        : 1;
    /* [7..6] Reserved */
  };
} regTcpcCtrl_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 CC1_TERM           : 2;
    FSC_U8 CC2_TERM           : 2;
    FSC_U8 RP_VAL             : 2;
    FSC_U8 DRP                : 1;
    /* [7] Reserved */
  };
} regRoleCtrl_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VCONN_OCP_DIS      : 1;
    FSC_U8 Reserved           : 2;
    FSC_U8 DISCH_TIMER_DIS    : 1;
    /* [7..4] Reserved */
  };
} regFaultCtrl_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 EN_VCONN           : 1;
    FSC_U8 VCONN_PWR          : 1;
    FSC_U8 FORCE_DISCH        : 1;
    FSC_U8 EN_BLEED_DISCH     : 1;
    FSC_U8 AUTO_DISCH         : 1;
    FSC_U8 DIS_VALARM         : 1;
    FSC_U8 DIS_VBUS_MON       : 1;
    /* [7] Reserved */
  };
} regPwrCtrl_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 CC1_STAT           : 2;
    FSC_U8 CC2_STAT           : 2;
    FSC_U8 CON_RES            : 1;
    FSC_U8 LOOK4CON           : 1;
    /* [7..6] Reserved */
  };
} regCCStat_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 SNKVBUS            : 1;
    FSC_U8 VCONN_VAL          : 1;
    FSC_U8 VBUS_VAL           : 1;
    FSC_U8 VBUS_VAL_EN        : 1;
    FSC_U8 SOURCE_VBUS        : 1;
    FSC_U8 SOURCE_HV          : 1;
    FSC_U8 TCPC_INIT          : 1;
    FSC_U8 DEBUG_ACC          : 1;
  };
} regPwrStat_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 I2C_ERR            : 1; /* R/WC */
    FSC_U8 VCONN_OCP          : 1; /* R/WC */
    FSC_U8 Reserved0          : 2; /* Read-only */
    FSC_U8 FORCE_DISCH_FAIL   : 1; /* R/WC */
    FSC_U8 AUTO_DISCH_FAIL    : 1; /* R/WC */
    FSC_U8 Reserved1          : 1;
    FSC_U8 ALL_REGS_RESET     : 1;
  };
} regFaultStat_t; /* R/WC */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 SRC_VBUS           : 1;
    FSC_U8 SRC_HV             : 1;
    FSC_U8 SNK_VBUS           : 1;
    FSC_U8 SWITCH_VCONN       : 1;
    FSC_U8 SOP_SUPPORT        : 1;
    FSC_U8 ROLES_SUPPORT      : 3;
  };
} regDevCap1L_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 RP_SUPPORT         : 2;
    FSC_U8 VBUS_MEAS_ALRM     : 1;
    FSC_U8 FORCE_DIS          : 1;
    FSC_U8 BLEED_DIS          : 1;
    /* [7..5] Reserved */
  };
} regDevCap1H_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VCONN_FAULT_CAP    : 1;
    FSC_U8 VCONN_POWER_CAP    : 3;
    FSC_U8 VBUS_ALRM_LSB      : 2;
    FSC_U8 STOP_DISCH         : 1;
    FSC_U8 SNK_DISC_DETECT    : 1;
  };
} regDevCap2L_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 ORIENT_CAP         : 1;
    FSC_U8 CON_PRESENT        : 1;
    FSC_U8 MUX_CTRL_CAP       : 1;
    FSC_U8 ACTIVE_CABLE       : 1;
    FSC_U8 AUDIO_ACC          : 1;
    FSC_U8 VBUS_MON           : 1;
    FSC_U8 DEBUG_ACC          : 1;
    /* [7] Reserved */
  };
} regStdOutCap_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 POWER_ROLE         : 1;
    FSC_U8 USBPD_REV          : 2;
    FSC_U8 DATA_ROLE          : 1;
    FSC_U8 CABLE_PLUG         : 1;
    /* [7..5] Reserved */
  };
} regMsgHeadr_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 EN_SOP             : 1;
    FSC_U8 EN_SOP1            : 1;
    FSC_U8 EN_SOP2            : 1;
    FSC_U8 EN_SOP1_DBG        : 1;
    FSC_U8 EN_SOP2_DBG        : 1;
    FSC_U8 EN_HRD_RST         : 1;
    FSC_U8 EN_CABLE_RST       : 1;
    /* [7] Reserved */
  };
} regRxDetect_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 RX_SOP             : 3;
    FSC_U8 Reserved           : 5;
  };
} regRxStat_t; /* Read-Only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 TX_SOP             : 3;
    FSC_U8 Reserved           : 1;
    FSC_U8 RETRY_CNT          : 2;
    /* [7..6] Reserved */
  };
} regTransmit_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_V_LO          : 8;
  };
} regVBusVoltageL_t; /* Read-only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_V_HI          : 2;
    FSC_U8 VBUS_SCALE         : 2;
    /* [7..4] Reserved */
  };
} regVBusVoltageH_t; /* Read-only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_SNK_DISC_LO   : 8;
  };
} regVBusSnkDiscL_t;

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_SNK_DISC_HI   : 2;
    /* [7..2] Reserved */
  };
} regVBusSnkDiscH_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_VTH_HI        : 8;
  };
} regVBusStopDiscL_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_VTH_LO        : 2;
    /* [7..2] Reserved */
  };
} regVBusStopDiscH_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_VTH_LO        : 8;
  };
} regVAlarmHCfgL_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_VTH_HI        : 2;
    /* [7..2] Reserved */
  };
} regVAlarmHCfgH_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_VTH_LO        : 8;
  };
} regVAlarmLCfgL_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 VBUS_VTH_HI        : 2;
    /* [7..2] Reserved */
  };
} regVAlarmLCfgH_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 OCP_CUR            : 3;
    FSC_U8 OCP_RANGE          : 1;
    /* [7..4] Reserved */
  };
} regVConnOCP_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 SDAC               : 6;
    FSC_U8 SDAC_HYS           : 2;
  };
} regSlice_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 SW_RST             : 1;
    FSC_U8 PD_RST             : 1;
    /* [7..2] Reserved */
  };
} regReset_t; /* R/WC */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 PWR_STAT                : 4;
    /* [7..4] Reserved */
  };
} regVDStat_t; /* Read-only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 GPO1_EN            : 1;
    FSC_U8 GPI1_EN            : 1;
    FSC_U8 GPO1_VAL           : 1;
    /* [7..3] Reserved */
  };
} regGpio1Cfg_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 GPO2_EN            : 1;
    FSC_U8 GPI2_EN            : 1;
    FSC_U8 GPO2_VAL           : 1;
    FSC_U8 FR_SWAP_EN         : 1;
    /* [7..4] Reserved */
  };
} regGpio2Cfg_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 GPI1_VAL           : 1;
    FSC_U8 GPI2_VAL           : 1;
    /* [7..2] Reserved */
  };
} regGpioStat_t; /* Read-only */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 DRP_TOGGLE         : 2;
    /* [7..2] Reserved */
  };
} regDrpToggle_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 TOGGLE_SM          : 5;
    /* [7..5] Reserved */
  };
} regToggleSM_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 TX_SOP             : 3;
    FSC_U8 Reserved           : 1;
    FSC_U8 RETRY_CNT          : 2;
    FSC_U8 DIS_SNK_TX         : 1;
    /* [7] Reserved */
  };
} regSinkTransmit_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 FR_SWAP            : 1; /* R/WC */
    FSC_U8 MANUAL_SNK_EN      : 1; /* R/W */
    FSC_U8 FRSWAP_SNK_DELAY   : 2; /* R/W */
    /* [7..4] Reserved */
  };
} regSrcFRSwap_t; /* R/W(C) */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 EN_FRSWAP_DTCT     : 1; /* R/WC */
    /* [7..1] Reserved */
  };
} regSnkFRSwap_t; /* R/WC */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 I_SWAP_RX          : 1;
    FSC_U8 I_SWAP_TX          : 1;
    FSC_U8 I_OTP              : 1;
    FSC_U8 I_VDD_DTCT         : 1;
    FSC_U8 I_GPI1             : 1;
    FSC_U8 I_GPI2             : 1;
    FSC_U8 I_DISCH_SUCC       : 1;
    FSC_U8 Reserved           : 1;
  };
} regAlertVD_t; /* R/WC */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 M_SWAP_RX          : 1;
    FSC_U8 M_SWAP_TX          : 1;
    FSC_U8 M_OTP              : 1;
    FSC_U8 M_VDD_DTCT         : 1;
    FSC_U8 M_GPI1             : 1;
    FSC_U8 M_GPI2             : 1;
    FSC_U8 M_DISCH_SUCC       : 1;
    FSC_U8 Reserved           : 1;
  };
} regAlertVDMsk_t; /* R/W */

typedef union {
  FSC_U8 byte;
  struct {
    FSC_U8 RP_VAL2            : 2;
    FSC_U8 RP_VAL1            : 2;
    FSC_U8 EN_RP_OVR          : 1;
    /* [7..5] Reserved */
  };
} regRpValOverride_t; /* R/W */

typedef struct
{
  FSC_U8              VendIDL;
  FSC_U8              VendIDH;
  FSC_U8              ProdIDL;
  FSC_U8              ProdIDH;
  FSC_U8              DevIDL;
  FSC_U8              DevIDH;
  FSC_U8              TypeCRevL;
  FSC_U8              TypeCRevH;
  FSC_U8              USBPDVer;
  FSC_U8              USBPDRev;
  FSC_U8              PDIFRevL;
  FSC_U8              PDIFRevH;
  regAlertL_t         AlertL;
  regAlertH_t         AlertH;
  regAlertMskL_t      AlertMskL;
  regAlertMskH_t      AlertMskH;
  regPwrStatMsk_t     PwrStatMsk;
  regFaultStatMsk_t   FaultStatMsk;
  regStdOutCfg_t      StdOutCfg;
  regTcpcCtrl_t       TcpcCtrl;
  regRoleCtrl_t       RoleCtrl;
  regFaultCtrl_t      FaultCtrl;
  regPwrCtrl_t        PwrCtrl;
  regCCStat_t         CCStat;
  regPwrStat_t        PwrStat;
  regFaultStat_t      FaultStat;
  FSC_U8              Command;
  regDevCap1L_t       DevCap1L;
  regDevCap1H_t       DevCap1H;
  regDevCap2L_t       DevCap2L;
  regStdOutCap_t      StdOutCap;
  regMsgHeadr_t       MsgHeadr;
  regRxDetect_t       RxDetect;
  FSC_U8              RxByteCnt;
  regRxStat_t         RxStat;
  FSC_U8              RxHeadL;
  FSC_U8              RxHeadH;
  FSC_U8              RxData[COMM_BUFFER_LENGTH];
  regTransmit_t       Transmit;
  FSC_U8              TxByteCnt;
  FSC_U8              TxHeadL;
  FSC_U8              TxHeadH;
  FSC_U8              TxData[COMM_BUFFER_LENGTH];
  regVBusVoltageL_t   VBusVoltageL;
  regVBusVoltageH_t   VBusVoltageH;
  regVBusSnkDiscL_t   VBusSnkDiscL;
  regVBusSnkDiscH_t   VBusSnkDiscH;
  regVBusStopDiscL_t  VBusStopDiscL;
  regVBusStopDiscH_t  VBusStopDiscH;
  regVAlarmHCfgL_t    VAlarmHCfgL;
  regVAlarmHCfgH_t    VAlarmHCfgH;
  regVAlarmLCfgL_t    VAlarmLCfgL;
  regVAlarmLCfgH_t    VAlarmLCfgH;
  regVConnOCP_t       VConnOCP;
  regSlice_t          Slice;
  regReset_t          Reset;
  regVDStat_t         VDStat;
  regGpio1Cfg_t       Gpio1Cfg;
  regGpio2Cfg_t       Gpio2Cfg;
  regGpioStat_t       GpioStat;
  regDrpToggle_t      DrpToggle;
  regToggleSM_t       ToggleSM;
  regSinkTransmit_t   SinkTransmit;
  regSrcFRSwap_t      SrcFRSwap;
  regSnkFRSwap_t      SnkFRSwap;
  regAlertVD_t        AlertVD;
  regAlertVDMsk_t     AlertVDMsk;
  regRpValOverride_t  RpValOverride;
} DeviceReg_t;

/*
 * AddressToRegister
 *
 * Arguments:   registers - ptr to register object to get registers from
 *              address - Register Address
 * Return:      Pointer to the register struct byte value
 * Description: A shortcut for reads and writes.
 * Note:        The only registers not included here are reserved registers.
 */
FSC_U8 *AddressToRegister(DeviceReg_t *registers, enum RegAddress address);

/*
 * GetLocalRegisters
 *
 * Arguments:   registers - ptr to register object to get registers from
 *              data - ptr to output array, must be >= TOTAL_REGISTER_CNT bytes
 *              length - length of data array
 * Description: Records the contents of the local register structs into a
 *              buffer to pass up to the caller.
 * Note:        Data buffer length must be at least TOTAL_REGISTER_CNT bytes.
 *              Does not get values from reserved registers.
 */
void GetLocalRegisters(DeviceReg_t *registers, FSC_U8 *data, FSC_U32 length);

/*
 * RegGetRxData
 *
 * Arguments:   registers - ptr to register object to get data registers from
 *              data - ptr to output array
 *              length - length of data array and number of data bytes to get,
 *                up to a total of COMM_BUFFER_LENGTH.
 * Description: Records the contents of the local Rx data registers into a
 *              buffer to pass up to the caller.
 * Note:        Length may be > COMM_BUFFER_LENGTH, but only the first
 *              COMM_BUFFER_LENGTH bytes will be written.
 */
void RegGetRxData(DeviceReg_t *registers, FSC_U8 *data, FSC_U32 length);

/*
 * RegSetTxData
 *
 * Arguments:   registers - ptr to register object in which to set data registers
 *              data - ptr to input array
 *              length - Number of bytes to set, up to COMM_BUFFER_LENGTH
 * Description: Copies the contents of data (up to COMM_BUFFER_LENGTH bytes) to
 *              the Tx registers
 * Note:        Length may be > COMM_BUFFER_LENGTH, but only the first
 *              COMM_BUFFER_LENGTH bytes will be written.
 */
void RegSetTxData(DeviceReg_t *registers, FSC_U8 *data, FSC_U32 length);

/*
 * RegSetBits
 *
 * Arguments:   registers - ptr to register object
 *              address - register to set bits of
 *              mask - Bits to set
 * Description: Sets the bits indicated as 1's in mask in register address.
 */
void RegSetBits(DeviceReg_t *registers, enum RegAddress address, FSC_U8 mask);

/*
 * RegClearBits
 *
 * Arguments:   registers - ptr to register object
 *              address - register to set bits of
 *              mask - Bits to set
 * Description: Clears the bits indicated as 1's in mask in register address.
 */
void RegClearBits(DeviceReg_t *registers, enum RegAddress address, FSC_U8 mask);
#endif /*  FSCPM_REGISTERS_H_ */
