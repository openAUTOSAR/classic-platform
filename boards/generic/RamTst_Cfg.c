/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#include "RamTst.h"

#warning "This default file may only be used as an example!"

#define RAMTSTALGPARAMS_RAMTSTBLOCKPARAMS_INDEX 0

const RamTst_BlockParamsType RamTstAlgParams_RamTstBlockParams[] = {
    {
            .RamTstBlockId = 1,
            .RamTstBlockIndex = RAMTSTALGPARAMS_RAMTSTBLOCKPARAMS_INDEX,
            .RamTstStartAddress = (uint32)MyRamTestArea,
            .RamTstEndAddress = ((uint32)MyRamTestArea+2048),
    }
};

const RamTst_AlgParamsType RamTstAlgParams[] = {
    {
            .RamTstAlgParamsId = 19,
            .RamTstAlgorithm = RAMTST_MARCH_TEST,
            .RamTstNumberOfBlocks = 2,
            .RamTstBlockParams = RamTstAlgParams_RamTstBlockParams,
    }
};

const RamTst_ConfigParamsType RamTst_ConfigParams = {
        .RamTstDefaultAlgParamsId = 1,
        .RamTstNumberOfAlgParamSets = 1,
#if defined(USE_DEM)
        .RAMTST_E_RAM_FAILURE = DemConf_DemEventParameter_RAMTST_E_RAM_FAILURE,
#endif
        .RamTstAlgParams = RamTstAlgParams,
};



