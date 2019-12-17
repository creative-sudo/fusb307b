/*******************************************************************************
 * @file     vdm.h
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
/* **************************************************************************
 *  Declares VDM functionality API.
 * ************************************************************************** */
#ifndef FSCPM_VDM_H_
#define FSCPM_VDM_H_

#ifdef FSC_HAVE_VDM

#ifdef FSC_HAVE_DP
#include "display_port_types.h"
#endif /*  FSC_HAVE_DP */
#include "platform.h" /*  Driver typedefs */
#include "PDTypes.h"  /*  SopType, etc */
#include "port.h"     /*  Port class interface */

#define NUM_VDM_MODES 6
#define MAX_NUM_SVIDS_PER_SOP 30
#define MAX_SVIDS_PER_MESSAGE 12
#define MIN_DISC_ID_RESP_SIZE 3

/* TODO: verify for removal */
/*
 *  Configures the VDM code
 */
//void ConfigureVdmResponses(struct Port *port, FSC_U8 *bytes);
//void ReadVdmConfiguration(struct Port *port, FSC_U8 *data);

/* Determine SVDM version by checking current Spec Rev */
SvdmVersion CurrentSVDMVersion(struct Port *port, SopType sop);

/*  VDM policy state machine */
void PolicyVdm(struct Port *port);
/*  Handles the port's peGiveVdm policy state */
void PolicyGiveVdm(struct Port *port);
/*  Converts the port's VDM message and processes it */
void ConvertAndProcessVdmMessage(struct Port *port);
/*  Processes the port's current VDM command */
void DoVdmCommand(struct Port *port);
/*  This assumes we're already in either Source or Sink Ready states! */
void AutoVdmDiscovery(struct Port *port);

/*  Sends vdm messages */
void SendVdmMessage(struct Port *port, SopType sop, FSC_U32 *arr,
                    FSC_U32 length, PolicyState_t next_ps);
void SendVdmMessageWithTimeout(struct Port *port, SopType sop, FSC_U32 *arr,
                    FSC_U32 length, PolicyState_t n_pe);

/*
 * Initiations from DPM
 * Issues Discover Identity commands
 * Discover Identity only valid for SOP/SOP'
 * returns 0 if successful
 * returns > 0 if not SOP or SOP', or if Policy State is wrong
 */
FSC_S32 RequestDiscoverIdentity(struct Port *port, SopType sop);

/*  Issues Discover SVID commands, valid with SOP* */
FSC_S32 RequestDiscoverSvids(struct Port *port, SopType sop);

/*  Issues Discover Modes command, valid with SOP*. */
FSC_S32 RequestDiscoverModes(struct Port *port, SopType sop, FSC_U16 svid);

/*  DPM (UFP) calls this to request sending an attention cmd to specified sop */
FSC_S32 RequestSendAttention(struct Port *port, SopType sop, FSC_U16 svid,
                             FSC_U8 mode);

/*  Enter mode specified by SVID and mode index */
FSC_S32 RequestEnterMode(struct Port *port, SopType sop, FSC_U16 svid,
                         FSC_U32 mode_index);

/*  exit mode specified by SVID and mode index */
FSC_S32 RequestExitMode(struct Port *port, SopType sop, FSC_U16 svid,
                        FSC_U32 mode_index);

/*  exits all modes (TODO) */
FSC_S32 RequestExitAllModes(struct Port *port);

/*  receiving end */
/*  Call when VDM messages are received - returns 0 on success, 1+ otherwise */
FSC_S32 ProcessVdmMessage(struct Port *port, FSC_U32* arr, FSC_U32 length);
FSC_S32 ProcessDiscoverIdentity(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);
FSC_S32 ProcessDiscoverSvids(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);
FSC_S32 ProcessDiscoverModes(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);
FSC_S32 ProcessEnterMode(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);
FSC_S32 ProcessExitMode(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);
FSC_S32 ProcessAttention(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);
FSC_S32 ProcessSvidSpecific(struct Port *port, SopType sop,
                          FSC_U32* arr_in, FSC_U32 length_in);

void StartVdmTimer(struct Port *port);
void ResetPolicyState(struct Port *port, SopType sop);

/*  TODO: These are the "vdm callback" functions from the 302.
 *  Make sure this is the best place to put these */
/*  VDM handling functions that will likely be customized by vendors */
Identity VdmRequestIdentityInfo(struct Port *port, SopType sop);
SvidInfo VdmRequestSvidInfo(struct Port *port);
ModesInfo VdmRequestModesInfo(struct Port *port, FSC_U16 svid);
FSC_BOOL VdmModeEntryRequest(struct Port *port, FSC_U16 svid,
                          FSC_U32 mode_index);
FSC_BOOL VdmModeExitRequest(struct Port *port, FSC_U16 svid,
                          FSC_U32 mode_index);
FSC_BOOL VdmEnterModeResult(struct Port *port, FSC_BOOL success, FSC_U16 svid,
                          FSC_U32 mode_index);
void VdmExitModeResult(struct Port *port, FSC_BOOL success, FSC_U16 svid,
                          FSC_U32 mode_index);
void VdmInformIdentity(struct Port *port, FSC_BOOL success, SopType sop,
                          Identity id);
void VdmInformSvids(struct Port *port, FSC_BOOL success, SopType sop,
                          SvidInfo svid_info);
void VdmInformModes(struct Port *port, FSC_BOOL success, SopType sop,
                          ModesInfo modes_info);
void VdmInformAttention(struct Port *port, FSC_U16 svid, FSC_U8 mode_index);
void VdmInitDpm(struct Port *port);

FSC_BOOL evalResponseToSopVdm(struct Port *port, doDataObject_t vdm_hdr);
FSC_BOOL evalResponseToCblVdm(struct Port *port, doDataObject_t vdm_hdr);

/*
 *  Functions that convert bits into internal header representations.
 *
 *  Returns structured/unstructured vdm type
 */
VdmType getVdmTypeOf(FSC_U32 in);

/*  Converts 32 bits into an unstructured vdm header struct */
UnstructuredVdmHeader getUnstructuredVdmHeader(FSC_U32 in);

/*  Converts 32 bits into a structured vdm header struct */
StructuredVdmHeader getStructuredVdmHeader(FSC_U32 in);

/*  Converts 32 bits into an ID Header struct */
IdHeader getIdHeader(FSC_U32 in);

/*  Functions that convert internal header representations into bits... */
/*  Converts unstructured vdm header struct into 32 bits */
FSC_U32 getBitsForUnstructuredVdmHeader(UnstructuredVdmHeader in);

/*  Converts structured vdm header struct into 32 bits */
FSC_U32 getBitsForStructuredVdmHeader(StructuredVdmHeader in);

/*  Converts ID Header struct into 32 bits */
FSC_U32 getBitsForIdHeader(IdHeader in);

/*  Functions that convert bits into internal VDO representations... */
CertStatVdo getCertStatVdo(FSC_U32 in);
ProductVdo getProductVdo(FSC_U32 in);
CableVdo getCableVdo(FSC_U32 in);
AmaVdo getAmaVdo(FSC_U32 in);

/*  Functions that convert internal VDO representations into bits... */
/*  Converts Product VDO struct into 32 bits */
FSC_U32 getBitsForProductVdo(ProductVdo in);

/*  Converts Cert Stat VDO struct into 32 bits */
FSC_U32 getBitsForCertStatVdo(CertStatVdo in);

/*  Converts Cable VDO struct into 32 bits */
FSC_U32 getBitsForCableVdo(CableVdo in);

/*  Converts AMA VDO struct into 32 bits */
FSC_U32 getBitsForAmaVdo(AmaVdo in);

#endif /*  FSC_HAVE_VDM */
#endif /*  FSCPM_VDM_H_ */
