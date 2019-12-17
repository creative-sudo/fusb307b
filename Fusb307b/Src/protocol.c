/*******************************************************************************
 * @file     protocol.c
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
 * protocol.c
 *
 * Implements the Protocol state machine functions
 */

#include "protocol.h"

#include "platform.h"
#include "port.h"
#include "timer.h"
#include "dpm.h"
#include "observer.h"

void USBPDProtocol(struct Port *port)
{
  /* Received hard reset? */
  if (port->registers_.AlertL.I_RXHRDRST) {
    ClearInterrupt(port, regALERTL, MSK_I_RXHRDRST);
    /* We are forcing the state machine to new state so disable
     * timers if it was being used. */
    TimerDisable(&port->policy_state_timer_);
    if (port->policy_is_source_) {
      TimerStart(&port->policy_state_timer_, ktPSHardReset);
      set_policy_state(port, PE_SRC_Transition_To_Default);
    }
    else {
      set_policy_state(port, PE_SNK_Transition_To_Default);
    }

    /* TODO check if we need to exit from any entered mode. */

    /* notify that TCPC has undergone hard reset. */
    notify_observers(EVENT_HARD_RESET, port->port_id_, 0);
    port->is_hard_reset_ = TRUE;
    port->policy_subindex_ = 0;

#ifdef FSC_LOGGING
    /* Store the hard reset */
    WritePDToken(&port->log_, FALSE, pdtHardResetRxd);
#endif /* FSC_LOGGING */
  }
  else {
    switch (port->protocol_state_) {
    case PRLReset:
      /* Hard reset reg write has already been done - monitor progress here. */
      port->pd_tx_status_ = txWait;
      port->protocol_state_ = PRLResetWait;
      TimerStart(&port->no_response_timer_, ktHardResetComplete);
      break;
    case PRLResetWait:
      ProtocolResetWait(port);
      break;
    case PRLRxWait:
      break;
    case PRLIdle:
      /* Waiting to send or receive a message */
      ProtocolIdle(port);
      break;
    case PRLTxSendingMessage:
      /* We have attempted to transmit and are waiting for it to */
      /* complete or detect a collision */
      ProtocolSendingMessage(port);
      break;
    case PRLDisabled:
      /* In the disabled state, don't do anything */
      break;
    default:
      break;
    }
  }
}

void ProtocolIdle(struct Port *port)
{
  if (port->pd_tx_status_ == txReset) {
    /* If we need to send a hard reset */
    port->protocol_state_ = PRLReset;
  }
  else if (port->registers_.AlertL.I_RXSTAT) {
    /* If we have received a message */
    ProtocolGetRxPacket(port);

    /* If we happened to get here by receiving a msg during a sinktx event,
     * receive the new msg first and then disable the attempted sinktx event.
     */
    if (port->registers_.SinkTransmit.DIS_SNK_TX == 0) {
      port->registers_.SinkTransmit.DIS_SNK_TX = 1;
      WriteRegister(port, regSINK_TRANSMIT);
    }

    /* Tx interrupted by incoming message? */
    if (port->pd_tx_status_ == txSend) {
        port->pd_tx_status_ = txAbort;
    }
    else {
        port->pd_tx_status_ = txIdle;
    }

#ifdef FSC_HAVE_EXTENDED
    /* Need to request a next chunk? */
    if (port->protocol_ext_state_active_ &&
       (port->protocol_ext_request_chunk_ ||
        port->protocol_ext_send_chunk_)) {
      port->pd_tx_status_ = txSend;
      ProtocolTransmitMessage(port);
      port->idle_ = TRUE;
    }
#endif /* FSC_HAVE_EXTENDED */
  }
  else if (port->pd_tx_status_ == txSend) {
    /* Otherwise check to see if there has been a request to send data... */
    if (port->protocol_msg_rx_) {
      /* Rx'd message before we can send - abort and handle. */
      port->pd_tx_status_ = txAbort;
    }
    else {
      ProtocolTransmitMessage(port);
    }
  }
}

void ProtocolResetWait(struct Port *port)
{
  /* Hard reset sent interrupt signaled by I_TXSUCC && I_TXFAIL */
  if ((port->registers_.AlertL.I_TXSUCC && port->registers_.AlertL.I_TXFAIL) ||
      TimerExpired(&port->no_response_timer_)) {
    /* Wait for the reset sequence to complete */
    ClearInterrupt(port, regALERTL, MSK_I_TXSUCC | MSK_I_TXFAIL);

    port->protocol_state_ = PRLIdle;
    port->pd_tx_status_ = txSuccess;

    notify_observers(EVENT_HARD_RESET, port->port_id_, 0);
    TimerDisable(&port->no_response_timer_);
  }
}

void ProtocolGetRxPacket(struct Port *port)
{
  FSC_U8 i = 0, j = 0;
#ifdef FSC_LOGGING
  sopMainHeader_t temp_GCRCHeader = {0};
#endif /* FSC_LOGGING */
#ifdef FSC_HAVE_EXTENDED
  sopExtendedHeader_t temp_ExtHeader = {0};
#endif /* FSC_HAVE_EXTENDED */

  /* Read the Rx token, two header bytes, and the byte count */
  ReadRegisters(port, regRXBYTECNT, 4);

  port->policy_rx_header_.byte[0] = port->registers_.RxHeadL;
  port->policy_rx_header_.byte[1] = port->registers_.RxHeadH;

#ifdef FSC_LOGGING
  /* Only setting the Tx header here so that we can store what we */
  /* expect was sent in our PD buffer for the GUI */
  temp_GCRCHeader.NumDataObjects = 0;
  temp_GCRCHeader.MessageType = CMTGoodCRC;
  temp_GCRCHeader.PortDataRole = port->policy_is_dfp_;
  temp_GCRCHeader.PortPowerRole = port->policy_is_source_;
  temp_GCRCHeader.SpecRevision = USBPDSPECREV;
  temp_GCRCHeader.MessageID = port->policy_rx_header_.MessageID;
#endif /* FSC_LOGGING */

  /* Figure out what SOP* the data came in on and record the sender */
  port->protocol_msg_rx_sop_ = TokenToSopType(port->registers_.RxStat.RX_SOP);

  if ((port->policy_rx_header_.NumDataObjects == 0) &&
      (port->policy_rx_header_.MessageType == CMTSoftReset)) {
    /* Clear the message ID counter for tx */
    port->message_id_counter_[port->protocol_msg_rx_sop_] = 0;

    /* Reset the message ID (always alert policy engine of soft reset) */
    port->message_id_[port->protocol_msg_rx_sop_] = 0xFF;

#ifdef FSC_HAVE_USBHID
    /* Set the source caps updated flag to trigger a GUI update */
    port->source_caps_updated_ = TRUE;
#endif /* FSC_HAVE_USBHID */
  }
  else if (port->policy_rx_header_.MessageID !=
           port->message_id_[port->protocol_msg_rx_sop_]) {
    /* A received message should have an incremented (+1) message ID,
     * so we'll update our value here */
    port->message_id_[port->protocol_msg_rx_sop_] =
      port->policy_rx_header_.MessageID;
  }
  else {
    /* Drop anything else - possible retried message with same ID */
    ClearInterrupt(port, regALERTL, MSK_I_RXSTAT);
    return;
  }

  /* Did we receive a data message? If so, we want to retrieve the data */
  if (port->policy_rx_header_.NumDataObjects > 0) {
    ReadRxRegisters(port, port->policy_rx_header_.NumDataObjects * 4);
#ifdef FSC_HAVE_EXTENDED
    if (port->policy_rx_header_.Extended == 1) {
      /* Extended message */
      if (((port->protocol_msg_rx_sop_ == SOP_TYPE_SOP) && !port->dpm_pd_30_) ||
          ((port->protocol_msg_rx_sop_ == SOP_TYPE_SOP1) &&
           !port->dpm_pd_30_srccab_)) {
        /* Not allowed */
        set_policy_state(port, PE_SRC_Send_Not_Supported);
      }
      else {
        temp_ExtHeader.byte[0] = port->registers_.RxData[0];
        temp_ExtHeader.byte[1] = port->registers_.RxData[1];

        if (temp_ExtHeader.RequestChunk == 1) {
          /* Rec'd a request chunk - tell the transmit function to continue */
          port->protocol_ext_send_chunk_ = TRUE;

          platform_printf(port->port_id_, "Rx'd Req Chnk\n", -1);
        }
        else if (temp_ExtHeader.DataSize <= MAX_EXT_MSG_LEGACY_LEN) {
          /* Single extended packet */
          port->protocol_ext_num_bytes_ = temp_ExtHeader.DataSize;

          for (i = 0; i < temp_ExtHeader.DataSize; ++i) {
            port->protocol_ext_buffer_[i] = port->registers_.RxData[2 + i];
          }

          /* Set the flag to pass the message to the policy engine */
          port->protocol_msg_rx_ = TRUE;
          platform_printf(port->port_id_, "Rx'd Ext Msg - 1 Chnk\n", -1);
        }
        else if (port->protocol_chunking_supported_) {
          port->protocol_ext_request_chunk_ = TRUE;
          port->protocol_ext_state_active_ = TRUE;

          platform_printf(port->port_id_, "Rx'd Chnk: \n",
                  temp_ExtHeader.ChunkNumber);

          if (temp_ExtHeader.ChunkNumber == 0) {
            /* First Chunk */
            port->protocol_ext_chunk_number_ = 0;
            port->protocol_ext_num_bytes_ = 0;
          }

          for (i = 0; i < MAX_EXT_MSG_LEGACY_LEN; ++i) {
            port->protocol_ext_buffer_[port->protocol_ext_num_bytes_] =
                port->registers_.RxData[2 + i];
            port->protocol_ext_num_bytes_ += 1;

            if (port->protocol_ext_num_bytes_ == temp_ExtHeader.DataSize) {
              /* Done */
              port->protocol_ext_request_chunk_ = FALSE;
              port->protocol_ext_state_active_ = FALSE;
              platform_printf(port->port_id_, "Ext Done - Bytes: \n",
                  port->protocol_ext_num_bytes_);

              /* Set the flag to pass the message to the policy engine */
              port->protocol_msg_rx_ = TRUE;

              break;
            }
          }

          port->protocol_ext_chunk_number_ += 1;

          if (port->protocol_ext_request_chunk_) {
            platform_printf(port->port_id_, "Req Next Chnk\n", -1);
            port->protocol_ext_request_cmd_ =
                    port->policy_rx_header_.MessageType;
          }
        }
        else {
          set_policy_state(port, PE_SRC_Chunk_Received);
        }
      }
    }
    else
#endif /* FSC_HAVE_EXTENDED */
    {
      /* Standard data message */
      /* Load the FIFO data into the data objects (loop through each object) */
      for (i = 0; i < port->policy_rx_header_.NumDataObjects; i++) {
        /* Loop through each byte in the object */
        for (j = 0; j < 4; j++) {
          /* Store the actual bytes */
          port->policy_rx_data_obj_[i].byte[j] =
            port->registers_.RxData[(i * 4) + j];
        }
      }
      /* Set the flag to pass the message to the policy engine */
      port->protocol_msg_rx_ = TRUE;
    }
  }
  else {
    /* Command message received */
    /* Set the flag to pass the message to the policy engine */
    port->protocol_msg_rx_ = TRUE;
  }

  /* Clear the interrupt here, as it also clears the RX data registers */
  ClearInterrupt(port, regALERTL, MSK_I_RXSTAT);

  /* Delay to allow for a fast, possibly interrupting next message */
  /* Fixes compliance issue VDM interrupt VDM command. Ellisys VDMU.E17 */
  if(port->policy_rx_header_.NumDataObjects != 0 &&
     port->policy_rx_header_.MessageType == DMTVendorDefined) {

    /* Delay may need to be reduced or removed for slower response
     * implementations - e.g. kernel driver.
     */
    platform_delay(3 * 1000);

    ReadRegister(port, regALERTL);

    if (port->registers_.AlertL.I_RXSTAT) {
      ProtocolGetRxPacket(port);
    }
  }

#ifdef FSC_LOGGING
  /* Time-stamped log entry */
  WritePEState(&port->log_, platform_timestamp(), DBG_Rx_Packet);

  /* Store the received PD message */
  if (port->policy_rx_header_.Extended == 0) {
    WritePDMsg(&port->log_, port->policy_rx_header_,
               (FSC_U8 *)port->policy_rx_data_obj_,
               FALSE, port->protocol_msg_rx_sop_);
  }
#ifdef FSC_HAVE_EXTENDED
  else {
    WritePDMsg(&port->log_, port->policy_rx_header_,
               port->registers_.RxData, FALSE, port->protocol_msg_rx_sop_);
  }
#endif /* FSC_HAVE_EXTENDED */

  /* Store the (recreated) GoodCRC message that we have sent */
  WritePDMsg(&port->log_, temp_GCRCHeader, 0, TRUE, port->protocol_msg_rx_sop_);
#endif /* FSC_LOGGING */
}

void ProtocolTransmitMessage(struct Port *port)
{
  FSC_U8 i = 0, j = 0;
  sopMainHeader_t temp_TxHeader = {0};
#ifdef FSC_HAVE_EXTENDED
  sopExtendedHeader_t temp_ExtHeader = {0};
#endif /* FSC_HAVE_EXTENDED */
  FSC_U8 bytestosend = 0;

  temp_TxHeader.word = port->policy_tx_header_.word;

  /* Handle soft reset case - clearing message counters */
  if ((temp_TxHeader.NumDataObjects == 0) &&
      (temp_TxHeader.MessageType == CMTSoftReset)) {
      port->message_id_counter_[port->protocol_msg_tx_sop_] = 0;
      port->message_id_[port->protocol_msg_tx_sop_] = 0xFF;

#ifdef FSC_HAVE_USBHID
      /* Set the source caps updated flag to trigger an update of the GUI */
      port->source_caps_updated_ = TRUE;
#endif /* FSC_HAVE_USBHID */
  }

#ifdef FSC_HAVE_EXTENDED
  if (port->protocol_ext_request_chunk_) {
    port->protocol_ext_request_chunk_ = FALSE;
    /* Special case where we are in the middle of receiving an ext msg
     * Here we need to build an intermediate chunk request message.
     */
    platform_printf(port->port_id_, "Send Chnk Req\n", -1);

    port->protocol_msg_tx_sop_ = port->protocol_msg_rx_sop_;

    temp_TxHeader.word = 0;
    temp_TxHeader.Extended = TRUE;
    temp_TxHeader.NumDataObjects = 1;
    temp_TxHeader.MessageType = port->protocol_ext_request_cmd_ & PDMsgTypeMask;
    temp_TxHeader.PortPowerRole = port->policy_is_source_;
    temp_TxHeader.SpecRevision =
            DPM_CurrentSpecRev(port, port->protocol_msg_tx_sop_);

    if (port->protocol_msg_tx_sop_ == SOP_TYPE_SOP) {
      /* This field is reserved for SOP', SOP'' */
      temp_TxHeader.PortDataRole = port->policy_is_dfp_;
    }

    temp_ExtHeader.Chunked = 1;
    temp_ExtHeader.RequestChunk = 1;
    temp_ExtHeader.ChunkNumber = port->protocol_ext_chunk_number_;

    port->registers_.TxData[0] = temp_ExtHeader.byte[0];
    port->registers_.TxData[1] = temp_ExtHeader.byte[1];
    port->registers_.TxData[2] = 0;
    port->registers_.TxData[3] = 0;
    bytestosend = 4;

#ifdef FSC_LOGGING
    /* For logging purpose */
    for (i = 0; i < bytestosend; i++)
    {
      ((FSC_U8*)port->policy_tx_data_obj_[0].byte)[i] =
                port->registers_.TxData[i];
    }
#endif
  }
  else if (port->protocol_ext_send_chunk_ == TRUE &&
      temp_TxHeader.Extended == TRUE) {
    port->protocol_ext_send_chunk_ = FALSE;
    /* Otherwise, are we sending an extended message? */
    temp_ExtHeader.Chunked = 1;
    temp_ExtHeader.ChunkNumber = port->protocol_ext_chunk_number_;
    temp_ExtHeader.DataSize = port->protocol_ext_num_bytes_;

    platform_printf(port->port_id_, "Send Chnk: \n",
            temp_ExtHeader.ChunkNumber);

    if (port->protocol_ext_chunk_number_ == 0) {
      if (port->protocol_ext_num_bytes_ <= MAX_EXT_MSG_LEGACY_LEN) {
        /* Chunk 0, and only one chunk to send */
        bytestosend = port->protocol_ext_num_bytes_;
      }
      else {
        /* First chunk, more than one chunk to send - fill the buffer */
        bytestosend = MAX_EXT_MSG_LEGACY_LEN;
      }
    }
    else {
      /* Chunk(s) 1 - N */
      bytestosend = (port->protocol_ext_num_bytes_ -
        (port->protocol_ext_chunk_number_ * MAX_EXT_MSG_LEGACY_LEN));

      if (bytestosend <= MAX_EXT_MSG_LEGACY_LEN) {
        /* Last Chunk */
      }
      else {
        /* Continuing... */
        bytestosend = MAX_EXT_MSG_LEGACY_LEN;
      }
    }

    /* Load the data */
    port->registers_.TxData[0] = temp_ExtHeader.byte[0];
    port->registers_.TxData[1] = temp_ExtHeader.byte[1];

    for (i = 0; i < bytestosend; ++i) {
      port->registers_.TxData[2 + i] = port->protocol_ext_buffer_
        [(port->protocol_ext_chunk_number_ * MAX_EXT_MSG_LEGACY_LEN) + i];
    }

    /* Add extended header to length */
    bytestosend += 2;

    /* Pad with 0's to 4-byte (data object) boundary. */
    for (; (i < MAX_EXT_MSG_LEGACY_LEN) &&
           (bytestosend % 4 != 0); ++i, ++bytestosend) {
      port->registers_.TxData[2 + i] = 0;
    }

    /* Update NumDataObjects accordingly, 4-byte boundary adjusted */
    temp_TxHeader.NumDataObjects = bytestosend / 4;

    /* Increment chunk number for next time through */
    port->protocol_ext_chunk_number_ += 1;
#ifdef FSC_LOGGING
    /* For logging purpose */
    for (i = 0; i < bytestosend; i++)
    {
      ((FSC_U8*)port->policy_tx_data_obj_[0].byte)[i] =
                port->registers_.TxData[i];
    }
#endif
  }
  else
#endif /* FSC_HAVE_EXTENDED */
  {
    /* Not extended messaging */
    bytestosend = temp_TxHeader.NumDataObjects * 4;

    if (temp_TxHeader.NumDataObjects > 0) {
      /* If this is a data object... */
      for (i = 0; i < temp_TxHeader.NumDataObjects; i++) {
        /* Load the data objects */
        for (j = 0; j < 4; ++j) {
          /* Loop through each byte in the object */
          port->registers_.TxData[(i * 4) + j] =
            port->policy_tx_data_obj_[i].byte[j];
        }
      }
    }
  }

  /* Update the tx message id to send */
  temp_TxHeader.MessageID =
    port->message_id_counter_[port->protocol_msg_tx_sop_];

  /* TXBYTECNT = number of bytes in the packet plus the 2-byte main header */
  port->registers_.TxByteCnt = 2 + bytestosend;

  /* Load in the header */
  port->registers_.TxHeadL = temp_TxHeader.byte[0];
  port->registers_.TxHeadH = temp_TxHeader.byte[1];

  /* Commit to device */
  WriteRegisters(port, regTXBYTECNT, 3);

  /* Commit the TxData array to the device */
  WriteTxRegisters(port, bytestosend);

  /* Send the SOP indicator to enable the transmitter */
  if (port->protocol_use_sinktx_ == FALSE) {
    port->registers_.Transmit.TX_SOP = port->protocol_msg_tx_sop_;
    port->registers_.Transmit.RETRY_CNT = port->protocol_retries_;
    WriteRegister(port, regTRANSMIT);

    /* Disable SinkTX for normal transmits */
    if (port->registers_.SinkTransmit.DIS_SNK_TX == 0) {
      port->registers_.SinkTransmit.DIS_SNK_TX = 1;
      WriteRegister(port, regSINK_TRANSMIT);
    }
  }
  else
  {
    port->registers_.SinkTransmit.DIS_SNK_TX = 0;
    port->registers_.SinkTransmit.TX_SOP = port->protocol_msg_tx_sop_;
    port->registers_.SinkTransmit.RETRY_CNT = port->protocol_retries_;
    WriteRegister(port, regSINK_TRANSMIT);

    // Clear for next time...
    port->protocol_use_sinktx_ = FALSE;
  }

  /* Move on to waiting for a success or fail */
  port->pd_tx_status_ = txBusy;
  port->protocol_state_ = PRLTxSendingMessage;

  /* Timeout specifically for chunked messages, but used with each transmit
   * to prevent a theoretical protocol hang.
   */
  TimerStart(&port->protocol_timer_, ktChunkSenderRequest);

#ifdef FSC_LOGGING
  /* Time-stamped log entry */
  WritePEState(&port->log_, platform_timestamp(), DBG_Tx_Packet);

  /* Store all messages that we attempt to send for debugging */
  WritePDMsg(&port->log_, temp_TxHeader,
             (FSC_U8 *)port->policy_tx_data_obj_,
             TRUE, port->protocol_msg_tx_sop_);
#endif /* FSC_LOGGING */
}

void ProtocolSendingMessage(struct Port *port)
{
  SopType rx_sop = port->protocol_msg_tx_sop_;
#ifdef FSC_LOGGING
  sopMainHeader_t header;
#endif /* FSC_LOGGING */

  if (port->registers_.AlertL.I_TXSUCC) {
    ClearInterrupt(port, regALERTL, MSK_I_TXSUCC);

#ifdef FSC_LOGGING
    /* The 307 doesn't provide received goodcrc messages, */
    /* so create a temporary one here for logging. */
    /* TODO - might need to use the opposite for dfp/source */
    header.word = 0;
    header.MessageType = CMTGoodCRC;
    header.PortDataRole = !port->policy_is_dfp_;
    header.SpecRevision = PDSpecRev2p0;
    header.PortPowerRole = !port->policy_is_source_;
    header.MessageID = port->message_id_counter_[rx_sop];
    header.NumDataObjects = 0;

    WritePDMsg(&port->log_, header, 0, FALSE, rx_sop);
#endif /* FSC_LOGGING */

    /* Transmission successful */
    port->message_id_counter_[rx_sop] =
      (port->message_id_counter_[rx_sop] + 1) & 0x07;
    port->protocol_state_ = PRLIdle;
    port->pd_tx_status_ = txSuccess;
  }
  else if (port->registers_.AlertL.I_TXDISC) {
    ClearInterrupt(port, regALERTL, MSK_I_TXDISC);

    port->message_id_counter_[rx_sop] =
         (port->message_id_counter_[rx_sop] + 1) & 0x07;
    /* Indicate to the policy engine that there was a collision */
    port->pd_tx_status_ = txCollision;
    /* Go to the Idle state to receive whatever message is incoming... */
    port->protocol_state_ = PRLIdle;
  }
  else if (port->registers_.AlertL.I_TXFAIL) {
    ClearInterrupt(port, regALERTL, MSK_I_TXFAIL);

    /* Transmission failed */
    port->protocol_state_ = PRLIdle;
    port->pd_tx_status_ = txError;
    port->message_id_counter_[rx_sop] =
      (port->message_id_counter_[rx_sop] + 1) & 0x07;
  }
  else if (port->registers_.AlertL.I_RXSTAT) {
    /* TODO - possibly determine better mechanism for handling sink transmit
     * discarded messages.  For now just go back to idle...
     */
    ProtocolGetRxPacket(port);
    if (port->policy_rx_header_.Extended == 0 &&
        port->policy_rx_header_.NumDataObjects == 0 &&
        port->policy_rx_header_.MessageType == CMTGoodCRC &&
        port->policy_tx_header_.Extended == 0 &&
        port->policy_tx_header_.NumDataObjects == 0 &&
        port->policy_tx_header_.MessageType == CMTGetSourceCapExt)
    {
      /* GoodCRC response for sent GSCE message */
      port->protocol_msg_rx_ = FALSE;
      port->message_id_counter_[rx_sop] =
        (port->message_id_counter_[rx_sop] + 1) & 0x07;
    }
    else
    {
      set_policy_state(port, PE_SNK_Ready);
    }
    port->protocol_state_ = PRLIdle;
    port->pd_tx_status_ = txSuccess;
  }
}

void ProtocolSendHardReset(struct Port *port, FSC_BOOL cable)
{
  /* Set the send hard reset TRANSMIT register code */
  FSC_U8 data = cable ? TRANSMIT_CABLERESET : TRANSMIT_HARDRESET;

  /* If this flag is set, we've already sent the hard reset command */
  if (port->waiting_on_hr_) {
    port->waiting_on_hr_ = FALSE;
  }
  else {
    /* Send the hard reset */
    platform_i2c_write(port->i2c_addr_, regTRANSMIT, 1, &data);
  }

  port->pd_tx_status_ = txReset;
  port->protocol_state_ = PRLReset;

#ifdef FSC_LOGGING
  /* Store the hard reset */
  WritePDToken(&port->log_, TRUE, cable ? pdtCableReset : pdtHardResetTxd);
#endif /* FSC_LOGGING */
}

SopType TokenToSopType(FSC_U8 data)
{
  SopType ret = SOP_TYPE_ERROR;

  /* Figure out what SOP* the data came in on */
  /* The register value from the FUSB307 maps directly to our SOP_TYPE_ enum */
  if ((data & 0x0b00000111) <= SOP_TYPE_LAST_VALUE) {
    ret = (SopType)(data & 0x0b00000111);
  }

  return ret;
}
