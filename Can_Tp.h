#ifndef CAN_TP_H
#define CAN_TP_H

/*================================================================================================================================================*\
  @file CanTp.h

  @brief Can Transport Layer
  
  CAN Transport Layer Implementation

  @see CAN Transport Layer specification (https://www.autosar.org/fileadmin/standards/R18-10_R4.4.0_R1.5.0/CP/AUTOSAR_SWS_CANTransportLayer.pdf)
\*================================================================================================================================================*/

/*====================================================================================================================*\
  Headers
\*====================================================================================================================*/
	#include "CommunicationStackTypes.h"
/*====================================================================================================================*\
  Global macros
\*====================================================================================================================*/
  #define NULL_PTR ((void *)0x00U) // Wypełnia: [SWS_CanTp_00319]
  
  #define CANTP_MODULE_ID (0x0EU)
  #define CANTP_SW_MAJOR_VER (0x01U)
  #define CANTP_SW_MINOR_VER (0x02U)
  #define CANTP_SW_PATCH_VER (0x03U)
  #define CANTP_VENDOR_ID (0x04U)
/*====================================================================================================================*\
  Typedefs
\*====================================================================================================================*/
	typedef enum {
    CANTP_OFF,
    CANTP_ON
  } CanTp_StateType; // Wypełnia: [SWS_CanTp_00027], [SWS_CanTp_00168]

  typedef enum {
    CANTP_RX_WAIT,
    CANTP_RX_PROCESSING,
    CANTP_RX_PROCESSING_SUSPEND
  } CanTp_RxStateType;

  typedef enum {
    CANTP_TX_WAIT,
    CANTP_TX_PROCESSING,
    CANTP_TX_PROCESSING_SUSPEND
  } CanTp_TxStateType;

  typedef struct {
    CanTp_RxStateType eCanTp_RxState;
  } CanTp_RxConfigType;

  typedef struct {
    CanTp_TxStateType eCanTp_TxState;
  } CanTp_TxConfigType;
  
  typedef struct {
    CanTp_RxConfigType CanTp_RxConfig;
    CanTp_TxConfigType CanTp_TxConfig;
  } CanTp_ConfigType;
/*====================================================================================================================*\
  Declarations of functions
\*====================================================================================================================*/
	void CanTp_Init(const CanTp_ConfigType* CfgPtr);
  void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo);
  void CanTp_Shutdown(void);

#endif /* CAN_TP_H */