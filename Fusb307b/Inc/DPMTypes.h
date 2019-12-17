/*******************************************************************************
 * @file     DPMTypes.h
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
#ifndef FSCPM_DPMTYPES_H_
#define FSCPM_DPMTYPES_H_

typedef enum {
  dpmPowerOnReset = 0,
  dpmCCErrorRecovery,
  dpmDischargeVIN,
  dpmCCDetached,
  dpmPriorAttachDischVbus,
  dpmCCAttached,
  dpmPDPrepareCapabilities,
  dpmPDReceiveRequest,
  dpmPowerTransition,
  dpmPDConnected,
  dpmSendReset,
  dpmPDExecuteReset,
  dpmIdentityRequest,
  dpmIdentityFound,
  dpmIdentityAbandoned,
  dpmIdentityStillTrying,
  dpmVCONNSwapRequired,
  dpmVCONNSwapReceived,
  dpmStatus,
  dpmPPSStatus,
  dpmAlertStatus,
  dpmExtendedCaps,
  dpmDiscoverVersion,
  dpmBISTTestData,
  dpmKamakaze
} DPMState_t;

#endif /* FSCPM_DPMTYPES_H_ */
