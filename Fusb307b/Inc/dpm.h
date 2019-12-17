/*******************************************************************************
 * @file     dpm.h
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
 * dpm.h
 *
 * Defines functionality for the Device Policy Manager state machine.
 */
#ifndef FSCPM_DPM_H_
#define FSCPM_DPM_H_

#include "platform.h"
#include "port.h"

#define DPM_REJECT_MAX 2
#define DPM_DISCID_MAX 4 // TODO Not Defined???

void DPM_Initialize(struct Port *port);
void DPM_Reset(struct Port *port);
void DPM_PrepareSrcCaps(struct Port *port);
FSC_BOOL DPM_EvaluateRequest(struct Port *port);

void DPM_Status(struct Port *port);
void DPM_PPSStatus(struct Port *port);
void DPM_AlertStatus(struct Port *port);
void DPM_ExtendedCaps(struct Port *port);
void DPM_DiscoverVersion(struct Port *port);

void DPM_PrepareStatus(struct Port *port);
void DPM_PreparePPSStatus(struct Port *port);
void DPM_PrepareExtendedCaps(struct Port *port);

FSC_BOOL DPM_VCS_Allowed(struct Port *port);

void DPM_SetSOPVersion(struct Port *port, FSC_U8 ver);
void DPM_SetSOP1Details(struct Port *port,
                        FSC_BOOL ack, FSC_U8 ver, FSC_BOOL is_5a_capable);

FSC_U8 DPM_CurrentSpecRev(struct Port *port, SopType sop);
FSC_BOOL DPM_IsSOPPAllowed(struct Port *port);
void DPM_ReConfigureRxDetect(struct Port *port);
FSC_U8 DPM_Retries(struct Port *port, SopType sop);

#endif /* FSCPM_DPM_H_ */
