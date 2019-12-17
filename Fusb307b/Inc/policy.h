/*******************************************************************************
 * @file     policy.h
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
 * policy.h
 *
 * Defines functionality for the Policy Engine state machine.
 */
#ifndef FSCPM_POLICY_H_
#define FSCPM_POLICY_H_

#include "platform.h"
#include "port.h"

void USBPDPolicyEngine(struct Port *port);
void PolicyErrorRecovery(struct Port *port);

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACC))
void PolicySourceSendHardReset(struct Port *port);
void PolicySourceSendSoftReset(struct Port *port);
void PolicySourceSoftReset(struct Port *port);
void PolicySourceStartup(struct Port *port);
void PolicySourceDiscovery(struct Port *port);
void PolicySourceSendCaps(struct Port *port);
void PolicySourceDisabled(struct Port *port);
void PolicySourceTransitionDefault(struct Port *port);
void PolicySourceNegotiateCap(struct Port *port);
void PolicySourceTransitionSupply(struct Port *port);
void PolicySourceCapabilityResponse(struct Port *port);
void PolicySourceReady(struct Port *port);
void PolicySourceGetSourceCap(struct Port *port);
void PolicySourceGetSinkCap(struct Port *port);
void PolicySourceGiveSinkCap(struct Port *port);
#ifdef FSC_HAVE_EXTENDED
void PolicySourceGiveSourceCapExtended(struct Port *port);
#endif /* FSC_HAVE_EXTENDED */
void PolicySourceSendPing(struct Port *port);
void PolicySourceSendDRSwap(struct Port *port);
void PolicySourceEvaluateDRSwap(struct Port *port);
void PolicySourceSendVCONNSwap(struct Port *port);
void PolicySourceEvaluateVCONNSwap(struct Port *port);
void PolicySourceSendPRSwap(struct Port *port);
void PolicySourceEvaluatePRSwap(struct Port *port);
#ifdef FSC_HAVE_SRC
void PolicySourceEvaluateFRSwap(struct Port *port);
#endif /* FSC_HAVE_SRC */
void PolicySourceWaitNewCapabilities(struct Port *port);
void PolicySourceChunkReceived(struct Port *port);
void PolicySourceSendNotSupported(struct Port *port);
void PolicySourceNotSupported(struct Port *port);
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACC) */
#ifdef FSC_HAVE_SNK
void PolicySinkStartup(struct Port *port);
void PolicySinkSendHardReset(struct Port *port);
void PolicySinkSoftReset(struct Port *port);
void PolicySinkSendSoftReset(struct Port *port);
void PolicySinkTransitionDefault(struct Port *port);
void PolicySinkDiscovery(struct Port *port);
void PolicySinkWaitCaps(struct Port *port);
void PolicySinkEvaluateCaps(struct Port *port);
void PolicySinkSelectCapability(struct Port *port);
void PolicySinkTransitionSink(struct Port *port);
void PolicySinkReady(struct Port *port);
void PolicySinkGiveSinkCap(struct Port *port);
void PolicySinkGetSourceCap(struct Port *port);
void PolicySinkGetSinkCap(struct Port *port);
void PolicySinkGiveSourceCap(struct Port *port);
void PolicySinkSendDRSwap(struct Port *port);
void PolicySinkEvaluateDRSwap(struct Port *port);
void PolicySinkSendVCONNSwap(struct Port *port);
void PolicySinkEvaluateVCONNSwap(struct Port *port);
void PolicySinkSendPRSwap(struct Port *port);
void PolicySinkEvaluatePRSwap(struct Port *port);
#if defined(FSC_HAVE_EXTENDED) && defined(FSC_HAVE_SRC)
void PolicySinkGiveSourceCapExtended(struct Port *port);
#endif /* FSC_HAVE_EXTENDED && FSC_HAVE_DRP */
#ifdef FSC_HAVE_FRSWAP
void PolicySinkSendFRSwap(struct Port *port);
void PolicySinkGetSourceCapExt(struct Port *port);
#endif /* FSC_HAVE_FRSWAP */
#endif /* FSC_HAVE_SNK */

void PolicySendNotSupported(struct Port *port);
void PolicyNotSupported(struct Port *port);

void PolicyDFPCBLSendSoftReset(struct Port *port);
void PolicyDFPCBLSendReset(struct Port *port);

#ifdef FSC_HAVE_EXTENDED
void PolicyGetSecurityMsg(struct Port *port);
void PolicySendSecurityMsg(struct Port *port);
void PolicySecurityMsgReceived(struct Port *port);
void PolicyGivePPSStatus(struct Port *port);
void PolicyGetPPSStatus(struct Port *port);
void PolicySinkGetSourceCapExt(struct Port *port);
void PolicyGiveManufacturerInfo(struct Port *port);
#endif /* FSC_HAVE_EXTENDED */

void UpdateCapabilitiesRx(struct Port *port, FSC_BOOL is_source_cap_update);

FSC_BOOL PolicySendHardReset(struct Port *port, PolicyState_t next_state,
                             FSC_BOOL cable);

void PolicySendGenericCommand(struct Port *port);
void PolicySendGenericData(struct Port *port);

FSC_U8 PolicySend(struct Port *port, FSC_U8 message_type,
                  FSC_U16 num_bytes, FSC_U8 *data,
                  PolicyState_t next_state,
                  FSC_U8 subindex, SopType sop, FSC_BOOL extended);

/* BIST functionality */
void ProcessDmtBist(struct Port *port);
void PolicyBISTCarrierMode2(struct Port *port);
void PolicyBISTTestData(struct Port *port);
void PolicyInvalidState(struct Port *port);
void ProcessCableResetState(struct Port *port);

#endif /* FSCPM_POLICY_H_ */

