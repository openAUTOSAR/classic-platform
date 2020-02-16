#include "Det.h"

#if !(((DET_SW_MAJOR_VERSION == 1) && (DET_SW_MINOR_VERSION == 0)) )
#error Det: Configuration file expected BSW module version to be 1.0.*
#endif

#if !(((DET_AR_MAJOR_VERSION == 4) && (DET_AR_MINOR_VERSION == 0)) )
#error Det: Expected AUTOSAR version to be 4.0.*
#endif

#if (DET_USE_STATIC_CALLBACKS == STD_ON )

extern void StaticErrorHookTest(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

detCbk_t DetStaticHooks[] = {
	StaticErrorHookTest,
};
#endif

