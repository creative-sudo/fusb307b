/*******************************************************************************
 * @file     log.h
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
 * log.h
 *
 * Defines a log implementation for use per channel.
 */
#ifndef FSCPM_LOG_H_
#define FSCPM_LOG_H_

#ifdef FSC_LOGGING

#include "platform.h"
#include "PDTypes.h"

#define FSC_LOG_SIZE    255
#define FSC_LOG_SIZE_PD 255

#define STATE_ENTRY_LENGTH 5

struct Log {
  /* Type-C State Log */
  FSC_U8 tc_data_[FSC_LOG_SIZE];
  FSC_U8 tc_writeindex_;
  FSC_U8 tc_readindex_;
  FSC_BOOL tc_overrun_;

  /* PolicyEngine State Log */
  FSC_U8 pe_data_[FSC_LOG_SIZE];
  FSC_U8 pe_writeindex_;
  FSC_U8 pe_readindex_;
  FSC_BOOL pe_overrun_;

  /* Protocol PD Message Log */
  FSC_U8 pd_data_[FSC_LOG_SIZE_PD];
  FSC_U8 pd_writeindex_;
  FSC_U8 pd_readindex_;
  FSC_BOOL pd_overrun_;
};

void LogClear(struct Log *obj);

/* Write functionality */
FSC_BOOL WriteTCState(struct Log *obj, FSC_U32 timestamp, FSC_U8 state);
FSC_BOOL WritePEState(struct Log *obj, FSC_U32 timestamp, FSC_U8 state);

FSC_BOOL WritePDToken(struct Log *obj, FSC_BOOL transmitter,
                      USBPD_BufferTokens_t token);
FSC_BOOL WritePDMsg(struct Log *obj, sopMainHeader_t header,
                    FSC_U8 *dataobject,
                    FSC_BOOL transmitter, FSC_U8 soptoken);

/* Read functionality */
FSC_U32 LogLength(void);

/* ReadLog takes a buffer and a size and returns the number of bytes written */
/* Call with a buffer bigger than LogLength() or until ReadLog returns 0. */
FSC_U32 ReadTCLog(struct Log *obj, FSC_U8 *data, FSC_U32 datalength);
FSC_U32 ReadPELog(struct Log *obj, FSC_U8 *data, FSC_U32 datalength);
FSC_U32 ReadPDLog(struct Log *obj, FSC_U8 *data, FSC_U32 datalength);

FSC_BOOL ReadTCEntry(struct Log *obj, FSC_U16 *state, FSC_U16 *timeMS10ths,
                                                      FSC_U16 *timeSeconds);
FSC_BOOL ReadPEEntry(struct Log *obj, FSC_U16 *state, FSC_U16 *timeMS10ths,
                                                      FSC_U16 *timeSeconds);

FSC_BOOL get_pd_overflow(struct Log *obj);
FSC_U32  get_pd_bytes(struct Log *obj);

#endif /* FSC_LOGGING */

#endif /* FSCPM_LOG_H_ */

