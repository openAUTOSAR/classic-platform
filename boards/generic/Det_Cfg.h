
#if !(((DET_SW_MAJOR_VERSION == 1) && (DET_SW_MINOR_VERSION == 0)) )
#error Det: Configuration file expected BSW module version to be 1.0.*
#endif

#if !(((DET_AR_MAJOR_VERSION == 4) && (DET_AR_MINOR_VERSION == 0)) )
#error Det: Expected AUTOSAR version to be 4.0.*
#endif

#ifndef DET_CFG_H_
#define DET_CFG_H_

/** @req DET101 */
#define DET_ENABLE_CALLBACKS STD_ON // Enable to use callback on errors
#define DET_USE_RAMLOG       STD_ON // Enable to log DET errors to ramlog
#define DET_WRAP_RAMLOG      STD_ON // The ramlog wraps around when reaching the end
#define DET_USE_STDERR       STD_OFF // Enable to get DET errors on stderr
#define DET_DEINIT_API       STD_ON // Enable/Disable the Det_DeInit function
#define DET_VERSIONINFO_API  STD_ON // Enable/Diable version info API

#define DET_RAMLOG_SIZE         (16) // Number of entries in ramlog
#define DET_NUMBER_OF_CALLBACKS (5) // Number of callbacks

#define DET_USE_STATIC_CALLBACKS STD_OFF // Enable static callbacks
#define DET_NUMBER_OF_STATIC_CALLBACKS 0

extern detCbk_t DetStaticHooks[];

#endif /* DET_CFG_H_ */

