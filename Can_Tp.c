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
CanTp_ConfigType ConfigPtr;
CanTp_ConfigRxTxType RxTxVariablesConfig;
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
    ConfigPtr = *CfgPtr;
    RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
    RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
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

