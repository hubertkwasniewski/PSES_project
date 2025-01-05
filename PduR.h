#ifndef PDUR_H
#define PDUR_H

/*===========================================================================================================================================================*\
  @file PduR.h

  @brief PDU Router

  @see PDU Router specification (https://www.autosar.org/fileadmin/standards/R18-10_R4.4.0_R1.5.0/CP/AUTOSAR_SWS_PDURouter.pdf)
\*===========================================================================================================================================================*/

/*====================================================================================================================*\
  Headers
\*====================================================================================================================*/
	#include "CommunicationStackTypes.h"
/*====================================================================================================================*\
  Declarations of functions
\*====================================================================================================================*/
	BufReq_ReturnType PduR_CanTpCopyRxData (PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr);
	void PduR_CanTpRxIndication (PduIdType id, Std_ReturnType result);
	BufReq_ReturnType PduR_CanTpStartOfReception (PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
	BufReq_ReturnType PduR_CanTpCopyTxData (PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr);
	void PduR_CanTpTxConfirmation (PduIdType id, Std_ReturnType result);

#endif /* PDUR_H */