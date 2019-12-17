/*******************************************************************************
 * @file     core.h
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
#ifndef _FSC_CORE_H_
#define _FSC_CORE_H_

#include "port.h"
#include "typec.h"
#include "log.h"
#include "version.h"

void core_initialize(struct Port *port);
void core_state_machine(struct Port *port);

FSC_U32 core_get_next_timeout(struct Port *port);

void core_enable_typec(struct Port *port, FSC_BOOL enable);
void core_set_advertised_current(struct Port *port, USBTypeCCurrent src_cur);

FSC_U8 core_get_rev_lower(void);
FSC_U8 core_get_rev_middle(void);
FSC_U8 core_get_rev_upper(void);

#ifdef FSC_DEBUG
void core_set_state_unattached(struct Port *port);
#endif // FSC_DEBUG

#endif /* _FSC_CORE_H_ */

