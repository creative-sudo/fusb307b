/*******************************************************************************
 * @file     observer.h
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
#ifndef MODULES_OBSERVER_H_
#define MODULES_OBSERVER_H_

#include "platform.h"

/** if MAX_OBSERVERS are not defined either at compile time or in platform.h */
#ifndef MAX_OBSERVERS
/* Only one observer by default. */
#define MAX_OBSERVERS       1
#endif /* MAX_OBSERVERS */
/**
 * Events that core uses to notify modules listening to the events.
 * The subscriber to event signal can subscribe to individual events or
 * a event in group.
 */
#define PLATFORM_EVENT_ID(id)       (1U) << (id)
#define CHECK_EVENT(event, flag)    ((event) & (flag))

#define EVENT_TYPEC_ATTACH          PLATFORM_EVENT_ID(0)
#define EVENT_TYPEC_DETACH          PLATFORM_EVENT_ID(1)
#define EVENT_CC1_ORIENT            PLATFORM_EVENT_ID(2)
#define EVENT_CC2_ORIENT            PLATFORM_EVENT_ID(3)
#define EVENT_CC_NO_ORIENT          PLATFORM_EVENT_ID(4)
#define EVENT_PD_NEW_CONTRACT       PLATFORM_EVENT_ID(5)
#define EVENT_PD_CONTRACT_FAILED    PLATFORM_EVENT_ID(6)
#define EVENT_SRC_CAPS_UPDATED      PLATFORM_EVENT_ID(7)
#define EVENT_DATA_ROLE_DFP         PLATFORM_EVENT_ID(8)
#define EVENT_DATA_ROLE_UFP         PLATFORM_EVENT_ID(9)
#define EVENT_BIST_ENABLED          PLATFORM_EVENT_ID(10)
#define EVENT_BIST_DISABLED         PLATFORM_EVENT_ID(11)
#define EVENT_ALERT_RECEIVED        PLATFORM_EVENT_ID(12)
#define EVENT_PPS_STATUS_RECIEVED   PLATFORM_EVENT_ID(13)
#define EVENT_IDENTITY_RECEIVED     PLATFORM_EVENT_ID(14)
#define EVENT_CBL_IDENTITY_RECEIVED PLATFORM_EVENT_ID(15)
#define EVENT_SVID_RECEIVED         PLATFORM_EVENT_ID(16)
#define EVENT_MODES_RECEIVED        PLATFORM_EVENT_ID(17)
#define EVENT_MODE_ENTER_SUCCESS    PLATFORM_EVENT_ID(18)
#define EVENT_MODE_EXIT_SUCCESS     PLATFORM_EVENT_ID(19)
#define EVENT_MODE_VDM_ATTENTION    PLATFORM_EVENT_ID(20)
#define EVENT_HARD_RESET            PLATFORM_EVENT_ID(21)
#define EVENT_UNSUPPORTED_ACCESSORY PLATFORM_EVENT_ID(22)
#define EVENT_DEBUG_ACCESSORY       PLATFORM_EVENT_ID(23)
#define EVENT_AUDIO_ACCESSORY       PLATFORM_EVENT_ID(24)
#define EVENT_ILLEGAL_CBL           PLATFORM_EVENT_ID(25)
#define EVENT_ALL                   (~(0U))

#define USER_EVENT_ID               PLATFORM_EVENT_ID(26)

/* Event is declared as 32-bit. It supports 32 events.
 * If more than 32 events are required change the
 * Event_t to the appropriate type and CHECK_EVENT() macro
 * to test the bits in the Event_t */
typedef FSC_U32 Event_t;
typedef void (*EventHandler)(Event_t event, FSC_U16 portId,
                             void *usr_ctx, void *app_ctx);


/**
 * @brief register an observer.
 * @param[in] event to subscribe
 * @param[in] handler to be called
 * @param[in] context data sent to the handler
 */
FSC_BOOL register_observer(Event_t event, EventHandler handler, void *context);

/**
 * @brief removes the observer. Observer stops getting notified
 * @param[in] handler handler to remove
 */
void remove_observer(EventHandler handler);

/**
 * @brief notifies all observer that are listening to the event.
 * @param[in] event that occured
 */
void notify_observers(Event_t event, FSC_U16 portId, void *app_ctx);


#endif /* MODULES_OBSERVER_H_ */
