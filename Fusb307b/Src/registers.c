/*******************************************************************************
 * @file     registers.c
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
 * registers.c
 *
 * Implements the I2C register definitions/containers for the FUSB307.
 * ************************************************************************** */

#include "port.h"
#include "registers.h"

/*
 * Returns a ptr to the cached value of the specified register in registers.
 * Note that this does not include reserved registers.
 */
FSC_U8 *AddressToRegister(DeviceReg_t *registers, enum RegAddress address)
{
  FSC_U8 *reg = 0;

  switch (address) {
  case regVENDIDL:
    reg = &registers->VendIDL;
    break;
  case regVENDIDH:
    reg = &registers->VendIDH;
    break;
  case regPRODIDL:
    reg = &registers->ProdIDL;
    break;
  case regPRODIDH:
    reg = &registers->ProdIDH;
    break;
  case regDEVIDL:
    reg = &registers->DevIDL;
    break;
  case regDEVIDH:
    reg = &registers->DevIDH;
    break;
  case regTYPECREVL:
    reg = &registers->TypeCRevL;
    break;
  case regTYPECREVH:
    reg = &registers->TypeCRevH;
    break;
  case regUSBPDVER:
    reg = &registers->USBPDVer;
    break;
  case regUSBPDREV:
    reg = &registers->USBPDRev;
    break;
  case regPDIFREVL:
    reg = &registers->PDIFRevL;
    break;
  case regPDIFREVH:
    reg = &registers->PDIFRevH;
    break;
  case regALERTL:
    reg = &registers->AlertL.byte;
    break;
  case regALERTH:
    reg = &registers->AlertH.byte;
    break;
  case regALERTMSKL:
    reg = &registers->AlertMskL.byte;
    break;
  case regALERTMSKH:
    reg = &registers->AlertMskH.byte;
    break;
  case regPWRSTATMSK:
    reg = &registers->PwrStatMsk.byte;
    break;
  case regFAULTSTATMSK:
    reg = &registers->FaultStatMsk.byte;
    break;
  case regSTD_OUT_CFG:
    reg = &registers->StdOutCfg.byte;
    break;
  case regTCPC_CTRL:
    reg = &registers->TcpcCtrl.byte;
    break;
  case regROLECTRL:
    reg = &registers->RoleCtrl.byte;
    break;
  case regFAULTCTRL:
    reg = &registers->FaultCtrl.byte;
    break;
  case regPWRCTRL:
    reg = &registers->PwrCtrl.byte;
    break;
  case regCCSTAT:
    reg = &registers->CCStat.byte;
    break;
  case regPWRSTAT:
    reg = &registers->PwrStat.byte;
    break;
  case regFAULTSTAT:
    reg = &registers->FaultStat.byte;
    break;
  case regCOMMAND:
    reg = &registers->Command;
    break;
  case regDEVCAP1L:
    reg = &registers->DevCap1L.byte;
    break;
  case regDEVCAP1H:
    reg = &registers->DevCap1H.byte;
    break;
  case regDEVCAP2L:
    reg = &registers->DevCap2L.byte;
    break;
  case regSTD_OUT_CAP:
    reg = &registers->StdOutCap.byte;
    break;
  case regMSGHEADR:
    reg = &registers->MsgHeadr.byte;
    break;
  case regRXDETECT:
    reg = &registers->RxDetect.byte;
    break;
  case regRXBYTECNT:
    reg = &registers->RxByteCnt;
    break;
  case regRXSTAT:
    reg = &registers->RxStat.byte;
    break;
  case regRXHEADL:
    reg = &registers->RxHeadL;
    break;
  case regRXHEADH:
    reg = &registers->RxHeadH;
    break;
  case regTRANSMIT:
    reg = &registers->Transmit.byte;
    break;
  case regTXBYTECNT:
    reg = &registers->TxByteCnt;
    break;
  case regTXHEADL:
    reg = &registers->TxHeadL;
    break;
  case regTXHEADH:
    reg = &registers->TxHeadH;
    break;
  case regVBUS_VOLTAGE_L:
    reg = &registers->VBusVoltageL.byte;
    break;
  case regVBUS_VOLTAGE_H:
    reg = &registers->VBusVoltageH.byte;
    break;
  case regVBUS_SNK_DISCL:
    reg = &registers->VBusSnkDiscL.byte;
    break;
  case regVBUS_SNK_DISCH:
    reg = &registers->VBusSnkDiscH.byte;
    break;
  case regVBUS_STOP_DISCL:
    reg = &registers->VBusStopDiscL.byte;
    break;
  case regVBUS_STOP_DISCH:
    reg = &registers->VBusStopDiscH.byte;
    break;
  case regVALARMHCFGL:
    reg = &registers->VAlarmHCfgL.byte;
    break;
  case regVALARMHCFGH:
    reg = &registers->VAlarmHCfgH.byte;
    break;
  case regVALARMLCFGL:
    reg = &registers->VAlarmLCfgL.byte;
    break;
  case regVALARMLCFGH:
    reg = &registers->VAlarmLCfgH.byte;
    break;
  case regVCONN_OCP:
    reg = &registers->VConnOCP.byte;
    break;
  case regSLICE:
    reg = &registers->Slice.byte;
    break;
  case regRESET:
    reg = &registers->Reset.byte;
    break;
  case regVD_STAT:
    reg = &registers->VDStat.byte;
    break;
  case regGPIO1_CFG:
    reg = &registers->Gpio1Cfg.byte;
    break;
  case regGPIO2_CFG:
    reg = &registers->Gpio2Cfg.byte;
    break;
  case regGPIO_STAT:
    reg = &registers->GpioStat.byte;
    break;
  case regDRPTOGGLE:
    reg = &registers->DrpToggle.byte;
    break;
  case regTOGGLE_SM:
    reg = &registers->ToggleSM.byte;
    break;
  case regSINK_TRANSMIT:
    reg = &registers->SinkTransmit.byte;
    break;
  case regSRC_FRSWAP:
    reg = &registers->SrcFRSwap.byte;
    break;
  case regSNK_FRSWAP:
    reg = &registers->SnkFRSwap.byte;
    break;
  case regALERT_VD:
    reg = &registers->AlertVD.byte;
    break;
  case regALERT_VD_MSK:
    reg = &registers->AlertVDMsk.byte;
    break;
  case regRPVAL_OVERRIDE:
    reg = &registers->RpValOverride.byte;
    break;
  default:
    break;
  }
  return reg;
}

/* Populates data with contents of registers, excluding reserved registers. */
void GetLocalRegisters(DeviceReg_t *registers, FSC_U8 *data, FSC_U32 length)
{
  if (length >= TOTAL_REGISTER_CNT) {
    data[0] = registers->VendIDL;
    data[1] = registers->VendIDH;
    data[2] = registers->ProdIDL;
    data[3] = registers->ProdIDH;
    data[4] = registers->DevIDL;
    data[5] = registers->DevIDH;
    data[6] = registers->TypeCRevL;
    data[7] = registers->TypeCRevH;
    data[8] = registers->USBPDVer;
    data[9] = registers->USBPDRev;
    data[10] = registers->PDIFRevL;
    data[11] = registers->PDIFRevH;
    data[12] = registers->AlertL.byte;
    data[13] = registers->AlertH.byte;
    data[14] = registers->AlertMskL.byte;
    data[15] = registers->AlertMskH.byte;
    data[16] = registers->PwrStatMsk.byte;
    data[17] = registers->FaultStatMsk.byte;
    data[18] = registers->StdOutCfg.byte;
    data[19] = registers->TcpcCtrl.byte;
    data[20] = registers->RoleCtrl.byte;
    data[21] = registers->FaultCtrl.byte;
    data[22] = registers->PwrCtrl.byte;
    data[23] = registers->CCStat.byte;
    data[24] = registers->PwrStat.byte;
    data[25] = registers->FaultStat.byte;
    data[26] = registers->Command;
    data[27] = registers->DevCap1L.byte;
    data[28] = registers->DevCap1H.byte;
    data[29] = registers->DevCap2L.byte;
    data[31] = registers->StdOutCap.byte;
    data[32] = registers->MsgHeadr.byte;
    data[33] = registers->RxDetect.byte;
    data[34] = registers->RxByteCnt;
    data[35] = registers->RxStat.byte;
    data[36] = registers->RxHeadL;
    data[37] = registers->RxHeadH;
    data[38] = registers->RxData[0];
    data[39] = registers->RxData[1];
    data[40] = registers->RxData[2];
    data[41] = registers->RxData[3];
    data[42] = registers->RxData[4];
    data[43] = registers->RxData[5];
    data[44] = registers->RxData[6];
    data[45] = registers->RxData[7];
    data[46] = registers->RxData[8];
    data[47] = registers->RxData[9];
    data[48] = registers->RxData[10];
    data[49] = registers->RxData[11];
    data[50] = registers->RxData[12];
    data[51] = registers->RxData[13];
    data[52] = registers->RxData[14];
    data[53] = registers->RxData[15];
    data[54] = registers->RxData[16];
    data[55] = registers->RxData[17];
    data[56] = registers->RxData[18];
    data[57] = registers->RxData[19];
    data[58] = registers->RxData[20];
    data[59] = registers->RxData[21];
    data[60] = registers->RxData[22];
    data[61] = registers->RxData[23];
    data[62] = registers->RxData[24];
    data[63] = registers->RxData[25];
    data[64] = registers->RxData[26];
    data[65] = registers->RxData[27];
    data[66] = registers->Transmit.byte;
    data[67] = registers->TxByteCnt;
    data[68] = registers->TxHeadL;
    data[69] = registers->TxHeadH;
    data[70] = registers->TxData[0];
    data[71] = registers->TxData[1];
    data[72] = registers->TxData[2];
    data[73] = registers->TxData[3];
    data[74] = registers->TxData[4];
    data[75] = registers->TxData[5];
    data[76] = registers->TxData[6];
    data[77] = registers->TxData[7];
    data[78] = registers->TxData[8];
    data[79] = registers->TxData[9];
    data[80] = registers->TxData[10];
    data[81] = registers->TxData[11];
    data[82] = registers->TxData[12];
    data[83] = registers->TxData[13];
    data[84] = registers->TxData[14];
    data[85] = registers->TxData[15];
    data[86] = registers->TxData[16];
    data[87] = registers->TxData[17];
    data[88] = registers->TxData[18];
    data[89] = registers->TxData[19];
    data[90] = registers->TxData[20];
    data[91] = registers->TxData[21];
    data[92] = registers->TxData[22];
    data[93] = registers->TxData[23];
    data[94] = registers->TxData[24];
    data[95] = registers->TxData[25];
    data[96] = registers->TxData[26];
    data[97] = registers->TxData[27];
    data[98] = registers->VBusVoltageL.byte;
    data[99] = registers->VBusVoltageH.byte;
    data[100] = registers->VBusSnkDiscL.byte;
    data[101] = registers->VBusSnkDiscH.byte;
    data[102] = registers->VBusStopDiscL.byte;
    data[103] = registers->VBusStopDiscH.byte;
    data[104] = registers->VAlarmHCfgL.byte;
    data[105] = registers->VAlarmHCfgH.byte;
    data[106] = registers->VAlarmLCfgL.byte;
    data[107] = registers->VAlarmLCfgH.byte;
    data[108] = registers->VConnOCP.byte;
    data[109] = registers->Slice.byte;
    data[110] = registers->Reset.byte;
    data[111] = registers->VDStat.byte;
    data[112] = registers->Gpio1Cfg.byte;
    data[113] = registers->Gpio2Cfg.byte;
    data[114] = registers->GpioStat.byte;
    data[115] = registers->DrpToggle.byte;
    data[116] = registers->ToggleSM.byte;
    data[117] = registers->SinkTransmit.byte;
    data[118] = registers->SrcFRSwap.byte;
    data[119] = registers->SnkFRSwap.byte;
    data[120] = registers->AlertVD.byte;
    data[121] = registers->AlertVDMsk.byte;
    data[122] = registers->RpValOverride.byte;
  }
}

void RegGetRxData(DeviceReg_t *registers, FSC_U8 *data, FSC_U32 length)
{
  FSC_U8 i = 0;
  for (i = 0; i < length && i < COMM_BUFFER_LENGTH; ++i) {
    data[i] = registers->RxData[i];
  }
}

void RegSetTxData(DeviceReg_t *registers, FSC_U8 *data, FSC_U32 length)
{
  FSC_U8 i = 0;
  for (i = 0; i < length && i < COMM_BUFFER_LENGTH; ++i) {
    registers->TxData[i] = data[i];
  }
}

void RegSetBits(DeviceReg_t *registers, enum RegAddress address, FSC_U8 mask)
{
  FSC_U8 *reg = AddressToRegister(registers, address);
  *reg |= mask;
}

void RegClearBits(DeviceReg_t *registers, enum RegAddress address, FSC_U8 mask)
{
  FSC_U8 *reg = AddressToRegister(registers, address);
  *reg &= ~mask;
}













