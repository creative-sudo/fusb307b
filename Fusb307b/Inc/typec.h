/*******************************************************************************
 * @file     typec.h
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
 * Defines the Type-C state machine functions
 */
#ifndef FSCPM_TYPEC_H_
#define FSCPM_TYPEC_H_

#include "platform.h"
#include "port.h"

#define SLEEP_DELAY_US    80    /* 0.08ms */

void StateMachineTypeC(struct Port *port);
void StateMachineDisabled(struct Port *port);
void StateMachineErrorRecovery(struct Port *port);
void StateMachineUnattached(struct Port *port);

#ifdef FSC_HAVE_SNK
void StateMachineAttachWaitSink(struct Port *port);
void StateMachineAttachedSink(struct Port *port);
void StateMachineDebugAccessorySink(struct Port *port);
void StateMachineTrySink(struct Port *port);
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void StateMachineAttachWaitSource(struct Port *port);
void StateMachineAttachedSource(struct Port *port);
void StateMachineUnattachedWaitSource(struct Port *port);
void StateMachineUnorientedDebugAccessorySource(struct Port *port);
void StateMachineOrientedDebugAccessorySource(struct Port *port);
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_DRP
void StateMachineTrySource(struct Port *port);
void StateMachineTryWaitSink(struct Port *port);
void StateMachineTryWaitSource(struct Port *port);
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_ACC
void StateMachineAttachWaitAccessory(struct Port *port);
void StateMachineAudioAccessory(struct Port *port);
void StateMachinePoweredAccessory(struct Port *port);
void StateMachineUnsupportedAccessory(struct Port *port);
#endif /* FSC_HAVE_ACC */

void StateMachineIllegalCable(struct Port *port);

void SetStateDisabled(struct Port *port);
void SetStateErrorRecovery(struct Port *port);
void SetStateUnattached(struct Port *port);

#ifdef FSC_HAVE_SNK
void SetStateAttachWaitSink(struct Port *port);
void SetStateAttachedSink(struct Port *port);
void SetStateDebugAccessorySink(struct Port *port);
void SetStateTrySink(struct Port *port);
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void SetStateAttachWaitSource(struct Port *port);
void SetStateAttachedSource(struct Port *port);
void SetStateUnattachedWaitSource(struct Port *port);
void SetStateUnorientedDebugAccessorySource(struct Port *port);
void SetStateOrientedDebugAccessorySource(struct Port *port);
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_DRP
void RoleSwapToAttachedSink(struct Port *port);
void RoleSwapToAttachedSource(struct Port *port);
void SetStateTryWaitSource(struct Port *port);
void SetStateTrySource(struct Port *port);
void SetStateTryWaitSink(struct Port *port);
#endif /* FSC_HAVE_DRP */

#if defined(FSC_HAVE_ACC) && (defined(FSC_HAVE_SNK) || defined(FSC_HAVE_SRC))
void SetStateAttachWaitAccessory(struct Port *port);
void SetStateAudioAccessory(struct Port *port);
void SetStatePoweredAccessory(struct Port *port);
void SetStateUnsupportedAccessory(struct Port *port);
#endif /* FSC_HAVE_ACC || (FSC_HAVE_SNK && FSC_HAVE_SRC) */

void SetStateIllegalCable(struct Port *port);

#endif /* FSCPM_TYPEC_H_ */

