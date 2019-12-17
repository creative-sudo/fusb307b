/*******************************************************************************
 * @file     hostcomm.h
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
 * hostcomm.h
 *
 * Defines the HostComm functionality for USB HID access.
 */
#ifndef FSCPM_HOSTCOMM_H_
#define FSCPM_HOSTCOMM_H_

#ifdef FSC_HAVE_USBHID

#include "FSCTypes.h"
#include "port.h"
#include "hcmd.h"

/* These are board/hw defs that we need to handle or tell the GUI to ignore */
#define MY_MCU                      0x01
#define MY_DEV_TYPE                 0x00
#define MY_BC                       0x0000

/* Version of hostcom implementation. Current spec revision is 1.2 */
#define HOSTCOM_REV_LOW             0x2
#define HOSTCOM_REV_HIGH            0x1

/* The ProcessMessage function starts the decoding of messages from the GUI */
void ProcessMsg(FSC_U8 *inMsgBuffer, FSC_U8 *outMsgBuffer, struct Port *port);

#endif /* FSC_HAVE_USBHID */
#endif /* FSCPM_HOSTCOMM_H_ */

