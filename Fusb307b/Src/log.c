/*******************************************************************************
 * @file     log.c
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
 * log.c
 *
 * Implements a log for use per channel
 */

#ifdef FSC_LOGGING

#include "log.h"

FSC_BOOL RequestSpace(FSC_U32 length, FSC_U8 writeidx, FSC_U8 readidx,
                      FSC_U32 logsize);

FSC_U32 LogLength(void) { return FSC_LOG_SIZE; };
FSC_BOOL get_pd_overflow(struct Log *obj) { return obj->tc_overrun_; };

void LogClear(struct Log *obj)
{
  FSC_U16 i = 0;

  for (i = 0; i < FSC_LOG_SIZE; ++i) {
    obj->tc_data_[i] = 0;
    obj->pe_data_[i] = 0;
  }

  for (i = 0; i < FSC_LOG_SIZE_PD; ++i) {
    obj->pd_data_[i] = 0;
  }

  obj->tc_writeindex_ = obj->tc_readindex_ = 0;
  obj->pe_writeindex_ = obj->pe_readindex_ = 0;
  obj->pd_writeindex_ = obj->pd_readindex_ = 0;

  obj->tc_overrun_ = obj->pe_overrun_ = obj->pd_overrun_ = FALSE;
}

FSC_BOOL WriteTCState(struct Log *obj, FSC_U32 timestamp, FSC_U8 state)
{
  /* This entry requires 5 bytes. */
  /* [State][TimeMSB][Time][Time][TimeLSB][State] */
  if (!RequestSpace(STATE_ENTRY_LENGTH,
                    obj->tc_writeindex_, obj->tc_readindex_, FSC_LOG_SIZE)) {
    /* If we are out of room, bump up the read index to overwrite old entries */
    obj->tc_readindex_ =
      (obj->tc_readindex_ + STATE_ENTRY_LENGTH) % FSC_LOG_SIZE;
    obj->tc_overrun_ = TRUE;
  }

  /* This order of entries is to support the current hostcomm interface. */
  obj->tc_data_[obj->tc_writeindex_] = state;
  obj->tc_writeindex_ = (obj->tc_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->tc_data_[obj->tc_writeindex_] = (timestamp & 0x000000FF);
  obj->tc_writeindex_ = (obj->tc_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->tc_data_[obj->tc_writeindex_] = (timestamp & 0x0000FF00) >>  8;
  obj->tc_writeindex_ = (obj->tc_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->tc_data_[obj->tc_writeindex_] = (timestamp & 0x00FF0000) >> 16;
  obj->tc_writeindex_ = (obj->tc_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->tc_data_[obj->tc_writeindex_] = (timestamp & 0xFF000000) >> 24;
  obj->tc_writeindex_ = (obj->tc_writeindex_ + 1) % FSC_LOG_SIZE;

  return TRUE;
}

FSC_BOOL WritePDToken(struct Log *obj, FSC_BOOL transmitter,
                      USBPD_BufferTokens_t token)
{
  FSC_U8 size = 0;
  if (!RequestSpace(2, obj->pd_writeindex_, obj->pd_readindex_,
                    FSC_LOG_SIZE_PD)) {
    /* If we are out of room, bump up the read index to overwrite old entries */
    /* This is a short msg - only need to overwrite one previous entry */
    size = (obj->pd_data_[obj->pd_readindex_] & 0x1F) + 1;

    obj->pd_readindex_ = (obj->pd_readindex_ + size) % FSC_LOG_SIZE_PD;
    obj->pd_overrun_ = TRUE;
  }

  /* First byte is length-1 and a transmitter flag */
  if (transmitter) {
    obj->pd_data_[obj->pd_writeindex_] = 0x01 | 0x40;
  }
  else {
    obj->pd_data_[obj->pd_writeindex_] = 0x01;
  }
  obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;

  obj->pd_data_[obj->pd_writeindex_] = token & 0x0F;
  obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;

  return TRUE;
}

FSC_BOOL WritePDMsg(struct Log *obj, sopMainHeader_t header,
                    FSC_U8 *dataobject,
                    FSC_BOOL transmitter, FSC_U8 soptoken)
{
  FSC_U8 size = 0;
  FSC_U32 i;
  FSC_U32 required = (header.NumDataObjects * 4) + 2 + 2;

  while (!RequestSpace(required , obj->pd_writeindex_, obj->pd_readindex_,
                       FSC_LOG_SIZE_PD)) {
    /* If we are "out of room", bump up the read index to overwrite old */
    /* entries until we have room for the new message */
    size = (obj->pd_data_[obj->pd_readindex_] & 0x1F) + 1;

    obj->pd_readindex_ = (obj->pd_readindex_ + size) % FSC_LOG_SIZE_PD;
    obj->pd_overrun_ = TRUE;
  }

  /* First byte is length-1, a transmitter flag (0x40), and a msg flag (0x80) */
  if (transmitter) {
    obj->pd_data_[obj->pd_writeindex_] = ((required - 1) & 0x1F) | 0xC0;
  }
  else {
    obj->pd_data_[obj->pd_writeindex_] = ((required - 1) & 0x1F) | 0x80;
  }
  obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;

  /* Second byte is the SOP token */
  obj->pd_data_[obj->pd_writeindex_] = soptoken;
  obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;

  /* Two header bytes */
  obj->pd_data_[obj->pd_writeindex_] = header.byte[0];
  obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;
  obj->pd_data_[obj->pd_writeindex_] = header.byte[1];
  obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;

  /* Finally, the power data objects */
  if (header.NumDataObjects > 0 && dataobject != 0) {
    for (i = 0; i < header.NumDataObjects * 4; i++) {
      obj->pd_data_[obj->pd_writeindex_] = dataobject[i];
      obj->pd_writeindex_ = (obj->pd_writeindex_ + 1) % FSC_LOG_SIZE_PD;
    }
  }

  return TRUE;
}

FSC_BOOL WritePEState(struct Log *obj, FSC_U32 timestamp, FSC_U8 state)
{
  /* This entry requires 5 bytes. */
  /* [TimeMSB][Time][Time][TimeLSB][State] */
  if (!RequestSpace(STATE_ENTRY_LENGTH,obj->pe_writeindex_,obj->pe_readindex_,
                    FSC_LOG_SIZE)){
    /* If we are out of room, bump up the read index to overwrite old entries */
    obj->pe_readindex_ =
      (obj->pe_readindex_ + STATE_ENTRY_LENGTH) % FSC_LOG_SIZE;
    obj->pe_overrun_ = TRUE;
  }

  /* This order of entries is to support the current hostcomm interface. */
  obj->pe_data_[obj->pe_writeindex_] = state;
  obj->pe_writeindex_ = (obj->pe_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->pe_data_[obj->pe_writeindex_] = (timestamp & 0x000000FF);
  obj->pe_writeindex_ = (obj->pe_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->pe_data_[obj->pe_writeindex_] = (timestamp & 0x0000FF00) >>  8;
  obj->pe_writeindex_ = (obj->pe_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->pe_data_[obj->pe_writeindex_] = (timestamp & 0x00FF0000) >> 16;
  obj->pe_writeindex_ = (obj->pe_writeindex_ + 1) % FSC_LOG_SIZE;
  obj->pe_data_[obj->pe_writeindex_] = (timestamp & 0xFF000000) >> 24;
  obj->pe_writeindex_ = (obj->pe_writeindex_ + 1) % FSC_LOG_SIZE;

  return TRUE;
}

FSC_U32 ReadTCLog(struct Log *obj, FSC_U8 *data, FSC_U32 datalength)
{
  FSC_U32 byteswritten = 1;
  FSC_U8 logEntries = 0;
  FSC_U32 i = 0;

  /* Do we have messages to read? */
  while (obj->tc_readindex_ != obj->tc_writeindex_) {
    /* Check return buffer space */
    if (byteswritten + STATE_ENTRY_LENGTH >= datalength)
      break;

    /* Copy the log data */
    for (i = 0; i < STATE_ENTRY_LENGTH; ++i) {
      data[byteswritten++] = obj->tc_data_[obj->tc_readindex_];
      obj->tc_readindex_ = (obj->tc_readindex_ + 1) % FSC_LOG_SIZE;
    }

    ++logEntries;
  }

  data[0] = logEntries;

  obj->tc_overrun_ = FALSE;

  return byteswritten;
}

FSC_BOOL ReadTCEntry(struct Log *obj, FSC_U16 *state, FSC_U16 *timeMS10ths,
                                                      FSC_U16 *timeSeconds)
{
  FSC_U16 value = 0;
  if (obj->tc_readindex_ != obj->tc_writeindex_) {
    *state = obj->tc_data_[obj->tc_readindex_];
    obj->tc_readindex_ = (obj->tc_readindex_ + 1) % FSC_LOG_SIZE;

    value = obj->tc_data_[obj->tc_readindex_ + 1];
    value = value << 8;
    value += obj->tc_data_[obj->tc_readindex_];
    *timeMS10ths = value;
    obj->tc_readindex_ = (obj->tc_readindex_ + 2) % FSC_LOG_SIZE;

    value = obj->tc_data_[obj->tc_readindex_ + 1];
    value = value << 8;
    value += obj->tc_data_[obj->tc_readindex_];
    *timeSeconds = value;
    obj->tc_readindex_ = (obj->tc_readindex_ + 2) % FSC_LOG_SIZE;

    return TRUE;
  }
  else {
    return FALSE;
  }
}

FSC_U32 ReadPELog(struct Log *obj, FSC_U8 *data, FSC_U32 datalength)
{
  FSC_U32 byteswritten = 1;
  FSC_U8 logEntries = 0;
  FSC_U32 i = 0;

  /* Do we have messages to read? */
  while (obj->pe_readindex_ != obj->pe_writeindex_) {
    /* Check return buffer space */
    if (byteswritten + STATE_ENTRY_LENGTH >= datalength)
      break;

    /* Copy the log data */
    for (i = 0; i < STATE_ENTRY_LENGTH; ++i) {
      data[byteswritten++] = obj->pe_data_[obj->pe_readindex_];
      obj->pe_readindex_ = (obj->pe_readindex_ + 1) % FSC_LOG_SIZE;
    }

    ++logEntries;
  }

  data[0] = logEntries;

  obj->pe_overrun_ = FALSE;

  return byteswritten;
}

FSC_BOOL ReadPEEntry(struct Log *obj, FSC_U16 *state, FSC_U16 *timeMS10ths,
                                                      FSC_U16 *timeSeconds)
{
  FSC_U16 value = 0;
  if (obj->pe_readindex_ != obj->pe_writeindex_) {
    *state = obj->pe_data_[obj->pe_readindex_];
    obj->pe_readindex_ = (obj->pe_readindex_ + 1) % FSC_LOG_SIZE;

    value = obj->pe_data_[obj->pe_readindex_ + 1];
    value = value << 8;
    value += obj->pe_data_[obj->pe_readindex_];
    *timeMS10ths = value;
    obj->pe_readindex_ = (obj->pe_readindex_ + 2) % FSC_LOG_SIZE;

    value = obj->pe_data_[obj->pe_readindex_ + 1];
    value = value << 8;
    value += obj->pe_data_[obj->pe_readindex_];
    *timeSeconds = value;
    obj->pe_readindex_ = (obj->pe_readindex_ + 2) % FSC_LOG_SIZE;

    return TRUE;
  }
  else {
    return FALSE;
  }
}

FSC_U32 ReadPDLog(struct Log *obj, FSC_U8 *data, FSC_U32 datalength)
{
  FSC_U32 byteswritten = 0;
  FSC_U32 nextlength = 0;
  FSC_U32 i = 0;

  i = get_pd_bytes(obj);
  if (i > 0xFF) i = 0xFF;

  data[byteswritten++] = (FSC_U8)i;
  data[byteswritten++] = 0; /* Place holder for byteswritten */

  /* Do we have messages to read? */
  while (obj->pd_readindex_ != obj->pd_writeindex_) {
    /* Check return buffer space */
    nextlength = (obj->pd_data_[obj->pd_readindex_] & 0x1F) + 1;

    if (byteswritten + nextlength >= datalength)
      break;

    /* Copy the log data */
    for (i = 0; i < nextlength; ++i) {
      data[byteswritten++] = obj->pd_data_[obj->pd_readindex_];
      obj->pd_readindex_ = (obj->pd_readindex_ + 1) % FSC_LOG_SIZE_PD;
    }
  }

  obj->pd_overrun_ = FALSE;

  /* Store number of msg bytes written */
  data[1] = byteswritten - 2;

  return byteswritten;
}

FSC_U32 get_pd_bytes(struct Log *obj)
{
  /* Empty? */
  if (obj->pd_writeindex_ == obj->pd_readindex_) {
    return 0;
  }
  /* Else, Write index is ahead? */
  else if (obj->pd_writeindex_ > obj->pd_readindex_) {
    return obj->pd_writeindex_ - obj->pd_readindex_;
  }
  /* Else, Write index has wrapped around */
  else {
    return (FSC_LOG_SIZE_PD - obj->pd_readindex_) + obj->pd_writeindex_;
  }
}

FSC_BOOL RequestSpace(FSC_U32 length, FSC_U8 writeidx, FSC_U8 readidx,
                      FSC_U32 logsize) {
  /* First check for too long */
  if (length >= logsize) {
    return FALSE;
  }
  /* Else, Empty? */
  else if (writeidx == readidx) {
    return TRUE;
  }
  /* Else, Write index is ahead? */
  else if (writeidx > readidx) {
    FSC_U32 newindex = (writeidx + length) % logsize;

    /* Still in the clear? */
    if (newindex > writeidx) {
      return TRUE;
    }
    /* Else, newindex wrapped around */
    else {
      if (newindex < readidx) {
        return TRUE;
      }
    }
  }
  /* Else, Write index has wrapped around */
  else {
    if ((writeidx + length) < readidx) {
      return TRUE;
    }
  }

  /* Otherwise... */
  return FALSE;
}

#endif /* FSC_LOGGING */

