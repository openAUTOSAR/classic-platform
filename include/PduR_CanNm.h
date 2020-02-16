/* @req PDUR216 */

#ifndef PDUR_CANNM_H
#define PDUR_CANNM_H

#include "ComStack_Types.h"

#if PDUR_ZERO_COST_OPERATION == STD_OFF

void PduR_CanNmRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr);
void PduR_CanNmTxConfirmation(PduIdType pduId);
Std_ReturnType PduR_CanNmTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr);

#endif

#endif
