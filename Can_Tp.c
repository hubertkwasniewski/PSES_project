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
    if(versioninfo != NULL_PTR) {
        versioninfo->moduleID = (uint16)CANTP_MODULE_ID;
        versioninfo->vendorID = (uint16)CANTP_VENDOR_ID;
        versioninfo->sw_major_version = CANTP_SW_MAJOR_VER;
        versioninfo->sw_minor_version = CANTP_SW_MINOR_VER;
        versioninfo->sw_patch_version = CANTP_SW_PATCH_VER;
    }
    else {
        printf("CANTP_E_PARAM_POINTER error - versioninfo is NULL pointer");
    }
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

  if(PduInfoPtr->MetaDataPtr == NULL || PduInfoPtr->SduDataPtr == NULL) {
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
          CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs = value;
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
