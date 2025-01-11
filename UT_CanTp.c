/** ==================================================================================================================*\
  @file UT_Can_TP.c

  @brief Unit tests for CAN Transport Layer
\*====================================================================================================================*/
/** ==================================================================================================================*\
	DEBUG ON/OFF - comment line with #define to turn debug off
\*====================================================================================================================*/
//#define DEBUG
//---------------------------------------------------------------------------------------------------------------------/
#include "acutest.h"
#include "Std_Types.h"

#include "Can_Tp.c"   

#include <stdio.h>
#include <string.h>

#include "fff.h"

DEFINE_FFF_GLOBALS;

/** ==================================================================================================================*\
	Fake functions
\*====================================================================================================================*/
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyTxData, PduIdType, const PduInfoType*, const RetryInfoType*, PduLengthType*);
FAKE_VOID_FUNC(PduR_CanTpTxConfirmation, PduIdType, Std_ReturnType);
FAKE_VOID_FUNC(PduR_CanTpRxIndication, PduIdType, Std_ReturnType);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyRxData, PduIdType, const PduInfoType*, PduLengthType*);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpStartOfReception, PduIdType, const PduInfoType*, PduLengthType, PduLengthType*);

FAKE_VALUE_FUNC(uint16, CanTp_CalcBlockSize, PduLengthType);
FAKE_VALUE_FUNC(Std_ReturnType, CanIf_Transmit, PduIdType, const PduInfoType*);
//Only to reset all fakes
#define FFF_FAKES_LIST(FAKE) \
		FAKE(PduR_CanTpCopyTxData) \
		FAKE(PduR_CanTpTxConfirmation) \
		FAKE(PduR_CanTpRxIndication) \
		FAKE(PduR_CanTpCopyRxData) \
		FAKE(PduR_CanTpStartOfReception) \
		FAKE(CanIf_Transmit) \
		FAKE(CanTp_CalcBlockSize)
/** ==================================================================================================================*\
	Custom fakes
\*====================================================================================================================*/

/** ==================================================================================================================*\
	Unit Tests
\*====================================================================================================================*/
/**
  @brief Test of CanTp_Init function

  This function tests CanTp_Init function.
*/
void TestOf_CanTp_Init(void){
	CanTp_ConfigType CanTp_ConfigTest = {100.0, 2};
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING;
	eCanTp_State = CANTP_OFF;
	
	CanTp_Init(&CanTp_ConfigTest);
	
	TEST_CHECK(CanTp_ConfigPtr.CanTpMainFunctionPeriod == 100.0);
	TEST_CHECK(CanTp_ConfigPtr.CanTpMaxChannelCnt == 2);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_WAIT);
	TEST_CHECK(eCanTp_State == CANTP_ON);
}
/**
  @brief Test of CanTp_GetVersionInfo function

  This function tests CanTp_GetVersionInfo function.
*/
void TestOf_CanTp_GetVersionInfo(void){
	Std_VersionInfoType version = {0xFFFFU, 0xFFFFU, 0xFFU, 0xFFU, 0xFFU};
	
	TEST_CHECK(version.vendorID == 0xFFFFU);
	TEST_CHECK(version.moduleID == 0xFFFFU);
	TEST_CHECK(version.sw_major_version == 0xFFU);
	TEST_CHECK(version.sw_minor_version == 0xFFU);
	TEST_CHECK(version.sw_patch_version == 0xFFU);
	
	CanTp_GetVersionInfo(&version);
	
	TEST_CHECK(version.vendorID == 0x0004U);
	TEST_CHECK(version.moduleID == 0x000EU);
	TEST_CHECK(version.sw_major_version == 0x01U);
	TEST_CHECK(version.sw_minor_version == 0x02U);
	TEST_CHECK(version.sw_patch_version == 0x03U);
}
/**
  @brief Test of CanTp_Shutdown function

  This function tests CanTp_Shutdown function.
*/
void TestOf_CanTp_Shutdown(void){
	eCanTp_State = CANTP_ON;
	
	CanTp_Shutdown();
	
	TEST_CHECK(eCanTp_State == CANTP_OFF);
}
/**
  @brief Test of CanTp_Transmit function

  This function tests CanTp_Transmit function.
*/
void TestOf_CanTp_Transmit(void){
	PduIdType PduId = 0x02;
	PduInfoType PduInfo;
	Std_ReturnType ret;
//=====================================================================================================================
//	Test 1 - PduInfoPtr == NULL_PTR --> ret = E_NOT_OK
//=====================================================================================================================
	PduInfo.SduDataPtr = NULL;
	PduInfo.MetaDataPtr = NULL;
	PduInfo.SduLength = 0x00;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 0);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 2 - CANTP_OFF --> ret = E_NOT_OK
//=====================================================================================================================
	uint8 SduData[8];
	uint8 *MetaData;
	PduInfo.SduDataPtr = SduData;
	PduInfo.MetaDataPtr = MetaData;
	PduInfo.SduLength = 4294967295;
	for(uint8 i = 0; i < 8; i++) {
		SduData[i] = i + 1;
	}
	
	eCanTp_State = CANTP_OFF;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_OK;
	CanIf_Transmit_fake.return_val = E_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(eCanTp_State == CANTP_OFF);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 0);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 3 - CANTP_ON --> Sending single frame (SduLength < 8), ret = E_OK, BufState = BUFREQ_OK
//=====================================================================================================================
	uint8 ObtainedFrame[8] = {0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	PduInfo.SduLength = 7;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_OK;
	CanIf_Transmit_fake.return_val = E_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
	for(uint8 i = 0; i < 8; i++){TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[i] == ObtainedFrame[i]);}
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 4 - CANTP_ON --> Sending first frame (SduLength <= 4095), ret = E_OK, BufState = BUFREQ_OK
//=====================================================================================================================
	uint8 ObtainedFrame2[8] = {0x1f, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	PduInfo.SduLength = 4095; //maximum available length in this case
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_OK;
	CanIf_Transmit_fake.return_val = E_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_PROCESSING_SUSPEND);
	for(uint8 i = 0; i < 8; i++){TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[i] == ObtainedFrame2[i]);}
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 5 - CANTP_ON --> Sending first frame (SduLength > 4095), ret = E_OK, BufState = BUFREQ_OK
//=====================================================================================================================
	uint8 ObtainedFrame3[8] = {0x10, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x02};
	PduInfo.SduLength = 4294967295; //maximum available length in this case
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_OK;
	CanIf_Transmit_fake.return_val = E_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_PROCESSING_SUSPEND);
	for(uint8 i = 0; i < 8; i++){TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[i] == ObtainedFrame3[i]);}
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 6 - CANTP_ON --> Sending single frame (SduLength < 8), BufState = BUFREQ_OK, CanIf_Transmit = E_NOT_OK
//=====================================================================================================================
	PduInfo.SduLength = 7;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_OK;
	CanIf_Transmit_fake.return_val = E_NOT_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	
//=====================================================================================================================
//	Test 7 - CANTP_ON --> Sending single frame (SduLength < 8), BufState = BUFREQ_E_NOT_OK
//=====================================================================================================================
	PduInfo.SduLength = 7;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_E_NOT_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=======-=============================================================================================================
//	Test 8 - CANTP_ON --> Sending single frame (SduLength < 8), BufState = BUFREQ_E_BUSY
//=====================================================================================================================
	PduInfo.SduLength = 7;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	PduR_CanTpCopyTxData_fake.return_val = BUFREQ_E_BUSY;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 9 - CANTP_ON --> Sending first frame (SduLength <= 4095), CanIf_Transmit = E_NOT_OK
//=====================================================================================================================
	PduInfo.SduLength = 4095; //maximum available length in this case
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	CanIf_Transmit_fake.return_val = E_NOT_OK;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_WAIT);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 10 - CANTP_ON --> CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState != CANTP_TX_WAIT, ret = E_NOT_OK
//=====================================================================================================================
	PduInfo.SduLength = 4096;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING;
	ret = CanTp_Transmit(PduId, &PduInfo);
	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(eCanTp_State == CANTP_ON);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0);
	TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 0);
	
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}
/**
  @brief Test of CanTp_CancelTransmit function

  This function tests CanTp_CancelTransmit function.
*/
void TestOf_CanTp_CancelTransmit(void) {
	Std_ReturnType ret;
	PduIdType ID = 0x02U;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId = 0x02U;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING;

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.CanTp_CurrentTxPduId == 0x02U);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_PROCESSING);

//=====================================================================================================================
//	Test 1 - TxID correct
//=====================================================================================================================
	ret = CanTp_CancelTransmit(ID);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_PROCESSING_SUSPEND);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 2 - TxID incorrect
//=====================================================================================================================
	ID = 0x05U;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_PROCESSING;
	ret = CanTp_CancelTransmit(ID);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_PROCESSING);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}
/**
  @brief Test of CanTp_CancelReceive function

  This function tests CanTp_CancelReceive function.
*/
void TestOf_CanTp_CancelReceive(void) {
	Std_ReturnType ret;
	PduIdType ID = 0x02U;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId = 0x02U;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING;

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId == 0x02U);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING);
//=====================================================================================================================
//	Test 1 - RxID correct
//=====================================================================================================================
	ret = CanTp_CancelReceive(ID);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);
	TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING_SUSPEND);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 2 - RxID incorrect
//=====================================================================================================================
	ID = 0x05U;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING;
	ret = CanTp_CancelReceive(ID);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}
/**
  @brief Test of CanTp_ChangeParameter function

  This function tests CanTp_ChangeParameter function.
*/
void TestOf_CanTp_ChangeParameter(void) {
	Std_ReturnType ret;
	PduIdType ID = 0x05;
	eCanTp_State = CANTP_ON;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId = 0x05U;
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = 0x00U;
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs = 0x00U;

//=====================================================================================================================
//	Test 1 - Changing TP_STMIN parameter from 0x00 to 0x7F
//=====================================================================================================================
	uint16 valueToWrite = 0x7FU;
	TPParameterType parameter = TP_STMIN;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin == 0x7FU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 2 - Changing TP_STMIN parameter from 0x7F to 0xF0
//=====================================================================================================================
	valueToWrite = 0xF0U;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin == 0x7FU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 3 - Changing TP_STMIN parameter from 0x7F to 0xF9
//=====================================================================================================================
	valueToWrite = 0xF9U;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin == 0xF9U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 4 - Changing TP_STMIN parameter from 0xF9 to 0xFF
//=====================================================================================================================
	valueToWrite = 0xFFU;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin == 0xF9U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 5 - Changing TP_BS parameter from 0x00 to 0xFF
//=====================================================================================================================
	valueToWrite = 0xFFU;
	parameter = TP_BS;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 6 - ID incorrect
//=====================================================================================================================
	valueToWrite = 0xFFU;
	ID = 0x10U;
	parameter = TP_BS;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 7 - RxState = CANTP_RX_PROCESSING
//=====================================================================================================================
	valueToWrite = 0xFFU;
	ID = 0x05U;
	parameter = TP_BS;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_PROCESSING;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 8 - CANTP_OFF
//=====================================================================================================================
	valueToWrite = 0xFFU;
	ID = 0x05U;
	parameter = TP_BS;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	eCanTp_State = CANTP_OFF;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 9 - parameter type invalid
//=====================================================================================================================
	valueToWrite = 0xFFU;
	ID = 0x05U;
	parameter = 0x10;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	eCanTp_State = CANTP_ON;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 10 - Changing TP_BS parameter from 0x00 to 0xFFF
//=====================================================================================================================
	valueToWrite = 0xFFFU;
	parameter = TP_BS;
	ret = CanTp_ChangeParameter(ID, parameter, valueToWrite);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}
/**
  @brief Test of CanTp_ReadParameter function

  This function tests CanTp_ReadParameter function.
*/
void TestOf_CanTp_ReadParameter(void) {
	Std_ReturnType ret;
	uint16 value = 0x00;
	PduIdType ID = 0x05;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.CanTp_CurrentRxPduId = 0x05U;
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = 0x55U;
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs = 0x55U;
	
//=====================================================================================================================
//	Test 1 - CANTP_OFF
//=====================================================================================================================	
	eCanTp_State = CANTP_OFF;
	TPParameterType parameter = TP_STMIN;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(value == 0x00U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 2 - Reading TP_STMIN parameter (TP_STMIN = 0x55)
//=====================================================================================================================	
	eCanTp_State = CANTP_ON;
	parameter = TP_STMIN;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(value == 0x55U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 3 - Reading TP_STMIN parameter (TP_STMIN = 0xF9)
//=====================================================================================================================	
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = 0xF9U;
	parameter = TP_STMIN;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(value == 0xF9U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 4 - Reading TP_STMIN parameter (TP_STMIN = 0xF0)
//=====================================================================================================================	
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = 0xF0U;
	parameter = TP_STMIN;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(value == 0xF9U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 5 - Reading TP_STMIN parameter (TP_STMIN = 0xFF)
//=====================================================================================================================	
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = 0xFFU;
	parameter = TP_STMIN;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(value == 0xF9U);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 6 - Reading TP_BS parameter (TP_BS = 0xFF)
//=====================================================================================================================	
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs = 0xFFU;
	parameter = TP_BS;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_OK);
	TEST_CHECK(value == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 7 - parameter type invalid
//=====================================================================================================================	
	parameter = 0x10;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(value == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 8 - ID incorrect
//=====================================================================================================================	
	parameter = TP_BS;
	ID = 0x08;
	ret = CanTp_ReadParameter(ID, parameter, &value);

	TEST_CHECK(ret == E_NOT_OK);
	TEST_CHECK(value == 0xFFU);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}
/**
  @brief Test of CanTp_MainFunction function

  This function tests CanTp_MainFunction function.
*/
void TestOf_CanTp_MainFunction(void) {
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpSTmin = 0x01;
	CanTp_ConfigPtr.CanTpChannel.RxNSdu.CanTpBs = 0x02;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
//=====================================================================================================================
//	Test 1 - TIMEOUTS TEST
//=====================================================================================================================	
	CanTp_BrTimer.eCanTp_TimerState = STOP;
	CanTp_ArTimer.CanTp_Counter = 999; CanTp_ArTimer.eCanTp_TimerState = START;
	CanTp_CrTimer.CanTp_Counter = 999; CanTp_CrTimer.eCanTp_TimerState = START;
	CanTp_BsTimer.CanTp_Counter = 999; CanTp_BsTimer.eCanTp_TimerState = START;
	CanTp_AsTimer.CanTp_Counter = 999; CanTp_AsTimer.eCanTp_TimerState = START;
	CanTp_CsTimer.CanTp_Counter = 999; CanTp_CsTimer.eCanTp_TimerState = START;
	
	CanTp_MainFunction();
	
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING_SUSPEND);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_PROCESSING_SUSPEND);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x02);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0x03);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 2 - TIMERS ON BUT NOT OVERFLOWED
//=====================================================================================================================	
	CanTp_BrTimer.eCanTp_TimerState = STOP;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState = CANTP_TX_WAIT;
	CanTp_ArTimer.CanTp_Counter = 0; CanTp_ArTimer.eCanTp_TimerState = START;
	CanTp_CrTimer.CanTp_Counter = 0; CanTp_CrTimer.eCanTp_TimerState = START;
	CanTp_BsTimer.CanTp_Counter = 0; CanTp_BsTimer.eCanTp_TimerState = START;
	CanTp_AsTimer.CanTp_Counter = 0; CanTp_AsTimer.eCanTp_TimerState = START;
	CanTp_CsTimer.CanTp_Counter = 0; CanTp_CsTimer.eCanTp_TimerState = START;

	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT);
	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_TxConfig.eCanTp_TxState == CANTP_TX_WAIT);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x00);
	TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0x00);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 3 - PduR_CanTpCopyRxData returns BUFREQ_E_NOT_OK
//=====================================================================================================================	
	CanTp_BrTimer.eCanTp_TimerState = START;
	PduR_CanTpCopyRxData_fake.return_val = BUFREQ_E_NOT_OK;

	CanTp_MainFunction();
	
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x01);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 4 - PduR_CanTpCopyRxData -> BUFREQ_OK, CalcBlockSize > 0, CanIfTransmit -> E_NOT_OK
//=====================================================================================================================	
	CanTp_ArTimer.CanTp_Counter = 0; CanTp_ArTimer.eCanTp_TimerState = STOP;
	CanTp_CrTimer.CanTp_Counter = 0; CanTp_CrTimer.eCanTp_TimerState = STOP;
	CanTp_BsTimer.CanTp_Counter = 0; CanTp_BsTimer.eCanTp_TimerState = STOP;
	CanTp_AsTimer.CanTp_Counter = 0; CanTp_AsTimer.eCanTp_TimerState = STOP;
	CanTp_CsTimer.CanTp_Counter = 0; CanTp_CsTimer.eCanTp_TimerState = STOP;
	PduR_CanTpCopyRxData_fake.return_val = BUFREQ_OK;
	CanTp_CalcBlockSize_fake.return_val = 0x02;
	CanIf_Transmit_fake.return_val = E_NOT_OK;
	CanTp_BrTimer.CanTp_Counter = 0;
	
	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING_SUSPEND);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x01);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0x01);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 5 - PduR_CanTpCopyRxData -> BUFREQ_OK, CalcBlockSize = 0
//=====================================================================================================================
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	CanTp_CalcBlockSize_fake.return_val = 0x00;
	CanTp_BrTimer.CanTp_Counter = 0;

	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x00);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0x00);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 6 - PduR_CanTpCopyRxData -> BUFREQ_OK, CalcBlockSize > 0, CanIfTransmit -> E_OK
//=====================================================================================================================
	PduR_CanTpCopyRxData_fake.return_val = BUFREQ_OK;
	CanTp_CalcBlockSize_fake.return_val = 0x02;
	CanIf_Transmit_fake.return_val = E_OK;
	CanTp_BrTimer.CanTp_Counter = 1;

	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x00);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0x01);
	TEST_CHECK(CanTp_BrTimer.CanTp_Counter == 0);
	TEST_CHECK(CanTp_BrTimer.eCanTp_TimerState == STOP);
	TEST_CHECK(CanTp_ArTimer.eCanTp_TimerState == START);
	TEST_CHECK(CanTp_CrTimer.eCanTp_TimerState == START);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 7 - PduR_CanTpCopyRxData -> BUFREQ_OK, CalcBlockSize = 0, BrTimer TIMEOUT, FCWFTcnt < CANTP_FCWFTMAX
//=====================================================================================================================
	CanTp_ArTimer.CanTp_Counter = 0; CanTp_ArTimer.eCanTp_TimerState = STOP;
	CanTp_CrTimer.CanTp_Counter = 0; CanTp_CrTimer.eCanTp_TimerState = STOP;
	CanTp_BsTimer.CanTp_Counter = 0; CanTp_BsTimer.eCanTp_TimerState = STOP;
	CanTp_AsTimer.CanTp_Counter = 0; CanTp_AsTimer.eCanTp_TimerState = STOP;
	CanTp_CsTimer.CanTp_Counter = 0; CanTp_CsTimer.eCanTp_TimerState = STOP;
	CanTp_BrTimer.eCanTp_TimerState = START;
	PduR_CanTpCopyRxData_fake.return_val = BUFREQ_OK;
	CanTp_CalcBlockSize_fake.return_val = 0x00;
	CanIf_Transmit_fake.return_val = E_NOT_OK;
	CanTp_BrTimer.CanTp_Counter = 999;
	CanTp_FCWFTcnt = 7;

	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_PROCESSING_SUSPEND);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x01);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0x01);
	TEST_CHECK(CanTp_BrTimer.CanTp_Counter == 0);
	TEST_CHECK(CanTp_FCWFTcnt == 8);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 8 - CanIfTransmit -> E_OK, CalcBlockSize = 0, BrTimer TIMEOUT, FCWFTcnt < CANTP_FCWFTMAX
//=====================================================================================================================
	PduR_CanTpCopyRxData_fake.return_val = BUFREQ_OK;
	CanTp_CalcBlockSize_fake.return_val = 0x00;
	CanIf_Transmit_fake.return_val = E_OK;
	CanTp_BrTimer.CanTp_Counter = 999;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;

	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x00);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0x01);
	TEST_CHECK(CanTp_BrTimer.CanTp_Counter == 0);
	TEST_CHECK(CanTp_ArTimer.eCanTp_TimerState == START);
	TEST_CHECK(CanTp_BrTimer.eCanTp_TimerState == START);
	TEST_CHECK(CanTp_FCWFTcnt == 9);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
//=====================================================================================================================
//	Test 9 - PduR_CanTpCopyRxData -> BUFREQ_OK, CalcBlockSize = 0, BrTimer TIMEOUT, FCWFTcnt >= CANTP_FCWFTMAX
//=====================================================================================================================
	PduR_CanTpCopyRxData_fake.return_val = BUFREQ_OK;
	CanTp_CalcBlockSize_fake.return_val = 0x00;
	CanIf_Transmit_fake.return_val = E_OK;
	CanTp_BrTimer.CanTp_Counter = 999;
	CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState = CANTP_RX_WAIT;
	CanTp_ArTimer.eCanTp_TimerState = STOP;

	CanTp_MainFunction();

	TEST_CHECK(CanTp_RxTxVariablesConfig.CanTp_RxConfig.eCanTp_RxState == CANTP_RX_WAIT);
	TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0x01);
	TEST_CHECK(CanIf_Transmit_fake.call_count == 0x00);
	TEST_CHECK(CanTp_BrTimer.CanTp_Counter == 0);
	TEST_CHECK(CanTp_ArTimer.eCanTp_TimerState == STOP);
	TEST_CHECK(CanTp_BrTimer.eCanTp_TimerState == START);
	TEST_CHECK(CanTp_FCWFTcnt == 0);
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}

TEST_LIST = {
	{ "Test of CanTp_Init", TestOf_CanTp_Init },
    { "Test of CanTp_GetVersionInfo", TestOf_CanTp_GetVersionInfo },
    { "Test of CanTp_Shutdown", TestOf_CanTp_Shutdown },
	{ "Test of CanTp_Transmit", TestOf_CanTp_Transmit },
	{ "Test of CanTp_CancelTransmit", TestOf_CanTp_CancelTransmit },
	{ "Test of CanTp_CancelReceive", TestOf_CanTp_CancelReceive },
	{ "Test of CanTp_ChangeParameter", TestOf_CanTp_ChangeParameter },
	{ "Test of CanTp_ReadParameter", TestOf_CanTp_ReadParameter },
	{ "Test of CanTp_MainFunction", TestOf_CanTp_MainFunction },
    { NULL, NULL }                           
};