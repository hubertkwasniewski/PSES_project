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
  #define CANTP_FRAME_SIZE (0x08U)

  #define N_AS_TIMEOUT_VALUE 1000
  #define N_AR_TIMEOUT_VALUE 1000
  #define N_BS_TIMEOUT_VALUE 1000
  #define N_BR_TIMEOUT_VALUE 1000
  #define N_CS_TIMEOUT_VALUE 1000
  #define N_CR_TIMEOUT_VALUE 1000
  #define STMIN_TIMEOUT_VALUE 1000

  #define CANTP_N_PCI_SF (0x00U)
  #define CANTP_N_PCI_FF (0x01U)
  #define CANTP_N_PCI_CF (0x02U)
  #define CANTP_N_PCI_FC (0x03U)

  #define CANTP_FCWFTMAX (10)
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

  typedef enum {
    SF = 0,
    FF,
    CF,
    FC
  } CanTp_FrameType;

  typedef enum {
    FC_CTS = 0,
    FC_WAIT,
    FC_OVFLW
  } CanTp_FCFlowStatusType;

  typedef struct {
    CanTp_FrameType eFrameType;
    PduLengthType uiFrameLength;
    uint8 uiSequenceNumber;
    uint8 uiBlockSize;
    uint8 uiFlowStatus;
    uint8 uiSeparationTime;
  } CanTp_PCIType;

  typedef enum {
    CANTP_EXTENDED,
    CANTP_MIXED,
    CANTP_MIXED29BIT,
    CANTP_NORMALFIXED,
    CANTP_STANDARD
  } CanTp_AddressingFormatType;

  typedef enum {
     CANTP_OFF_PADDING,
     CANTP_ON_PADDING
  } CanTp_PaddingActivationType;
  
  typedef enum {
    CANTP_FUNCTIONAL,
    CANTP_PHYSICAL
  } CanTp_TaType;

  typedef struct {
    CanTp_RxStateType eCanTp_RxState;
    PduIdType CanTp_CurrentRxPduId;
  } CanTp_RxConfigType;

  typedef struct {
    CanTp_TxStateType eCanTp_TxState;
    PduIdType CanTp_CurrentTxPduId;
  } CanTp_TxConfigType;

  typedef struct{
    boolean CanTpChangeParameterApi;
    boolean CanTpDevErrorDetect;
    boolean CanTpDynIdSupport;
    boolean CanTpFlexibleDataRateSupport;
    boolean CanTpGenericConnectionSupport;
    uint8 CanTpPaddingByte;
    boolean CanTpReadParameterApi;
    boolean CanTpVersionInfoApi;
  } CanTp_GeneralType;

  typedef struct {
    uint16 CanTpRxNPduId;
    PduIdType CanTpRxNPduRef;
  } CanTp_RxNPduType;

  typedef struct {
    uint16 CanTpTxNPduConfirmationPduId;
    PduIdType CanTpTxNPduRef;
  } CanTp_TxNPduType;

  typedef struct {
    uint16 CanTpTxFcNPduConfirmationPduId;
    PduIdType CanTpTxFcNPduRef;
  } CanTp_TxFcNPduType;

  typedef struct {
    uint16 CanTpRxFcNPduId;
    PduIdType CanTpRxFcNPduRef;
  } CanTp_RxFcNPduType;

  typedef struct {
    uint8 CanTpNTa;
  } CanTp_NTaType;

  typedef struct {
    uint8 CanTpNSa;
  } CanTp_NSaType;

  typedef struct {
    uint8 CanTpNAe;
  } CanTp_NAeType;

  typedef struct {
    float32 CanTpNas;
    float32 CanTpNbs;
    float32 CanTpNcs;
    boolean CanTpTc;
    CanTp_AddressingFormatType eCanTpTxAddressingFormat;
    uint16 CanTpTxNSduId;
    CanTp_PaddingActivationType eCanTpTxPaddingActivation;
    CanTp_TaType eCanTpTxTaType;
    PduIdType CanTpTxNSduRef;
    CanTp_NAeType CanTpNAe;
    CanTp_NSaType CanTpNSa;
    CanTp_NTaType CanTpNTa;
    CanTp_RxFcNPduType CanTpRxFcNPdu;
    CanTp_TxNPduType CanTpTxNPdu;
  } CanTp_TxNSduType;

  typedef struct{
    uint8 CanTpBs;
    float32 CanTpNar;
    float32 CanTpNbr;
    float32 CanTpNcr;
    CanTp_AddressingFormatType eCanTpRxAddressingFormat;
    uint16 CanTpRxNSduId;
    CanTp_PaddingActivationType eCanTpRxPaddingActivation;
    CanTp_TaType eCanTpRxTaType;
    uint16 CanTpRxWftMax;
    float32 CanTpSTmin;
    PduIdType CanTpRxNSduRef;
    CanTp_NAeType CanTpNAe;
    CanTp_NSaType CanTpNSa;
    CanTp_NTaType CanTpNTa;
    CanTp_RxNPduType CanTpRxNPdu;
    CanTp_TxFcNPduType CanTpTxFcNPdu;
  } CanTp_RxNSduType;

  typedef struct{
    CanTp_RxNSduType RxNSdu;
    CanTp_TxNSduType TxNSdu;
  } CanTp_ChannelType;

  typedef struct{
    float32 CanTpMainFunctionPeriod;
    uint64 CanTpMaxChannelCnt;
    CanTp_ChannelType CanTpChannel;
  } CanTp_ConfigType;

  typedef struct {
    CanTp_RxConfigType CanTp_RxConfig;
    CanTp_TxConfigType CanTp_TxConfig;
  } CanTp_ConfigRxTxType;
/*====================================================================================================================*\
  Timers
\*====================================================================================================================*/
  typedef enum {
    STOP,
    START
  } CanTp_TimerState;
  
  typedef struct {
    CanTp_TimerState eCanTp_TimerState;
    uint32 CanTp_Counter;
  } CanTp_TimerType;
/*====================================================================================================================*\
  Declarations of functions
\*====================================================================================================================*/
	void CanTp_Init(const CanTp_ConfigType* CfgPtr);
  void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo);
  void CanTp_Shutdown(void);

  Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);
  Std_ReturnType CanTp_CancelTransmit(PduIdType TxPduId);
  Std_ReturnType CanTp_CancelReceive(PduIdType RxPduId);
  Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value);
  Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16* value);
  void CanTp_MainFunction(void);
  void CanTp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
  void CanTp_TxConfirmation(PduIdType TxPduId, Std_ReturnType result);

  void CanTp_ResetTimer(CanTp_TimerType *CanTp_Timer);
  void CanTp_IncrementTimer(CanTp_TimerType *CanTp_Timer);
  uint16 CanTp_CalcBlockSize(PduLengthType BufferSize);

#endif /* CAN_TP_H */