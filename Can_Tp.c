/**======================================================================================================*\
  @file CanTp.c

  @brief Can Transport Layer
  
    Can Transport Layer Implementation
\*======================================================================================================*/

/*======================================================================================================*\
  Headers
\*======================================================================================================*/
#include "Can_Tp.h"
#include "PduR.h"
#include "CAN_IF.h"
/*=======================================================================================================*\
  Global variables
\*=======================================================================================================*/
CanTp_ConfigType CanTp_ConfigPtr;
CanTp_ConfigRxTxType CanTp_RxTxVariablesConfig;
CanTp_StateType eCanTp_State;
CanTp_TimerType CanTp_ArTimer = {0, 0};
CanTp_TimerType CanTp_BrTimer = {0, 0}; 
CanTp_TimerType CanTp_CrTimer = {0, 0}; 
CanTp_TimerType CanTp_AsTimer = {0, 0}; 
CanTp_TimerType CanTp_BsTimer = {0, 0}; 
CanTp_TimerType CanTp_CsTimer = {0, 0};
uint32 CanTp_FCWFTcnt;
/*=======================================================================================================*\
  Timer functions
\*=======================================================================================================*/
void CanTp_ResetTimer(CanTp_TimerType *CanTp_Timer) {
    CanTp_Timer->eCanTp_TimerState = STOP;
    CanTp_Timer->CanTp_Counter = 0;
}
void CanTp_IncrementTimer(CanTp_TimerType *CanTp_Timer) {  
    if(CanTp_Timer->eCanTp_TimerState == START){
      CanTp_Timer->CanTp_Counter++;
    }
}
/*=======================================================================================================*\
  Definitions of functions
\*=======================================================================================================*/
/*=======================================================================================================*/
/**
  @brief CanTp_Init

 This function initializes the CanTp module. [SWS_CANTP_00208]
*/
/*=======================================================================================================*/
void CanTp_Init(const CanTp_ConfigType* CfgPtr) {
    CanTp_ConfigPtr = *CfgPtr;
    CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
    CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
    eCanTp_State = CANTP_ON;
}
/*========================================================================================================*/
/**
  @brief CanTp_GetVersionInfo

 This function returns the version information of the CanTp module [SWS_CANTP_00210]
*/
/*========================================================================================================*/
void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    //if(versioninfo != NULL_PTR) {
        versioninfo->moduleID = (uint16)CANTP_MODULE_ID;
        versioninfo->vendorID = (uint16)CANTP_VENDOR_ID;
        versioninfo->sw_major_version = CANTP_SW_MAJOR_VER;
        versioninfo->sw_minor_version = CANTP_SW_MINOR_VER;
        versioninfo->sw_patch_version = CANTP_SW_PATCH_VER;
    //}
    //else {
        //printf("CANTP_E_PARAM_POINTER error - versioninfo is NULL pointer");
    //}
}
/*========================================================================================================*/
/**
  @brief CanTp_Shutdown

  This function is called to shutdown the CanTp module. [SWS_CANTP_00211], [SWS_CANTP_00202], [SWS_CANTP_00200]
*/
/*========================================================================================================*/
void CanTp_Shutdown(void) {
    eCanTp_State = CANTP_OFF;
}
/*========================================================================================================*/
/**
  @brief CanTp_Transmit

  This function requests transmission of a PDU. 
   [SWS_CanTp_00212], [SWS_CANTP_00231],
   [SWS_CANTP_00232], [SWS_CANTP_00204],
   [SWS_CANTP_00205], [SWS_CANTP_00206],
   [SWS_CANTP_00298], [SWS_CANTP_00299],
   [SWS_CANTP_00321], [SWS_CANTP_00354]
*/
/*========================================================================================================*/
Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr) {
  Std_ReturnType ret = E_OK;
  BufReq_ReturnType BufState;
  PduLengthType availableDataPtr;
  PduInfoType PduInfoTmp;
  uint8 SduTable[8];
  PduInfoTmp.SduDataPtr = SduTable;
  #ifdef DEBUG
  printf("\nSduLength: 0x%04x\n",PduInfoPtr->SduLength);
  #endif

  if(PduInfoPtr->SduDataPtr == NULL) {
    ret = E_NOT_OK;
  }
  else {
    if(eCanTp_State == CANTP_ON) {
      if(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_WAIT) {
        if(PduInfoPtr->SduLength < 8) {
          BufState = PduR_CanTpCopyTxData(TxPduId, &PduInfoTmp, NULL, &availableDataPtr);
          if(BufState == BUFREQ_OK) {
            PduInfoTmp.SduDataPtr[0] = CANTP_N_PCI_SF << 4;
            PduInfoTmp.SduDataPtr[0] = 0x0F & PduInfoPtr->SduLength; 
            for(uint8 i = 0; i < PduInfoPtr->SduLength; i++) {
              PduInfoTmp.SduDataPtr[i+1] = PduInfoPtr->SduDataPtr[i];
            }
            #ifdef DEBUG
            for(uint8 i = 0; i < 8; i++) {
              printf("SingleFrame[%d]: 0x%02x\n", i, PduInfoTmp.SduDataPtr[i]);
            }
            #endif
            if(CanIf_Transmit(TxPduId, &PduInfoTmp) == E_OK) {
              #ifdef DEBUG
              printf("Timer N_AS start\n");
              #endif
              PduR_CanTpTxConfirmation(TxPduId, E_OK);
            }
            else {
              PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
              ret = E_NOT_OK;
            }  
          }
          else if(BufState == BUFREQ_E_NOT_OK) {
            CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
            PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
            ret = E_NOT_OK;
          }
          else {
            #ifdef DEBUG
            printf("Timer N_CS start\n");
            #endif
            ret = E_OK;
          }
        }
        else {
          PduInfoTmp.SduDataPtr[0] = CANTP_N_PCI_FF << 4;
          if(PduInfoPtr->SduLength <= 4095){
              PduInfoTmp.SduDataPtr[0] |= (0x0F & (PduInfoPtr->SduLength >> 8));
              PduInfoTmp.SduDataPtr[1] = (0xFF & (PduInfoPtr->SduLength));
              PduInfoTmp.SduDataPtr[2] = PduInfoPtr->SduDataPtr[0];
              PduInfoTmp.SduDataPtr[3] = PduInfoPtr->SduDataPtr[1];
              PduInfoTmp.SduDataPtr[4] = PduInfoPtr->SduDataPtr[2];
              PduInfoTmp.SduDataPtr[5] = PduInfoPtr->SduDataPtr[3];
              PduInfoTmp.SduDataPtr[6] = PduInfoPtr->SduDataPtr[4];
              PduInfoTmp.SduDataPtr[7] = PduInfoPtr->SduDataPtr[5];
          }
          else{
              PduInfoTmp.SduDataPtr[1] = 0;
              PduInfoTmp.SduDataPtr[2] = (PduInfoPtr->SduLength >> 24) & 0xFF;
              PduInfoTmp.SduDataPtr[3] = (PduInfoPtr->SduLength >> 16) & 0xFF;
              PduInfoTmp.SduDataPtr[4] = (PduInfoPtr->SduLength >> 8) & 0xFF;
              PduInfoTmp.SduDataPtr[5] = (PduInfoPtr->SduLength >> 0) & 0xFF;
              PduInfoTmp.SduDataPtr[6] = PduInfoPtr->SduDataPtr[0];
              PduInfoTmp.SduDataPtr[7] = PduInfoPtr->SduDataPtr[1];
          }
          #ifdef DEBUG
          for(uint8 i = 0; i < 8; i++) {
              printf("FirstFrame[%d]: 0x%02x\n", i, PduInfoTmp.SduDataPtr[i]);
          }
          #endif
          if(CanIf_Transmit(TxPduId, &PduInfoTmp) == E_OK) {
              #ifdef DEBUG
              printf("Timer N_AS start\n");
              printf("Timer N_BS start\n");
              #endif
              PduR_CanTpTxConfirmation(TxPduId, E_OK);
              CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING_SUSPEND;
          }
          else {
              PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
              ret = E_NOT_OK;
          }
        }
      }
      else {
        ret = E_NOT_OK;
      }
    }
    else {
      ret = E_NOT_OK;
    }
  }
  return ret;
}
/**
  @brief CanTp_CancelTransmit
   This function requests cancellation of an ongoing transmission of a PDU in a lower layer
   communication module.
   [SWS_CANTP_00246], [SWS_CANTP_00254], [SWS_CANTP_00255], [SWS_CANTP_00256]
*/
Std_ReturnType CanTp_CancelTransmit(PduIdType TxPduId) {
  Std_ReturnType ret = E_OK;

  if(CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId == TxPduId) {
    PduR_CanTpTxConfirmation(CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId, E_NOT_OK);
    CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING_SUSPEND;
  }
  else {
    ret = E_NOT_OK;
  }
  return ret;
}
/**
  @brief CanTp_CancelReceive
   This function requests cancellation of an ongoing reception of a PDU in a lower layer
   protocol module.
   [SWS_CANTP_00257], [SWS_CANTP_00260], [SWS_CANTP_00261], [SWS_CANTP_00262], [SWS_CANTP_00263]
*/
Std_ReturnType CanTp_CancelReceive(PduIdType RxPduId) {
  Std_ReturnType ret = E_OK;

  if(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId == RxPduId) {
    PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
    CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING_SUSPEND;
  }
  else {
    ret = E_NOT_OK;
  }
  return ret;
}
/**
  @brief CanTp_ChangeParameter
   This function requests to change a specific transport protocol parameter (e.g. block size).
   [SWS_CANTP_00302], [SWS_CANTP_00303], [SWS_CANTP_00304], [SWS_CANTP_00305], [SWS_CANTP_00338]
*/
Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value) {
  Std_ReturnType ret = E_OK;

  if(eCanTp_State == CANTP_ON && CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId == id 
      && CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState != CANTP_RX_PROCESSING) {
    switch (parameter) {
      case TP_STMIN:
        if((value >= 0x00U && value <= 0x7FU) || (value >= 0xF1U && value <= 0xF9U)) {
          CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = value;
        }
        else {
          ret = E_NOT_OK;
        }
        break;
      case TP_BS:
        if(value <= 0x0FF) {
          CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs = value;
        }
        else {
          ret = E_NOT_OK;
        }
        break;
      default:
        ret = E_NOT_OK;
        break;
    }   
  }
  else {
    ret = E_NOT_OK;
  }
  return ret;
}
/**
  @brief CanTp_ReadParameter
   This function  is used to read the current value of reception parameters BS and STmin for a specified N-SDU.
   [SWS_CANTP_00323], [SWS_CANTP_00324]
*/
Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16* value) {
  Std_ReturnType ret = E_OK;
  uint16 obtainedValue;

  if(eCanTp_State == CANTP_ON && CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId == id) {
    switch (parameter) {
      case TP_STMIN:
        obtainedValue = CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin;
        if((obtainedValue >= 0x00U && obtainedValue <= 0x7FU) || (obtainedValue >= 0xF1U && obtainedValue <= 0xF9U)) {
          *value = obtainedValue;
        }
        else {
          ret = E_NOT_OK;
        }
        break;
      case TP_BS:
        obtainedValue = CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs;
        *value = obtainedValue;
        break;
      default:
        ret = E_NOT_OK;
        break;
    }   
  }
  else {
    ret = E_NOT_OK;
  }
  return ret;
}
/**
  @brief CanTp_MainFunction
   The main function for scheduling the CAN TP. [SWS_CANTP_00213], [SWS_CANTP_00164], [SWS_CANTP_00300]
*/
void CanTp_MainFunction(void) {
  uint16 CalcBlockSize;
  uint16 SepTime;
  Std_ReturnType ret;
  BufReq_ReturnType BufReqState;
  PduInfoType PduInfoNull = {NULL, NULL, 0};
  PduInfoType PduInfoPtr;
  PduLengthType PduLength;
  TPParameterType parameter1 = TP_BS;
  TPParameterType parameter2 = TP_STMIN;
  CanTp_FCFlowStatusType FlowStatus;

  CanTp_ReadParameter(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, TP_STMIN, &SepTime);

  CanTp_IncrementTimer(&CanTp_AsTimer);
  CanTp_IncrementTimer(&CanTp_BsTimer);
  CanTp_IncrementTimer(&CanTp_CsTimer);
  CanTp_IncrementTimer(&CanTp_ArTimer);
  CanTp_IncrementTimer(&CanTp_BrTimer);
  CanTp_IncrementTimer(&CanTp_CrTimer);

  if(CanTp_BrTimer.eCanTp_TimerState == START) {
    BufReqState = PduR_CanTpCopyRxData(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, &PduInfoNull, &PduLength);
    if(BufReqState == BUFREQ_E_NOT_OK) {
      PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
    }
    else {
      CalcBlockSize = CanTp_CalcBlockSize(PduLength);
      if(CalcBlockSize > 0) {
        FlowStatus = FC_CTS;
        CanTp_ChangeParameter(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, parameter1, CalcBlockSize);
        CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING;
        PduInfoPtr.SduDataPtr[0] = CANTP_N_PCI_FC << 4;
        PduInfoPtr.SduDataPtr[0] |= (0x0F & FlowStatus);
        PduInfoPtr.SduDataPtr[1] = CalcBlockSize;
        PduInfoPtr.SduDataPtr[2] = SepTime;
        ret = CanIf_Transmit(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, &PduInfoPtr);
        if(ret == E_NOT_OK) {
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING_SUSPEND;
          PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
        }
        else {
          CanTp_ResetTimer(&CanTp_BrTimer);
          CanTp_ArTimer.eCanTp_TimerState = START;
          CanTp_CrTimer.eCanTp_TimerState = START;
        }
      }
      if(CanTp_BrTimer.CanTp_Counter >= N_BR_TIMEOUT_VALUE) {
        CanTp_FCWFTcnt++;
        CanTp_BrTimer.CanTp_Counter = 0;
        if(CanTp_FCWFTcnt >= CANTP_FCWFTMAX) {
          PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
          CanTp_FCWFTcnt = 0;
        }
        else {
          FlowStatus = FC_WAIT;
          PduInfoPtr.SduDataPtr[0] = CANTP_N_PCI_FC << 4;
          PduInfoPtr.SduDataPtr[0] |= (0x0F & FlowStatus);
          PduInfoPtr.SduDataPtr[1] = CalcBlockSize;
          PduInfoPtr.SduDataPtr[2] = SepTime;
          ret = CanIf_Transmit(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, &PduInfoPtr);
          if(ret == E_NOT_OK) {
            CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING_SUSPEND;
            PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          }
          else {
            CanTp_ArTimer.eCanTp_TimerState = START;
            CanTp_BrTimer.eCanTp_TimerState = START;
          }
        }
      }
    }
  }
  if(CanTp_CrTimer.eCanTp_TimerState == START) {
    if(CanTp_CrTimer.CanTp_Counter >= N_CR_TIMEOUT_VALUE) {
      PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
      CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING_SUSPEND;
    }
  }
  if(CanTp_ArTimer.eCanTp_TimerState == START) {
    if(CanTp_ArTimer.CanTp_Counter >= N_AR_TIMEOUT_VALUE) {
      PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
      CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING_SUSPEND;
    }
  }
  if(CanTp_CsTimer.eCanTp_TimerState == START) {
    if(CanTp_CsTimer.CanTp_Counter >= N_CS_TIMEOUT_VALUE) {
      PduR_CanTpTxConfirmation(CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId, E_NOT_OK);
      CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING_SUSPEND;
    }
  }
  if(CanTp_AsTimer.eCanTp_TimerState == START) {
    if(CanTp_AsTimer.CanTp_Counter >= N_AS_TIMEOUT_VALUE) {
      PduR_CanTpTxConfirmation(CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId, E_NOT_OK);
      CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING_SUSPEND;
    }
  }
  if(CanTp_BsTimer.eCanTp_TimerState == START) {
    if(CanTp_BsTimer.CanTp_Counter >= N_BS_TIMEOUT_VALUE) {
      PduR_CanTpTxConfirmation(CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId, E_NOT_OK);
      CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING_SUSPEND;
    }
  }

}
/**
  @brief CanTp_RxIndication
   Indication of a received PDU from a lower layer communication interface module. 
   [SWS_CANTP_00214], [SWS_CANTP_00235], [SWS_CANTP_00322]
*/
void CanTp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr) {
  PduInfoType PduInfoTmp;
  Std_ReturnType ret;
  uint8 SduTable[8];
  PduLengthType BuffSize;
  BufReq_ReturnType BuffState;
  PduInfoTmp.SduDataPtr = SduTable;
  CanTp_PCIType CanPCI;

  if(eCanTp_State == CANTP_ON) {
    switch (PduInfoPtr->SduDataPtr[0] >> 4)
    {
      case CANTP_N_PCI_SF:
        CanPCI.eFrameType = SF;
        CanPCI.uiFrameLength = PduInfoPtr->SduDataPtr[0];
        break;
      case CANTP_N_PCI_FF:
        CanPCI.eFrameType = FF;
        if( (PduInfoPtr->SduDataPtr[0] & 0x0F) | PduInfoPtr->SduDataPtr[1] ) {
          CanPCI.uiFrameLength =  PduInfoPtr->SduDataPtr[0] & 0x0F;
          CanPCI.uiFrameLength =  (CanPCI.uiFrameLength << 8) | PduInfoPtr->SduDataPtr[1]; 
        }
        else{
          CanPCI.uiFrameLength =  PduInfoPtr->SduDataPtr[2];
          CanPCI.uiFrameLength =  (CanPCI.uiFrameLength << 8) | PduInfoPtr->SduDataPtr[3]; 
          CanPCI.uiFrameLength =  (CanPCI.uiFrameLength << 8) | PduInfoPtr->SduDataPtr[4];
          CanPCI.uiFrameLength =  (CanPCI.uiFrameLength << 8) | PduInfoPtr->SduDataPtr[5];
        }
        break;
      case CANTP_N_PCI_FC:
        CanPCI.eFrameType = FC;
        CanPCI.uiFlowStatus = PduInfoPtr->SduDataPtr[0] & 0x0F; 
        CanPCI.uiBlockSize = PduInfoPtr->SduDataPtr[1]; 
        CanPCI.uiSeparationTime = PduInfoPtr->SduDataPtr[2];
        break;
      case CANTP_N_PCI_CF:
        CanPCI.eFrameType = CF;
        CanPCI.uiSequenceNumber= PduInfoPtr->SduDataPtr[0] & 0x0F;
        break;
      default:
        CanPCI.eFrameType = 4;
        break;
    }
    switch (CanPCI.eFrameType)
    {
      case SF:
        if(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT) {
          BuffState = PduR_CanTpStartOfReception(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, CanPCI.uiFrameLength, &BuffSize);
          if(BuffState == BUFREQ_OK) {
            if(BuffSize >= CanPCI.uiFrameLength) {
              PduInfoTmp.SduLength = CanPCI.uiFrameLength;
              PduInfoTmp.SduDataPtr = PduInfoPtr->SduDataPtr+1;
              BuffState = PduR_CanTpCopyRxData(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, &PduInfoTmp, &BuffSize);
              PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_OK);
            }
            else {
              PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
            }
          }
        }
        else if(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING) {
          PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
          BuffState = PduR_CanTpStartOfReception(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, CanPCI.uiFrameLength, &BuffSize);
          if(BuffState == BUFREQ_OK) {
            if(BuffSize >= CanPCI.uiFrameLength) {
              PduInfoTmp.SduLength = CanPCI.uiFrameLength;
              PduInfoTmp.SduDataPtr = PduInfoPtr->SduDataPtr+1;
              BuffState = PduR_CanTpCopyRxData(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, &PduInfoTmp, &BuffSize);
              PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_OK);
            }
            else {
              PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
            }
          }
        }
        else {
          PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
          BuffState = PduR_CanTpStartOfReception(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, CanPCI.uiFrameLength, &BuffSize);
          if(BuffState == BUFREQ_OK) {
            if(BuffSize >= CanPCI.uiFrameLength) {
              PduInfoTmp.SduLength = CanPCI.uiFrameLength;
              PduInfoTmp.SduDataPtr = PduInfoPtr->SduDataPtr+1;
              BuffState = PduR_CanTpCopyRxData(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, &PduInfoTmp, &BuffSize);
              PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_OK);
            }
            else {
              PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
            }
          }
        }
        break;
      case FF:
        if(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT) {
          BuffState = PduR_CanTpStartOfReception(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, CanPCI.uiFrameLength, &BuffSize);
          ret = CanTp_ReceiveFF(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, &CanPCI, BuffState, FC_CTS);
        }
        else if(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING) {
          PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
          ret = CanTp_ReceiveFF(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, &CanPCI, BuffState, FC_WAIT);
        }
        else {
          PduR_CanTpRxIndication(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
          ret = CanTp_ReceiveFF(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, &CanPCI, BuffState, FC_WAIT);
        }
        break;
      case CF:
        if(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING) {
          ret = CanTp_ReceiveCF(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, &CanPCI, FC_CTS);
        }
        else {
          PduR_CanTpRxIndication (CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, E_NOT_OK);
          CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
        }
        break;
      case FC:
        ret = CanTp_ReceiveFC(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId, PduInfoPtr, &CanPCI, FC_CTS);
        break;
      default:
        #ifdef DEBUG
        printf("\nERROR 404 :)\n");
        #endif
        break;
    }
  }



}