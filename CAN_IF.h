#ifndef CAN_IF_H
#define CAN_IF_H

/*===========================================================================================================================================================*\
  @file CAN_IF.h

  @brief CAN Interface

  @see CAN Interface specification (https://www.autosar.org/fileadmin/standards/R18-10_R4.4.0_R1.5.0/CP/AUTOSAR_SWS_CANInterface.pdf)
\*===========================================================================================================================================================*/

/*====================================================================================================================*\
  Headers
\*====================================================================================================================*/
	#include "CommunicationStackTypes.h"
/*====================================================================================================================*\
  Declarations of functions
\*====================================================================================================================*/
	Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);

#endif /* CAN_IF_H */