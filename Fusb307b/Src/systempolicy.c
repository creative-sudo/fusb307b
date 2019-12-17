/*******************************************************************************
 * @file     systempolicy.c
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
 * systempolicy.c
 *
 * Implements communication with system level operators/code and
 * provides access to the Type-C and PD world.
 */

#include "systempolicy.h"

#include "port.h"
#include "platform.h"

#ifdef FSC_HAVE_USBHID
#include "usbd_hid.h"
#include "hostcomm.h"
#endif /* FSC_HAVE_USBHID */

/* Character buffer for UART receive messages.  Used for debugging, etc. */
#ifdef FSC_HAVE_UART
FSC_U8 UARTRecBuffer[64];
FSC_U8 UARTRecIndex = 0;
FSC_BOOL UARTRecHaveMsg = FALSE;
#endif /* FSC_HAVE_UART */

void SystemPolicyProcess(struct Port *ports) {
#ifdef FSC_HAVE_UART
  FSC_U8 i;
#endif /* FSC_HAVE_UART */

  /* Process any current HostComm interactions */
#ifdef FSC_HAVE_USBHID
  if (haveUSBInMsg) {
        /* HostComm */
        ProcessMsg(USBInputMsg, USBOutputMsg, &ports[0]);
        haveUSBInMsg = FALSE;
        USBD_HID_SendReport(&USBD_Device, USBOutputMsg, USB_MSG_LENGTH);
    }
#endif /* FSC_HAVE_USBHID */

  /* Look for incoming UART commands */
#ifdef FSC_HAVE_UART
  if (UARTRecHaveMsg) {
/* For Example
 *    if (UARTRecBuffer[0] == 'r') {
 *      DoSomethingAboutR();
 *    }
 */
    /* Buffer[0] should be a channel number */
    UARTRecBuffer[0] -= 0x31; /* Hex offset for digit '1' */
    if (UARTRecBuffer[0] < 1 || UARTRecBuffer[0] > FSC_NUMBER_OF_PORTS)
      UARTRecBuffer[0] = 0;

    if (UARTRecBuffer[2] == 's' &&
        UARTRecBuffer[3] == 'e' &&
        UARTRecBuffer[4] == 'c') {
      ports[0].idle_ = FALSE;
      set_policy_state(&ports[UARTRecBuffer[0]], PE_Send_Security_Request);
    }

    for (i = 0; i < 64; ++i) UARTRecBuffer[i] = 0;
    UARTRecHaveMsg = FALSE;
    UARTRecIndex = 0;
  }
#endif /* FSC_HAVE_UART */
}
