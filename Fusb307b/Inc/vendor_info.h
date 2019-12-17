/*******************************************************************************
 * @file     vendor_info.h
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
#ifndef VENDOR_INFO_H
#define VENDOR_INFO_H

#include "vif_macros.h"
#include "display_port_types.h"

/* Leave any fields that do not apply as their defaults */
/*
 * All fields marked as unimplemented are supported by device, but are not
 * part of the example code. Features can be implemented on request.
 */

/*
 * Change the number of PDOs enabled to the correct value. If you enable PPS
 * PDOs make sure that firmware is built with PPS enabled flag.
 */

#define $VIF_Specification "Revision 1.32, Version 1.0"
#define $VIF_Producer "USB-IF Vendor Info File Generator, Version 1.2.0.2"
#define $Vendor_Name "Samsung"
#define $Model_Part_Number "1"
#define $Product_Revision "1"
#define $TID "0"
#define $Port_Label "J1"

/* VIF Product tab */
#define Connector_Type 2                    /* Type-C */
#define USB_PD_Support YES
#define PD_Port_Type 0                      /* 0:Consumer, 3:Provider, 4:DRP */
#define Type_C_State_Machine 1              /* 0:Src, 1:Snk, 2:DRP */
#define Port_Battery_Powered NO
#define BC_1_2_Support NO

/* General PD Settings Tab */
#define PD_Specification_Revision 2        /* Revision 3.0 */
#define SOP_Capable YES
#define SOP_P_Capable NO
#define SOP_PP_Capable NO
#define SOP_P_Debug_Capable NO
#define SOP_PP_Debug_Capable NO
#define Manufacturer_Info_Supported_Port YES
#define Manufacturer_Info_VID_Port 0x04E8
#define Manufacturer_Info_PID_Port 0x0000
#define Num_Fixed_Batteries 0
#define Num_Swappable_Battery_Slots 0
#define USB_Comms_Capable YES
#define DR_Swap_To_DFP_Supported NO
#define DR_Swap_To_UFP_Supported NO
#define Unconstrained_Power NO
#define VCONN_Swap_To_On_Supported NO
#define VCONN_Swap_To_Off_Supported NO
#define Responds_To_Discov_SOP_UFP YES
#define Responds_To_Discov_SOP_DFP NO
#define Attempts_Discov_SOP NO
#define Chunking_Implemented_SOP YES
#define Unchunked_Extended_Messages_Supported NO
#define Security_Msgs_Supported_SOP NO

/* USB Type-C Tab */
#define Type_C_Implements_Try_SRC NO
#define Type_C_Implements_Try_SNK NO
#define Rp_Value 2                          /* 0:Def, 1:1.5A, 2:3.0A */
#define Type_C_Supports_VCONN_Powered_Accessory NO
#define Type_C_Is_VCONN_Powered_Accessory NO
#define Type_C_Is_Debug_Target_SRC NO
#define Type_C_Is_Debug_Target_SNK NO
#define Type_C_Can_Act_As_Host NO
#define Type_C_Can_Act_As_Device YES
#define Type_C_Is_Alt_Mode_Controller NO
#define Type_C_Is_Alt_Mode_Adapter YES
#define Type_C_Power_Source 1               /* 0: Self-Powered, 1 : UFP-Powered, 2 : Both */
#define Type_C_Port_On_Hub NO
#define Type_C_Supports_Audio_Accessory NO
#define Captive_Cable NO
#define Type_C_Sources_VCONN NO

/* Source tab */
#define PD_Power_as_Source 15000
#define USB_Suspend_May_Be_Cleared YES
#define Sends_Pings NO
#define FR_Swap_Supported_As_Source NO
#define Num_Src_PDOs 1

#define Src_PDO_Supply_Type1 0              //; 0: Fixed
#define Src_PDO_Peak_Current1 0             //; 0: 100% IOC
#define Src_PDO_Voltage1 100                //; 5V
#define Src_PDO_Max_Current1 300            //; 3A
#define Src_PDO_Min_Voltage1 0              //; 0 V
#define Src_PDO_Max_Voltage1 0              //; 0 V
#define Src_PDO_Max_Power1 0                //; 0 W

#define Src_PDO_Supply_Type2 0              //; 0: Fixed
#define Src_PDO_Peak_Current2 0             //; 0: 100% IOC
#define Src_PDO_Voltage2 180                //; 9V
#define Src_PDO_Max_Current2 300            //; 3A
#define Src_PDO_Min_Voltage2 0              //; 0 V
#define Src_PDO_Max_Voltage2 0              //; 0 V
#define Src_PDO_Max_Power2 0                //; 0 W

#define Src_PDO_Supply_Type3 3              //; 3: Augmented
#define Src_PDO_Peak_Current3 0             //; 0: 100% IOC
#define Src_PDO_Voltage3 0
#define Src_PDO_Max_Current3 60             //; 3A
#define Src_PDO_Min_Voltage3 30             //; 3 V
#define Src_PDO_Max_Voltage3 110            //; 11 V
#define Src_PDO_Max_Power3 0                //; 0 W

#define Src_PDO_Supply_Type4 0              //; 0: Fixed
#define Src_PDO_Peak_Current4 0             //; 0: 100% IOC
#define Src_PDO_Voltage4 0
#define Src_PDO_Max_Current4 0
#define Src_PDO_Min_Voltage4 0              //; 0 V
#define Src_PDO_Max_Voltage4 0              //; 0 V
#define Src_PDO_Max_Power4 0                //; 0 W

#define Src_PDO_Supply_Type5 0              //; 0: Fixed
#define Src_PDO_Peak_Current5 0             //; 0: 100% IOC
#define Src_PDO_Voltage5 0
#define Src_PDO_Max_Current5 0
#define Src_PDO_Min_Voltage5 0              //; 0 V
#define Src_PDO_Max_Voltage5 0              //; 0 V
#define Src_PDO_Max_Power5 0                //; 0 W

#define Src_PDO_Supply_Type6 0              //; 0: Fixed
#define Src_PDO_Peak_Current6 0             //; 0: 100% IOC
#define Src_PDO_Voltage6 0
#define Src_PDO_Max_Current6 0
#define Src_PDO_Min_Voltage6 0              //; 0 V
#define Src_PDO_Max_Voltage6 0              //; 0 V
#define Src_PDO_Max_Power6 0                //; 0 W

#define Src_PDO_Supply_Type7 0              //; 0: Fixed
#define Src_PDO_Peak_Current7 0             //; 0: 100% IOC
#define Src_PDO_Voltage7 0
#define Src_PDO_Max_Current7 0
#define Src_PDO_Min_Voltage7 0              //; 0 V
#define Src_PDO_Max_Voltage7 0              //; 0 V
#define Src_PDO_Max_Power7 0                //; 0 W

/* Sink Tab */
#define PD_Power_as_Sink 5000
#define No_USB_Suspend_May_Be_Set NO
#define GiveBack_May_Be_Set NO
#define Higher_Capability_Set NO
#define FR_Swap_Reqd_Type_C_Current 0
#define Num_Snk_PDOs 1

#define Snk_PDO_Supply_Type1 0              //; 0: Fixed
#define Snk_PDO_Voltage1 100                // *50mV
#define Snk_PDO_Op_Current1 100             // *10mA
#define Snk_PDO_Min_Voltage1 0              //; 0 V
#define Snk_PDO_Max_Voltage1 0              //; 0 V
#define Snk_PDO_Op_Power1 0                 //; 0 W

#define Snk_PDO_Supply_Type2 0              //; 0: Fixed
#define Snk_PDO_Voltage2 180
#define Snk_PDO_Op_Current2 10
#define Snk_PDO_Min_Voltage2 0              //; 0 V
#define Snk_PDO_Max_Voltage2 0              //; 0 V
#define Snk_PDO_Op_Power2 0                 //; 0 W

#define Snk_PDO_Supply_Type3 0              //; 0: Fixed
#define Snk_PDO_Voltage3 0
#define Snk_PDO_Op_Current3 0
#define Snk_PDO_Min_Voltage3 0              //; 0 V
#define Snk_PDO_Max_Voltage3 0              //; 0 V
#define Snk_PDO_Op_Power3 0                 //; 0 W

#define Snk_PDO_Supply_Type4 0              //; 0: Fixed
#define Snk_PDO_Voltage4 0
#define Snk_PDO_Op_Current4 0
#define Snk_PDO_Min_Voltage4 0              //; 0 V
#define Snk_PDO_Max_Voltage4 0              //; 0 V
#define Snk_PDO_Op_Power4 0                 //; 0 W

#define Snk_PDO_Supply_Type5 0              //; 0: Fixed
#define Snk_PDO_Voltage5 0
#define Snk_PDO_Op_Current5 0
#define Snk_PDO_Min_Voltage5 0              //; 0 V
#define Snk_PDO_Max_Voltage5 0              //; 0 V
#define Snk_PDO_Op_Power5 0                 //; 0 W

#define Snk_PDO_Supply_Type6 0              //; 0: Fixed
#define Snk_PDO_Voltage6 0
#define Snk_PDO_Op_Current6 0
#define Snk_PDO_Min_Voltage6 0              //; 0 V
#define Snk_PDO_Max_Voltage6 0              //; 0 V
#define Snk_PDO_Op_Power6 0                 //; 0 W

#define Snk_PDO_Supply_Type7 0              //; 0: Fixed
#define Snk_PDO_Voltage7 0
#define Snk_PDO_Op_Current7 0
#define Snk_PDO_Min_Voltage7 0              //; 0 V
#define Snk_PDO_Max_Voltage7 0              //; 0 V
#define Snk_PDO_Op_Power7 0                 //; 0 W

/* Dual Role Tab */
#define Accepts_PR_Swap_As_Src YES
#define Accepts_PR_Swap_As_Snk YES
#define Requests_PR_Swap_As_Src NO
#define Requests_PR_Swap_As_Snk NO

/* SOP Discovery - Part One Tab */
#define Structured_VDM_Version_SOP 1        /* V2.0 (w/ fallback to V1.0) */
#define XID_SOP 0
#define Data_Capable_as_USB_Host_SOP NO
#define Data_Capable_as_USB_Device_SOP YES
#define Product_Type_UFP_SOP 5              /* 0: Undefined, 5 : AMA */
#define Product_Type_DFP_SOP 0              /* Undefined*/
#define Modal_Operation_Supported_SOP YES
#define USB_VID_SOP 0x04E8                  /* 0x04E8 : == Samsung VID */
#define PID_SOP 0x0000
#define bcdDevice_SOP 0x0000

/* SOP Discovery - Part Two Tab */
#define SVID_fixed_SOP YES
#define Num_SVIDs_min_SOP 1
#define Num_SVIDs_max_SOP 1                 /* Currently Implements Up To 1 */

#define SVID1_SOP 0xFF01                    /* Display Port VID */
#define SVID1_modes_fixed_SOP YES
#define SVID1_num_modes_min_SOP 1
#define SVID1_num_modes_max_SOP 1           /* Currently Implements Up To 1 */

#define SVID1_mode1_enter_SOP YES
#define SVID1_mode1_recog_mask_SOP 0xFFFFFFFF
#define SVID1_mode1_recog_value_SOP 0xFFFFFFFF

/* AMA Tab */
#define AMA_HW_Vers 0x1
#define AMA_FW_Vers 0x1
#define AMA_VCONN_reqd NO
#define AMA_VCONN_power 0                   //; 0: 1W
#define AMA_VBUS_reqd YES
#define AMA_Superspeed_Support 0            //; 0: USB 2.0 only

/*******************************************************************************
 * Custom Configuration
 ******************************************************************************/
#define Attempt_DR_Swap_to_Ufp_As_Src        NO
#define Attempt_DR_Swap_to_Dfp_As_Sink       NO
#define Attempt_Vconn_Swap_to_Off_As_Src     NO
#define Attempt_Vconn_Swap_to_On_As_Sink     NO
#define Attempts_DiscvId_SOP_P_First         NO
#define Sources_VBus_For_Powered_Accessory   NO  /* No for compliance, Yes for most devices */
/* Display port related configuration */
/* Display port sink configuration supported */
#define DisplayPort_Auto_Mode_Entry      YES                                                     /* Automatically enter display port mode if found, Auto VDM required */
#define DisplayPort_Enabled              YES
#define DisplayPort_Preferred_Snk        NO                                                      /* Prefer to be sink when DFP_D & UFP_D capable device attached */
#define DisplayPort_UFP_Capable          YES                                                     /* DP Sink */
#define DisplayPort_DFP_Capable          NO                                                     /* DP Source */
#define DisplayPort_Signaling            YES                                                     /* YES == 0x1, DP Standard signaling */
#define DisplayPort_Receptacle           1                                                       /* DP is presented in Type-C 0 plug, 1 - Receptacle */
#define DisplayPort_USBr2p0Signal_Req    NO                                                      /* USB r2.0 signaling required */
#define DisplayPort_DFP_Pin_Mask         DP_PIN_ASSIGN_C | DP_PIN_ASSIGN_D | DP_PIN_ASSIGN_E     /* Only Pin Assignment C,D,E supported */
#define DisplayPort_UFP_Pin_Mask         DP_PIN_ASSIGN_C                                         /* Only Pin Assignment C,D,E supported */
#define DisplayPort_UFP_PinAssign_Start  'C'                                                     /* Only Pin assignment and higher are allowed. Setting it C allows C, D, E*/

#endif /* VENDOR_INFO_H */
