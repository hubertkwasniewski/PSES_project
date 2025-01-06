#ifndef COMMSTACK_TYPES_H
#define COMMSTACK_TYPES_H

/*===========================================================================================================================================================*\
  @file CommunicationStackTypes.h

  @brief Communication Stack Types

  @see Communication Stack Types specification (https://www.autosar.org/fileadmin/standards/R18-10_R4.4.0_R1.5.0/CP/AUTOSAR_SWS_CommunicationStackTypes.pdf)
\*===========================================================================================================================================================*/

/*====================================================================================================================*\
  Headers
\*====================================================================================================================*/
	#include "Std_Types.h"
	#include "Platform_Types.h"
/*====================================================================================================================*\
  Typedefs
\*====================================================================================================================*/
	typedef enum {
		TP_DATACONF, 
		TP_DATARETRY,   
		TP_CONFPENDING  
	}TpDataStateType;

	typedef enum {
		BUFREQ_OK,       
		BUFREQ_E_NOT_OK, 
		BUFREQ_E_BUSY,     
		BUFREQ_E_OVFL 
	}BufReq_ReturnType;

	typedef uint16 PduIdType;
	typedef uint32 PduLengthType;

	typedef struct{
		uint8*        SduDataPtr;   
		uint8*        MetaDataPtr;  
		PduLengthType SduLength;    
	}PduInfoType;

	typedef struct{
		TpDataStateType TpDataState; 
		PduLengthType   TxTpDataCnt;
	}RetryInfoType;

	typedef enum{
		TP_STMIN,  
		TP_BS,      
		TP_BC       
	}TPParameterType;

#endif /* COMMSTACK_TYPES_H */