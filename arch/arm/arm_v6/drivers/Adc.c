/*
 * Adc.c
 *
 *  Created on: 1 sep 2014
 *      Author: sse
 */


#include <assert.h>
#include <stdlib.h>
//#include "System.h"
//#include "Modules.h"
#include "Mcu.h"
#include "Adc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_KERNEL)
#include "Os.h"
#include "isr.h"
#endif
#include "Adc_Internal.h"

void Adc_Init (const Adc_ConfigType *ConfigPtr){

}


Std_ReturnType Adc_SetupResultBuffer (Adc_GroupType group, Adc_ValueGroupType *bufferPtr){

}


static void Adc_Group0ConversionComplete (void){

}
