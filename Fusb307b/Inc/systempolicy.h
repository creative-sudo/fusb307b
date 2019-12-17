/*******************************************************************************
 * @file     systempolicy.h
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
 * Defines a class that contains the SystemPolicy interface
 */
#ifndef FSCPM_SYSTEMPOLICY_H_
#define FSCPM_SYSTEMPOLICY_H_

#include "platform.h"
#include "port.h"

/* Called from the while(1) loop, process events as needed */
void SystemPolicyProcess(struct Port *ports);

#endif /* FSCPM_SYSTEMPOLICY_H_ */

