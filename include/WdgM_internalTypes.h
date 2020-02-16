#ifndef WDGM_INTERNALTYPESH
#define WDGM_INTERNALTYPESH

#include "WdgM_Cfg.h"

typedef struct
{
	uint32			AliveCounter; 				/* maybe a smaller datatype applies */
	uint16			SupervisionCycleCounter;	/* maybe a smaller datatype applies */
	boolean			wasEvaluated;
} WdgM_internal_AliveSupervision;

typedef struct
{
	uint32			TimestampStart;
} WdgM_internal_DeadlineSupervision;

typedef struct
{
	WdgM_LocalStatusType	LocalState;
	WdgM_Substate			SubstateAlive;
	WdgM_Substate			SubstateDeadline;
	WdgM_Substate			SubstateLogical;

	boolean					IsInternalGraphActive; /* active flag for the internal graph */	/** @req WDGM271 */

	uint16					PreviousCheckpointId_Deadline;			/** @req WDGM246 */
	uint16					PreviousCheckpointId_internalLogic;		/** @req WDGM246 */

	uint8					FailedAliveCyclesCounter;
} WdgM_internal_SupervisedEntity;

typedef struct
{
	WdgM_internal_AliveSupervision 		*AliveSupervisions;
	const uint16						Length_AliveSupervisions;

	WdgM_internal_DeadlineSupervision 	*DeadlineSupervisions;
	const uint16						Length_DeadlineSupervisions;
} WdgM_internal_SupervisedEntityConfig;

typedef struct
{
	WdgM_internal_SupervisedEntityConfig	*SE_Configs;
	const uint16							Length_SEConfigs;

	uint16									ExpiredSupervisionCycleCounter;
} WdgM_internal_Mode;

typedef struct
{
	WdgM_internal_SupervisedEntity			*SEs;
	const uint16							Length_SEs;

	WdgM_internal_Mode						*Modes;
	const uint16							Length_Modes;

} WdgM_internal_RuntimeData;

#endif
