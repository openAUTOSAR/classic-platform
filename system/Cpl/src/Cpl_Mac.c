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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=GENERIC */

/* @req SWS_Cal_00534 The code file structure shall contain one or more MISRA-C 2004 conform source files Cpl_<xxx>.c */
/* @req SWS_Cal_00505 The implementation shall be syncronous and reentrant */

#include "Cpl_Mac.h"
#include <string.h>
#include "crypto/hmac_sha2.h"

/*
 * Private functions
 */

/*
 * Copy values from ctx values to MacGenerateCtxBuffer
 */
static void copyToGenerateBuffer(Cal_MacGenerateCtxBufType contextBuffer, hmac_sha256_ctx ctx) {
#if CAL_MACGENERATE_CONTEXT_BUFFER_SIZE < 465
    return;
#else

    // ctx inside
    memcpy(&contextBuffer[1], &ctx.ctx_inside.len, sizeof(ctx.ctx_inside.len));
    memcpy(&contextBuffer[5], &ctx.ctx_inside.tot_len, sizeof(ctx.ctx_inside.tot_len));
    memcpy(&contextBuffer[9], ctx.ctx_inside.block, sizeof(ctx.ctx_inside.block));
    memcpy(&contextBuffer[137], ctx.ctx_inside.h, sizeof(ctx.ctx_inside.h));

    // ctx outside
    memcpy(&contextBuffer[169], &ctx.ctx_outside.len, sizeof(ctx.ctx_outside.len));
    memcpy(&contextBuffer[173], &ctx.ctx_outside.tot_len, sizeof(ctx.ctx_outside.tot_len));
    memcpy(&contextBuffer[177], ctx.ctx_outside.block, sizeof(ctx.ctx_outside.block));
    memcpy(&contextBuffer[305], ctx.ctx_outside.h, sizeof(ctx.ctx_outside.h));

    // block ipad
    memcpy(&contextBuffer[337], ctx.block_ipad, sizeof(ctx.block_ipad));

    // block opad
    memcpy(&contextBuffer[401], ctx.block_opad, sizeof(ctx.block_opad));
#endif
}


/*
 * Copy values from MacGenerateCtxBuffer to ctx
 */

static void copyFromGenerateBuffer(Cal_MacGenerateCtxBufType contextBuffer, hmac_sha256_ctx* ctx) {
#if CAL_MACGENERATE_CONTEXT_BUFFER_SIZE < 465
    return;
#else

    // ctx inside
    memcpy(&ctx->ctx_inside.len, &contextBuffer[1], sizeof(ctx->ctx_inside.len));
    memcpy(&ctx->ctx_inside.tot_len, &contextBuffer[5], sizeof(ctx->ctx_inside.tot_len));
    memcpy(ctx->ctx_inside.block, &contextBuffer[9], sizeof(ctx->ctx_inside.block));
    memcpy(ctx->ctx_inside.h, &contextBuffer[137], sizeof(ctx->ctx_inside.h));

    // ctx outside
    memcpy(&ctx->ctx_outside.len, &contextBuffer[169], sizeof(ctx->ctx_outside.len));
    memcpy(&ctx->ctx_outside.tot_len, &contextBuffer[173], sizeof(ctx->ctx_outside.tot_len));
    memcpy(ctx->ctx_outside.block, &contextBuffer[177], sizeof(ctx->ctx_outside.block));
    memcpy(ctx->ctx_outside.h, &contextBuffer[305], sizeof(ctx->ctx_outside.h));

    // block ipad
    memcpy(ctx->block_ipad, &contextBuffer[337], sizeof(ctx->block_ipad));

    // block opad
    memcpy(ctx->block_opad, &contextBuffer[401], sizeof(ctx->block_opad));
#endif
}

/*
 * Copy values from ctx to MacVerifyCtxBuffer
 */
static void copyToVerifyBuffer(Cal_MacVerifyCtxBufType contextBuffer, hmac_sha256_ctx ctx) {
#if CAL_MACVERIFY_CONTEXT_BUFFER_SIZE < 465
    return;
#else

    // ctx inside
    memcpy(&contextBuffer[1], &ctx.ctx_inside.len, sizeof(ctx.ctx_inside.len));
    memcpy(&contextBuffer[5], &ctx.ctx_inside.tot_len, sizeof(ctx.ctx_inside.tot_len));
    memcpy(&contextBuffer[9], ctx.ctx_inside.block, sizeof(ctx.ctx_inside.block));
    memcpy(&contextBuffer[137], ctx.ctx_inside.h, sizeof(ctx.ctx_inside.h));

    // ctx outside
    memcpy(&contextBuffer[169], &ctx.ctx_outside.len, sizeof(ctx.ctx_outside.len));
    memcpy(&contextBuffer[173], &ctx.ctx_outside.tot_len, sizeof(ctx.ctx_outside.tot_len));
    memcpy(&contextBuffer[177], ctx.ctx_outside.block, sizeof(ctx.ctx_outside.block));
    memcpy(&contextBuffer[305], ctx.ctx_outside.h, sizeof(ctx.ctx_outside.h));

    // block ipad
    memcpy(&contextBuffer[337], ctx.block_ipad, sizeof(ctx.block_ipad));

    // block opad
    memcpy(&contextBuffer[401], ctx.block_opad, sizeof(ctx.block_opad));
#endif
}

/*
 * Copy values from MacVerifyCtxBuffer to ctx
 */
static void copyFromVerifyBuffer(Cal_MacVerifyCtxBufType contextBuffer, hmac_sha256_ctx* ctx) {
#if CAL_MACVERIFY_CONTEXT_BUFFER_SIZE < 465
    return;
#else

    // ctx inside
    memcpy(&ctx->ctx_inside.len, &contextBuffer[1], sizeof(ctx->ctx_inside.len));
    memcpy(&ctx->ctx_inside.tot_len, &contextBuffer[5], sizeof(ctx->ctx_inside.tot_len));
    memcpy(ctx->ctx_inside.block, &contextBuffer[9], sizeof(ctx->ctx_inside.block));
    memcpy(ctx->ctx_inside.h, &contextBuffer[137], sizeof(ctx->ctx_inside.h));

    // ctx outside
    memcpy(&ctx->ctx_outside.len, &contextBuffer[169], sizeof(ctx->ctx_outside.len));
    memcpy(&ctx->ctx_outside.tot_len, &contextBuffer[173], sizeof(ctx->ctx_outside.tot_len));
    memcpy(ctx->ctx_outside.block, &contextBuffer[177], sizeof(ctx->ctx_outside.block));
    memcpy(ctx->ctx_outside.h, &contextBuffer[305], sizeof(ctx->ctx_outside.h));

    // block ipad
    memcpy(ctx->block_ipad, &contextBuffer[337], sizeof(ctx->block_ipad));

    // block opad
    memcpy(ctx->block_opad, &contextBuffer[401], sizeof(ctx->block_opad));
#endif
}

/*
 * Public functions
 */

/* @req SWS_Cal_00701 Implementation of Cpl<MAC>Start */
Cal_ReturnType Cpl_MacGenerateStart(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr) {

    Cal_ReturnType status;
    status = CAL_E_OK;
    if ((cfgPtr == NULL) || (keyPtr == NULL)) {
        status = CAL_E_NOT_OK;
    } else {
        //Activate the service state
        contextBuffer[0] = 1;


        // Copy key to the buffer
        hmac_sha256_ctx ctx;
        hmac_sha256_init(&ctx, keyPtr->data, keyPtr->length);

        //copy hmac ctx to context buffer
        copyToGenerateBuffer(contextBuffer,ctx);
        status = CAL_E_OK;
    }
    return status;
}

/* @req SWS_Cal_00702 Implementation of Cpl<MAC>Update */
Cal_ReturnType Cpl_MacGenerateUpdate(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength) {

    Cal_ReturnType status;
    status = CAL_E_OK;
    if ((cfgPtr == NULL) || (dataPtr == NULL)) {
        status = CAL_E_NOT_OK;
    } else {
        // Handle cfg

        // Read contextBuffer
        hmac_sha256_ctx ctx;

        //copy from contextBuffer to ctxBuffer
        copyFromGenerateBuffer(contextBuffer,&ctx);

        // Calculate MAC
        hmac_sha256_update(&ctx, dataPtr, dataLength);

        //copy hmac ctx to context buffer

        copyToGenerateBuffer(contextBuffer,ctx);

        status = CAL_E_OK;
    }
    return status;
}

/* @req SWS_Cal_00703 Implementation of Cpl<MAC>Finish */
Cal_ReturnType Cpl_MacGenerateFinish(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, uint8* resultPtr, uint32* resultLengthPtr, boolean TruncationIsAllowed) {

    Cal_ReturnType status;
    status = CAL_E_OK;
    if ((cfgPtr == NULL) || (resultPtr == NULL) || (resultLengthPtr == NULL)) {
        status = CAL_E_NOT_OK;
    } else {
        // Handle cfg

        // Read contextBuffer
        hmac_sha256_ctx ctx;

        //copy from buffer to ctx
        copyFromGenerateBuffer(contextBuffer,&ctx);

        // Calculate MAC
        if ((*resultLengthPtr < SHA256_DIGEST_SIZE) && (FALSE == TruncationIsAllowed)) {
            *resultLengthPtr = SHA256_DIGEST_SIZE;
            status = CAL_E_NOT_OK;
        } else {
            hmac_sha256_final(&ctx, resultPtr, *resultLengthPtr);
            *resultLengthPtr = SHA256_DIGEST_SIZE;
            status = CAL_E_OK;
        }

    }
    return status;
}

/* @req SWS_Cal_00701 Implementation of Cpl_MacVerifyStart */
Cal_ReturnType Cpl_MacVerifyStart(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr) {

    Cal_ReturnType status;
    status = CAL_E_OK;
    if((keyPtr == NULL) || (cfgPtr == NULL)) {
        status = CAL_E_NOT_OK;
    } else {
        hmac_sha256_ctx ctx;

        // Copy key to the buffer
        hmac_sha256_init(&ctx, keyPtr->data, keyPtr->length);

        //copy hmac ctx to context buffer
        copyToVerifyBuffer(contextBuffer,ctx);
    }
    return status;
}

/* @req SWS_Cal_00702 Implementation of Cpl_MacVerifyStart */
Cal_ReturnType Cpl_MacVerifyUpdate(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength) {

    Cal_ReturnType status;
    status = CAL_E_OK;
    if(cfgPtr == NULL) {
        status = CAL_E_NOT_OK;
    } else {
        hmac_sha256_ctx ctx;

        //copy from contextBuffer to ctxBuffer
        copyFromVerifyBuffer(contextBuffer,&ctx);

        // Calculate MAC
        hmac_sha256_update(&ctx, dataPtr, dataLength);

        //copy hmac ctx to context buffer
        copyToVerifyBuffer(contextBuffer,ctx);
    }
    return status;
}

/* @req SWS_Cal_00703 Implementation of Cpl_MacVerifyFinish */
Cal_ReturnType Cpl_MacVerifyFinish(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const uint8* MacPtr, uint32 MacLength, Cal_VerifyResultType* resultPtr) {

    Cal_ReturnType status = CAL_E_OK;
    if((MacPtr == NULL) || (cfgPtr == NULL)) {
        *resultPtr = CAL_E_VER_NOT_OK;
        status = CAL_E_NOT_OK;
    } else {
        hmac_sha256_ctx ctx;

        //result array
        uint8 result[32];
        uint32 resultLength=32;

        //copy from ctx buffer
        copyFromVerifyBuffer(contextBuffer,&ctx);

        hmac_sha256_final(&ctx, result, resultLength);

        //verify the MAC
        if(memcmp(result, MacPtr, MacLength) == 0){
            *resultPtr = CAL_E_VER_OK;
        } else {
            *resultPtr = CAL_E_VER_NOT_OK;
        }
    }
    return status;
}
