/*******************************************************************************
 * @file     TypeCTypes.h
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
#ifndef FSCPM_TYPECTYPES_H_
#define FSCPM_TYPECTYPES_H_

typedef enum {
  NONE,
  CC1, /* CC1 == 1 */
  CC2  /* CC2 == 2 */
} CCOrientation;

typedef enum {
  Source = 0,
  Sink
} SourceOrSink;

typedef enum {
  USBTypeC_Sink = 0,
  USBTypeC_Source,
  USBTypeC_DRP,
  USBTypeC_UNDEFINED
} USBTypeCPort;

typedef enum {
  Disabled = 0,
  ErrorRecovery,
  Unattached,
  AttachWaitSink,
  AttachedSink,
  AttachWaitSource,
  AttachedSource,
  TrySource,
  TryWaitSink,
  TrySink,
  TryWaitSource,
  AudioAccessory,
  AttachWaitAccessory,
  UnorientedDebugAccessorySource,
  OrientedDebugAccessorySource,
  DebugAccessorySink,
  PoweredAccessory,
  UnsupportedAccessory,
  DelayUnattached,
  UnattachedWaitSource,
  IllegalCable
} TypeCState;

typedef enum {
  CCTypeOpen = 0,
  CCTypeRa,
  CCTypeRdUSB,
  CCTypeRd1p5,
  CCTypeRd3p0,
  CCTypeUndefined
} CCTermType;

typedef enum {
  CCTermSrcOpen = 0,
  CCTermSrcRa = 0b01,
  CCTermSrcRd = 0b10,
  CCTermSrcUndefined = 0b11
} CCSourceTermType;

typedef enum {
  CCTermSnkOpen = 0,
  CCTermSnkDefault = 0b01,
  CCTermSnkRp1p5 = 0b10,
  CCTermSnkRp3p0 = 0b11
} CCSinkTermType;

typedef enum {
  CCRoleRa = 0,
  CCRoleRp = 0b01,
  CCRoleRd = 0b10,
  CCRoleOpen = 0b11
} CCRoleTermType;

typedef enum {
  RpValDefault = 0,
  RpVal1p5 = 0b01,
  RpVal3p0 = 0b10,
  RpValUndefined = 0b11
} RoleRpVal;

typedef enum {
  TypeCPin_None = 0,
  TypeCPin_GND1,
  TypeCPin_TXp1,
  TypeCPin_TXn1,
  TypeCPin_VBUS1,
  TypeCPin_CC1,
  TypeCPin_Dp1,
  TypeCPin_Dn1,
  TypeCPin_SBU1,
  TypeCPin_VBUS2,
  TypeCPin_RXn2,
  TypeCPin_RXp2,
  TypeCPin_GND2,
  TypeCPin_GND3,
  TypeCPin_TXp2,
  TypeCPin_TXn2,
  TypeCPin_VBUS3,
  TypeCPin_CC2,
  TypeCPin_Dp2,
  TypeCPin_Dn2,
  TypeCPin_SBU2,
  TypeCPin_VBUS4,
  TypeCPin_RXn1,
  TypeCPin_RXp1,
  TypeCPin_GND4
} TypeCPins_t;

typedef enum {
  utccOpen = 0,
  utccDefault = 1,
  utcc1p5A = 2,
  utcc3p0A = 3,
  utccInvalid,
} USBTypeCCurrent;

typedef enum {
  FRS_None = 0,
  FRS_Hub_Sink = 1,
  FRS_Hub_Source = 2,
  FRS_Host_Sink = 3,
} FRSMode;

#endif /* FSCPM_TYPECTYPES_H_ */
