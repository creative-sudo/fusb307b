/*******************************************************************************
 * @file     PDTypes.h
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
#ifndef FSCPM_PDTYPES_H_
#define FSCPM_PDTYPES_H_

#include "platform.h"

#define NUM_SOP_SUPPORTED       3           /* SOP, SOP', SOP'' */

/* USB PD Extended Message Values */
#define MAX_MSG_LEGACY_LEN      28
#define MAX_EXT_MSG_LEGACY_LEN  26
#define MAX_EXT_MSG_LEN         260

#define STAT_BUSY               0
#define STAT_SUCCESS            1
#define STAT_ERROR              2
#define STAT_ABORT              3

/* USB PD Header Message Definitions */
#define PDPortRoleSink          0
#define PDPortRoleSource        1
#define PDSpecRev1p0            0x0
#define PDSpecRev2p0            0x1
#define PDSpecRev3p0            0x2
#define PDDataRoleUFP           0
#define PDDataRoleDFP           1
#define PDCablePlugSource       0
#define PDCablePlugPlug         1
#define PDMsgTypeMask           0x1F

#define USBPDSPECREV            PDSpecRev2p0  /* Revision 2.0 */


/* USB PD Control Message Types */
#define CMTGoodCRC              0b00001
#define CMTGotoMin              0b00010
#define CMTAccept               0b00011
#define CMTReject               0b00100
#define CMTPing                 0b00101
#define CMTPS_RDY               0b00110
#define CMTGetSourceCap         0b00111
#define CMTGetSinkCap           0b01000
#define CMTDR_Swap              0b01001
#define CMTPR_Swap              0b01010
#define CMTVCONN_Swap           0b01011
#define CMTWait                 0b01100
#define CMTSoftReset            0b01101
#define CMTNotSupported         0b10000
#define CMTGetSourceCapExt      0b10001
#define CMTGetSourceStatus      0b10010
#define CMTFR_Swap              0b10011
#define CMTGetPPSStatus         0b10100
#define CMTGetCountryCodes      0b10101

/* USB PD Data Message Types */
#define DMTSourceCapabilities   0b00001
#define DMTRequest              0b00010
#define DMTBIST                 0b00011
#define DMTSinkCapabilities     0b00100
#define DMTBatteryStatus        0b00101
#define DMTAlert                0b00110
#define DMTGetCountryInfo       0b00111
#define DMTVendorDefined        0b01111

/* Extended Message Types */
#ifdef FSC_HAVE_EXTENDED
#define EMTSourceCapsExtended   0x1
#define EMTStatus               0x2
#define EMTGetBatteryCap        0x3
#define EMTGetBatteryStatus     0x4
#define EMTBatteryStatus        0x5
#define EMTGetManufacturerInfo  0x6
#define EMTManufacturerInfo     0x7
#define EMTSecurityRequest      0x8
#define EMTSecurityResponse     0x9
#define EMTFWUpdateRequest      0xA
#define EMTFWUpdateResponse     0xB
#define EMTPPSStatus            0xC
#define EMTCountryInfo          0xD
#define EMTCountryCodes         0xE
#endif /* FSC_HAVE_EXTENDED */

/* BIST Data Objects */
#define BDO_BIST_Receiver_Mode      0b0000  /* Not Implemented */
#define BDO_BIST_Transmit_Mode      0b0001  /* Not Implemented */
#define BDO_Returned_BIST_Counters  0b0010  /* Not Implemented */
#define BDO_BIST_Carrier_Mode_0     0b0011  /* Not Implemented */
#define BDO_BIST_Carrier_Mode_1     0b0100  /* Not Implemented */
#define BDO_BIST_Carrier_Mode_2     0b0101  /* Implemented */
#define BDO_BIST_Carrier_Mode_3     0b0110  /* Not Implemented */
#define BDO_BIST_Eye_Pattern        0b0111  /* Not Implemented */
#define BDO_BIST_Test_Data          0b1000  /* Implemented */

#define HARD_RESET_COUNT        2
#define RETRIES_PD20            3
#define RETRIES_PD30            2
#define MAX_CAPS_COUNT          50

/* Maximum number of discover sop'. Actual number of message sent will need
 * to account for retries.
 */
#define MAX_DISC_ID_COUNT       18

/* PD Voltage values in 50mV resolution */
#define PD_05_V                 100
#define PD_09_V                 180
#define PD_12_V                 240
#define PD_15_V                 300
#define PD_20_V                 400

/* PD Current values in 10mA resolution */
#define PD_0_9_A                90
#define PD_1_5_A                150
#define PD_3_0_A                300

typedef union {
  FSC_U16 word;
  FSC_U8 byte[2] __PACKED;

  struct {
    unsigned MessageType:5;         /* 0-4      : Message Type */
    unsigned PortDataRole:1;        /* 5        : Port Data Role */
    unsigned SpecRevision:2;        /* 6-7      : Specification Revision */
    unsigned PortPowerRole:1;       /* 8        : Port Power Role */
    unsigned MessageID:3;           /* 9-11     : Message ID */
    unsigned NumDataObjects:3;      /* 12-14    : Number of Data Objects */
    unsigned Extended:1;            /* 15       : Extended Message Flag */
  };
} sopMainHeader_t;

typedef union {
  FSC_U16 word;
  FSC_U8 byte[2] __PACKED;
  struct {
    unsigned MessageType:5;         /* 0-4      : Message Type */
    unsigned Reserved:1;            /* 5        : Reserved */
    unsigned SpecRevision:2;        /* 6-7      : Specification Revision */
    unsigned CablePlug:1;           /* 8        : Cable Plug */
    unsigned MessageID:3;           /* 9-11     : Message ID */
    unsigned NumDataObjects:3;      /* 12-14    : Number of Data Objects */
    unsigned Extended:1;            /* 15       : Extended Message Flag */
  };
} sopPlugHeader_t;

typedef union {
  FSC_U32 object;
  FSC_U16 word[2] __PACKED;
  FSC_U8 byte[4] __PACKED;
  struct {
    unsigned :30;
    unsigned SupplyType:2;
  } PDO;                            /* General purpose Power Data Object */
  struct {
    unsigned MaxCurrent:10;         /* Max current in 10mA units */
    unsigned Voltage:10;            /* Voltage in 50mV units */
    unsigned PeakCurrent:2;         /* Peak I (divergent from Ioc ratings) */
    unsigned Reserved:3;            /* Reserved */
    unsigned DataRoleSwap:1;        /* Data role swap supported */
    unsigned USBCommCapable:1;      /* USB communications capable */
    unsigned ExternallyPowered:1;   /* Externally powered */
    unsigned USBSuspendSupport:1;   /* USB Suspend Supported */
    unsigned DualRolePower:1;       /* Dual-Role power  - supports PR swap */
    unsigned SupplyType:2;          /* (Fixed Supply) */
  } FPDOSupply;                     /* Fixed Power Data Object for Supplies */
  struct {
    unsigned OperationalCurrent:10; /* Operational current in 10mA units */
    unsigned Voltage:10;            /* Voltage in 50mV units */
    unsigned Reserved:5;            /* Reserved */
    unsigned DataRoleSwap:1;        /* Data role swap supported */
    unsigned USBCommCapable:1;      /* USB communications capable */
    unsigned ExternallyPowered:1;   /* Externally powered */
    unsigned HigherCapability:1;    /* Needs more than vSafe5V */
    unsigned DualRolePower:1;       /* Dual-Role power - supports PR swap */
    unsigned SupplyType:2;          /* (Fixed Supply) */
  } FPDOSink;                       /* Fixed Power Data Object for Sinks */
  struct {
    unsigned MaxCurrent:10;         /* Max current in 10mA units */
    unsigned MinVoltage:10;         /* Minimum Voltage in 50mV units */
    unsigned MaxVoltage:10;         /* Maximum Voltage in 50mV units */
    unsigned SupplyType:2;          /* (Variable Supply) */
  } VPDO;                           /* Variable Power Data Object */
  struct {
    unsigned MaxPower:10;           /* Max power in 250mW units */
    unsigned MinVoltage:10;         /* Minimum Voltage in 50mV units */
    unsigned MaxVoltage:10;         /* Maximum Voltage in 50mV units */
    unsigned SupplyType:2;          /* (Battery Supply) */
  } BPDO;                           /* Battery Power Data Object */
  struct {
    unsigned :28;                   /* APDO-Defined Bits */
    unsigned APDOType:2;            /* Augmented Type */
    unsigned SupplyType:2;          /* (Augmented PDO) */
  } APDO;                           /* Augmented Power Data Object */
  struct {
    unsigned MaxCurrent:7;          /* Max current in 50mA units */
    unsigned :1;                    /* Reserved */
    unsigned MinVoltage:8;          /* Min voltage in 100mV units */
    unsigned :1;                    /* Reserved */
    unsigned MaxVoltage:8;          /* Max voltage in 100mV units */
    unsigned :3;                    /* Reserved */
    unsigned APDOType:2;            /* (PPS: 0) */
    unsigned SupplyType:2;          /* (Augmented PDO) */
  } PPSAPDO;                        /* Programable Power Supply Data Object */
  struct {
    unsigned MinMaxCurrent:10;      /* Min/Max current in 10mA units */
    unsigned OpCurrent:10;          /* Operating current in 10mA units */
    unsigned Reserved0:4;           /* Reserved - set to zero */
    unsigned NoUSBSuspend:1;        /* Set when the sink wants to continue
                                     * the contract during USB suspend
                                     * (i.e. charging battery)
                                     */
    unsigned USBCommCapable:1;      /* USB communications capable */
    unsigned CapabilityMismatch:1;  /* Set if the sink cannot satisfy its
                                     * power requirements from caps offered
                                     */
    unsigned GiveBack:1;            /* Whether the sink will respond to
                                     * the GotoMin message
                                     */
    unsigned ObjectPosition:3;      /* Index of source cap being requested */
    unsigned Reserved1:1;           /* Reserved - set to zero */
  } FVRDO;                          /* Fixed/Variable Request Data Object */
  struct {
    unsigned MinMaxPower:10;        /* Min/Max power in 250mW units */
    unsigned OpPower:10;            /* Operating power in 250mW units */
    unsigned Reserved0:4;           /* Reserved - set to zero */
    unsigned NoUSBSuspend:1;        /* Set when the sink wants to continue
                                     * the contract during USB suspend
                                     * (i.e. charging battery)
                                     */
    unsigned USBCommCapable:1;      /* USB communications capable */
    unsigned CapabilityMismatch:1;  /* Set if the sink cannot satisfy its
                                     * power requirements from caps offered
                                     */
    unsigned GiveBack:1;            /* Whether the sink will respond to the
                                     * GotoMin message
                                     */
    unsigned ObjectPosition:3;      /* Index of source cap being requested */
    unsigned Reserved1:1;           /* Reserved - set to zero */
  } BRDO;                           /* Battery Request Data Object */
  struct {
    unsigned OpCurrent:7;           /* Operating current in 50mA units */
    unsigned :2;
    unsigned OpVoltage:11;          /* Requested voltage in 20mV units */
    unsigned :4;
    unsigned NoUSBSuspend:1;        /* Set when the sink wants to continue
                                     * the contract during USB suspend
                                     * (i.e. charging battery)
                                     */
    unsigned USBCommCapable:1;      /* USB communications capable */
    unsigned CapabilityMismatch:1;  /* Set if the sink cannot satisfy its
                                     * power requirements from caps offered
                                     */
    unsigned :1;
    unsigned ObjectPosition:3;      /* Index of source cap being requested */
    unsigned :1;
  } PPSRDO;                         /* PPS Request Data Object */
  struct {
    unsigned VendorDefined:15;      /* Defined by the vendor */
    unsigned VDMType:1;             /* Unstructured or structured msg header */
    unsigned VendorID:16;           /* Unique 16-bit unsigned integer
                                     * assigned by the USB-IF
                                     */
  } UVDM;
  struct {
    unsigned Command:5;
    unsigned Reserved0:1;           /* Reserved - set to zero */
    unsigned CommandType:2;         /* Init, ACK, NAK, BUSY... */
    unsigned ObjPos:3;
    unsigned Reserved1:2;           /* Reserved - set to zero */
    unsigned Version:2;             /* Structured VDM version */
    unsigned VDMType:1;             /* Unstructured or structured msg header */
    unsigned SVID:16;               /* Unique 16-bit unsigned integer
                                     * assigned by the USB-IF
                                     */
  } SVDM;
} doDataObject_t;


typedef union {
  FSC_U16 word;
  FSC_U8 byte[2] __PACKED;

  struct {
    unsigned DataSize:9;            /* 0-8      : Full Message Length */
    unsigned Reserved:1;            /* 9        : Message ID */
    unsigned RequestChunk:1;        /* 10       : Request Chunk */
    unsigned ChunkNumber:4;         /* 11-14    : Chunk Number */
    unsigned Chunked:1;             /* 15       : Chunked Message Flag */
  };
} sopExtendedHeader_t;

#ifdef FSC_HAVE_EXTENDED
typedef union {
  FSC_U8 byte[23] __PACKED;

  struct {                          /* Byte Offsets */
    unsigned VID:16;                /* 0-1      : Vendor ID */
    unsigned PID:16;                /* 2-3      : Product ID */
    unsigned XID:32;                /* 4-7      : USB-IF extended ID */
    unsigned FWVersion:8;           /* 8        : Firmware Version */
    unsigned HWVersion:8;           /* 9        : Hardware Version */
    unsigned VotageReg:8;           /* 10       : Bitfield (See Spec) */
    unsigned HoldupTime:8;          /* 11       : Output Hold Time */
    unsigned Compliance:8;          /* 12       : Bitfield (See spec) */
    unsigned TouchCurrent:8;        /* 13       : Bitfield (See spec) */
    unsigned PeakCurrent1:16;       /* 14-15    : Bitfield (See spec) */
    unsigned PeakCurrent2:16;       /* 16-17    : Bitfield (See spec) */
    unsigned PeakCurrent3:16;       /* 18-19    : Bitfield (See spec) */
    unsigned TouchTemp:8;           /* 20       : Temp Spec */
    unsigned SourceInputs:8;        /* 21       : Bitfield (See spec) */
    unsigned Batteries:8;           /* 22       : Battery Count */
    unsigned SrcPDP:8;              /* 23       : Source's PDP */
  };
} ExtSrcCapBlock_t;

typedef union {
  FSC_U8 byte[3] __PACKED;

  struct {                          /* Byte Offsets */
    unsigned Temp:8;                /* 0        : Internal Temp (Deg C) 0=NA */
    unsigned Input:8;               /* 1        : Bitfield (See spec) */
    unsigned BatteryInput:8;        /* 2        : Battery Index */
  };
} ExtStatusDataBlock_t;

typedef union {
  FSC_U8 byte[9] __PACKED;

  struct {                          /* Byte Offsets */
    unsigned VID:16;                /* 0-1      : Vendor ID */
    unsigned PID:16;                /* 2-3      : Product ID */
    unsigned DesignCap:16;          /* 4-5      : Design Capacity (1/10 WH) */
    unsigned LastFullCap:16;        /* 6-7      : Full Charge Cap (1/10 WH) */
    unsigned Type:8;                /* 8        : Battery Type */
                                    /*            1: Hot Swappable, 0: Not. */
  };
} ExtBatteryCapBlock_t;

typedef union {
  FSC_U8 byte[2] __PACKED;

  struct {                          /* Byte Offsets */
    unsigned Target:8;              /* 0        : 0: Plug, 1: Battery */
    unsigned Reference:8;           /* 1        : Battery Index */
  };
} ExtMfrInfoReqBlock_t;

typedef union {
  FSC_U8 byte[27] __PACKED;

  struct {                          /* Byte Offsets */
    unsigned VID:16;                /* 0-1      : Vendor ID */
    unsigned PID:16;                /* 2-3      : Product ID */
    unsigned MfrString:8;           /* 4-26     : Vendor defined byte array */
  };
} ExtMfrInfoBlock_t;

typedef union {
  FSC_U32 object;
  FSC_U16 word[2];
  FSC_U8 byte[4];
  struct
  {
    FSC_U32 OutputVoltage :16;      /* Bit[0-15] Output Voltage (20mV) */
    FSC_U32 OutputCurrent :8;       /* Bit[0-7] Output Current (50mA) */
    FSC_U32 :4;                     /* Bit[4-7] */
    FSC_U32 FlagOMF :1;             /* Bit[3] Operating Mode Flag */
    FSC_U32 FlagPTF :2;             /* Bit[1-2] Present Temp Flag */
    FSC_U32 :1;                     /* Bit[0] */
  };
} PPSStatus_t;
#endif /* FSC_HAVE_EXTENDED */

typedef enum {
  pdtNone = 0,                /* Reserved token (nothing) */
  pdtAttach,                  /* USB PD attached */
  pdtDetach,                  /* USB PD detached */
  pdtHardReset,               /* USB PD hard reset */
  pdtBadMessageID,            /* An incorrect message ID was received */
  pdtCableReset,              /* Cable reset */
  pdtHardResetTxd,            /* USB PD hard reset */
  pdtHardResetRxd,            /* USB PD hard reset */
} USBPD_BufferTokens_t;

typedef enum {
  // Source Port
  PE_SRC_Startup,
  PE_SRC_Discovery,
  PE_SRC_Send_Capabilities,
  PE_SRC_Negotiate_Capability,
  PE_SRC_Transition_Supply,
  PE_SRC_Ready,
  PE_SRC_Disabled,
  PE_SRC_Capability_Response,
  PE_SRC_Hard_Reset,
  PE_SRC_Hard_Reset_Received,
  PE_SRC_Transition_To_Default,
  PE_SRC_Get_Sink_Cap,
  PE_SRC_Wait_New_Capabilities,
  // Sink Port
  PE_SNK_Startup,
  PE_SNK_Discovery,
  PE_SNK_Wait_For_Capabilities,
  PE_SNK_Evaluate_Capability,
  PE_SNK_Select_Capability,
  PE_SNK_Transition_Sink,
  PE_SNK_Ready,
  PE_SNK_Hard_Reset,
  PE_SNK_Transition_To_Default,
  PE_SNK_Give_Sink_Cap,
  PE_SNK_Get_Source_Cap,
  // Soft Reset and Protocol Error
  PE_SRC_Send_Soft_Reset,
  PE_SRC_Soft_Reset,
  PE_SNK_Send_Soft_Reset,
  PE_SNK_Soft_Reset,
  // Not Supported Message
  PE_SRC_Send_Not_Supported,
  PE_SRC_Not_Supported_Received,
  PE_SRC_Chunk_Received,
  PE_SNK_Send_Not_Supported,
  PE_SNK_Not_Supported_Received,
  PE_SNK_Chunk_Received,
  // Source Port Ping
  PE_SRC_Ping,
  // Source/Sink Alert
  PE_SRC_Send_Source_Alert,
  PE_SNK_Source_Alert_Received,
  PE_SNK_Send_Sink_Alert,
  PE_SRC_Sink_Alert_Received,
  // Source Extended Capabilities
  PE_SNK_Get_Source_Cap_Ext,
  PE_SRC_Give_Source_Cap_Ext,
  // Sink/Source Status
  PE_SNK_Get_Source_Status,
  PE_SRC_Give_Source_Status,
  PE_SRC_Get_Sink_Status,
  PE_SNK_Give_Sink_Status,
  // PPS Status
  PE_SNK_Get_PPS_Status,
  PE_SRC_Give_PPS_Status,
  // Battery Capabilities
  PE_Get_Battery_Cap,
  PE_Give_Battery_Cap,
  // Battery Status
  PE_Get_Battery_Status,
  PE_Give_Battery_Status,
  // Manufacturer Information
  PE_Get_Manufacturer_Info,
  PE_Give_Manufacturer_Info,
  // Country Codes and Information
  PE_Get_Country_Codes,
  PE_Give_Country_Codes,
  PE_Get_Country_Info,
  PE_Give_Country_Info,
  // Security Request/Response
  PE_Send_Security_Request,
  PE_Send_Security_Response,
  PE_Security_Response_Received,
  // Firmware Update Request/Response
  PE_Send_Firmware_Update_Request,
  PE_Send_Firmware_Update_Response,
  PE_Firmware_Update_Response_Received,
  // Data Role Swap
  PE_DRS_DFP_UFP_Evaluate_Swap,
  PE_DRS_DFP_UFP_Accept_Swap,
  PE_DRS_DFP_UFP_Change_to_UFP,
  PE_DRS_DFP_UFP_Send_Swap,
  PE_DRS_DFP_UFP_Reject_Swap,
  PE_DRS_UFP_DFP_Evaluate_Swap,
  PE_DRS_UFP_DFP_Accept_Swap,
  PE_DRS_UFP_DFP_Change_to_DFP,
  PE_DRS_UFP_DFP_Send_Swap,
  PE_DRS_UFP_DFP_Reject_Swap,
  // Power Role Swap
  PE_PRS_SRC_SNK_Evaluate_Swap,
  PE_PRS_SRC_SNK_Accept_Swap,
  PE_PRS_SRC_SNK_Transition_to_off,
  PE_PRS_SRC_SNK_Assert_Rd,
  PE_PRS_SRC_SNK_Wait_Source_on,
  PE_PRS_SRC_SNK_Send_Swap,
  PE_PRS_SRC_SNK_Reject_Swap,
  PE_PRS_SNK_SRC_Evaluate_Swap,
  PE_PRS_SNK_SRC_Accept_Swap,
  PE_PRS_SNK_SRC_Transition_to_off,
  PE_PRS_SNK_SRC_Assert_Rp,
  PE_PRS_SNK_SRC_Source_on,
  PE_PRS_SNK_SRC_Send_Swap,
  PE_PRS_SNK_SRC_Reject_Swap,
  // Fast Role Swap
  PE_FRS_SRC_SNK_CC_Signal,
  PE_FRS_SRC_SNK_Evaluate_Swap,
  PE_FRS_SRC_SNK_Accept_Swap,
  PE_FRS_SRC_SNK_Transition_to_off,
  PE_FRS_SRC_SNK_Assert_Rd,
  PE_FRS_SRC_SNK_Wait_Source_on,
  PE_FRS_SNK_SRC_Start_AMS,
  PE_FRS_SNK_SRC_Send_Swap,
  PE_FRS_SNK_SRC_Transition_to_off,
  PE_FRS_SNK_SRC_Vbus_Applied,
  PE_FRS_SNK_SRC_Assert_Rp,
  PE_FRS_SNK_SRC_Source_on,
  // DRP Capabilities
  PE_DR_SRC_Get_Source_Cap,
  PE_DR_SRC_Give_Sink_Cap,
  PE_DR_SNK_Get_Sink_Cap,
  PE_DR_SNK_Give_Source_Cap,
  PE_DR_SRC_Get_Source_Cap_Ext,
  PE_DR_SNK_Give_Source_Cap_Ext,
  // USB Type-C VCONN Swap
  PE_VCS_Send_Swap,
  PE_VCS_Evaluate_Swap,
  PE_VCS_Accept_Swap,
  PE_VCS_Reject_Swap,
  PE_VCS_Wait_For_VCONN,
  PE_VCS_Turn_Off_VCONN,
  PE_VCS_Turn_On_VCONN,
  PE_VCS_Send_Ps_Rdy,
  // Initiator Structured VDM
  PE_INIT_PORT_VDM_Identity_Request,
  FIRST_VDM_STATE = PE_INIT_PORT_VDM_Identity_Request,
  PE_INIT_PORT_VDM_Identity_ACKed,
  PE_INIT_PORT_VDM_Identity_NAKed,
  PE_INIT_VDM_SVIDs_Request,
  PE_INIT_VDM_SVIDs_ACKed,
  PE_INIT_VDM_SVIDs_NAKed,
  PE_INIT_VDM_Modes_Request,
  PE_INIT_VDM_Modes_ACKed,
  PE_INIT_VDM_Modes_NAKed,
  PE_INIT_VDM_Attention_Request,
  // Responder Structured VDM
  PE_RESP_VDM_Get_Identity,
  PE_RESP_VDM_Send_Identity,
  PE_RESP_VDM_Get_Identity_NAK,
  PE_RESP_VDM_Get_SVIDs,
  PE_RESP_VDM_Send_SVIDs,
  PE_RESP_VDM_Get_SVIDs_NAK,
  PE_RESP_VDM_Get_Modes,
  PE_RESP_VDM_Send_Modes,
  PE_RESP_VDM_Get_Modes_NAK,
  PE_RCV_VDM_Attention_Request,
  // DFP Structured VDM
  PE_DFP_VDM_Mode_Entry_Request,
  PE_DFP_VDM_Mode_Entry_ACKed,
  PE_DFP_VDM_Mode_Entry_NAKed,
  PE_DFP_VDM_Mode_Exit_Request,
  PE_DFP_VDM_Mode_Exit_ACKed,
  PE_DFP_VDM_DP_Status_Request,
  PE_DFP_VDM_DP_Status_ACKed,
  PE_DFP_VDM_DP_Status_NAKed,
  PE_DFP_VDM_DP_Config_Request,
  PE_DFP_VDM_DP_Config_ACKed,
  PE_DFP_VDM_DP_Config_NAKed,
  // UFP Structure VDM
  PE_UFP_VDM_Evaluate_Mode_Entry,
  PE_UFP_VDM_Mode_Entry_ACK,
  PE_UFP_VDM_Mode_Entry_NAK,
  PE_UFP_VDM_Mode_Exit,
  PE_UFP_VDM_Mode_Exit_ACK,
  PE_UFP_VDM_Mode_Exit_NAK,
  // VDM states for cable discovery
  PE_SRC_VDM_Identity_Request,
  PE_SRC_VDM_Identity_ACKed,
  PE_SRC_VDM_Identity_NAKed,
  LAST_VDM_STATE = PE_SRC_VDM_Identity_NAKed,
  // Cable Plug Specific
  PE_CBL_Ready,
  PE_CBL_Evaluate_Mode_Entry,
  PE_CBL_Mode_Entry_ACK,
  PE_CBL_Mode_Entry_NAK,
  PE_CBL_Mode_Exit,
  PE_CBL_Mode_Exit_ACK,
  PE_CBL_Mode_Exit_NAK,
  PE_CBL_Soft_Reset,
  PE_CBL_Hard_Reset,
  PE_DFP_CBL_Send_Soft_Reset,
  PE_DFP_CBL_Send_Cable_Reset,
  PE_UFP_CBL_Send_Soft_Reset,
  // BIST Carrier Mode
  PE_BIST_Carrier_Mode,
  // USB Type-C referenced states
  PE_ErrorRecovery,
  // CUSTOM States
  PE_BIST_Test_Data,
  PE_GIVE_VDM,
  PE_Send_Generic_Cmd,
  PE_Send_Generic_Data,
  DBG_Rx_Packet,
  DBG_Tx_Packet,
  //PE_COUNT_OF_STATES,
  PE_DPM_STATES               /* ~ 25 States here temporarily for GUI display */
} PolicyState_t;

typedef enum {
  PRLDisabled = 0,            /* Protocol state machine is disabled */
  PRLIdle,                    /* Ready to receive or accept tx requests
                               * (after tx reset state, reset retry counter)
                               */
  PRLReset,                   /* Received a soft reset request message or
                               * exit from a hard reset
                               */
  PRLResetWait,               /* Waiting for the hard reset to complete */
  PRLRxWait,                  /* Actively receiving a message */
  PRLTxSendingMessage,        /* Sending message and waiting for
                               * TXSUCC, TXDISC, or TXFAIL
                               */
  /* ------- BIST Receiver Test -------- */
  PRL_BIST_Rx_Reset_Counter,  /* Reset BISTErrorCounter and preload PRBS */
  PRL_BIST_Rx_Test_Frame,     /* Wait for test Frame form PHY */
  PRL_BIST_Rx_Error_Count,    /* Construct and send BIST err count msg to PHY */
  PRL_BIST_Rx_Inform_Policy,  /* Inform PE error count has been sent */
} ProtocolState_t;

typedef enum {
  txIdle = 0,
  txReset,
  txSend,
  txBusy,
  txWait,
  txSuccess,
  txError,
  txCollision,
  txPending,
  txAbort
} PDTxStatus_t;

typedef enum {
  pdoTypeFixed = 0,
  pdoTypeBattery,
  pdoTypeVariable,
  pdoTypeAugmented
} pdoSupplyType;

typedef enum {
  apdoTypePPS = 0,
  apdoReserved0,
  apdoReserved1,
  apdoReserved2
} apdoSupplyType;

typedef enum {
  SinkTxOK,
  SinkTxNG
} SinkTxState_t;

typedef enum {
  AUTO_VDM_INIT,
  AUTO_VDM_DISCOVER_ID_PP,
  AUTO_VDM_DISCOVER_SVIDS_PP,
  AUTO_VDM_DISCOVER_MODES_PP,
  AUTO_VDM_ENTER_MODE_PP,
  AUTO_VDM_DP_GET_STATUS,
  AUTO_VDM_DP_SET_CONFIG,
  AUTO_VDM_DONE
} VdmDiscoveryState_t;

typedef enum {
  SOP_TYPE_SOP        = 0,
  SOP_TYPE_SOP1       = 1,
  SOP_TYPE_SOP2       = 2,
  SOP_TYPE_SOP1_DEBUG = 3,
  SOP_TYPE_SOP2_DEBUG = 4,
  SOP_TYPE_LAST_VALUE = SOP_TYPE_SOP2_DEBUG,
  SOP_TYPE_ERROR = 0xFF
} SopType;

/*
 * @brief State for cable reset
 */
typedef enum
{
  CBL_RST_DISABLED,
  CBL_RST_START,
  CBL_RST_VCONN_SOURCE,
  CBL_RST_DR_DFP,
  CBL_RST_SEND,
} CableResetState_t;

#endif /* FSCPM_PDTYPES_H_ */
