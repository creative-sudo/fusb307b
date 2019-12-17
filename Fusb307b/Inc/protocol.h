/*******************************************************************************
 * @file     protocol.h
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
 * Defines the PD Protocol state machine functions
 */
#ifndef FSCPM_PROTOCOL_H_
#define FSCPM_PROTOCOL_H_

#include "platform.h"
#include "port.h"

#define tProtocolTxTimeout      10  * kMSTimeFactor

void USBPDProtocol(struct Port *port);
void ProtocolIdle(struct Port *port);
void ProtocolResetWait(struct Port *port);
void ProtocolRxWait(struct Port *port);
void ProtocolGetRxPacket(struct Port *port);
void ProtocolTransmitMessage(struct Port *port);
void ProtocolSendingMessage(struct Port *port);
void ProtocolSendHardReset(struct Port *port, FSC_BOOL cable);
SopType TokenToSopType(FSC_U8 data);

#endif /* FSCPM_PROTOCOL_H_ */

