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

FAKE_VALUE_FUNC(Std_ReturnType, CanIf_Transmit, PduIdType, const PduInfoType*);
//Only to reset all fakes
#define FFF_FAKES_LIST(FAKE) \
		FAKE(PduR_CanTpCopyTxData) \
		FAKE(PduR_CanTpTxConfirmation) \
		FAKE(PduR_CanTpRxIndication) \
		FAKE(PduR_CanTpCopyRxData) \
		FAKE(PduR_CanTpStartOfReception) \
		FAKE(CanIf_Transmit)
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

TEST_LIST = {
	{ "Test of CanTp_Init", TestOf_CanTp_Init },
    { "Test of CanTp_Transmit", TestOf_CanTp_Transmit },
    { "Test of CanTp_GetVersionInfo", TestOf_CanTp_GetVersionInfo },
    { "Test of CanTp_Shutdown", TestOf_CanTp_Shutdown },
    { NULL, NULL }                           
};