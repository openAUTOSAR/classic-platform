/*
 * ip_edma.h
 *
 *  Created on: 31 maj 2011
 *      Author: mahi
 */

#ifndef IP_EDMA_H_
#define IP_EDMA_H_

/* VERSION     : 2.04 */


/*for "standard" format TCD (when EDMA.TCD[x].CITER.E_LINK==BITER.E_LINK=0) */
    struct EDMA_TCD_STD_tag {

        vuint32_t SADDR;               /* source address */

        vuint16_t SMOD:5;              /* source address modulo */
        vuint16_t SSIZE:3;             /* source transfer size */
        vuint16_t DMOD:5;              /* destination address modulo */
        vuint16_t DSIZE:3;             /* destination transfer size */
        vint16_t SOFF;                 /* signed source address offset */

        union {
            vuint32_t R;
            struct {
                vuint32_t SMLOE:1;
                vuint32_t DMLOE:1;
                int32_t   MLOFF:20;
                vuint32_t NBYTES:10;
            } B;
        } NBYTESu;                     /* Region Descriptor Alternate Access Control n */

        vint32_t SLAST;                /* last destination address adjustment, or
                                          scatter/gather address (if e_sg = 1) */

        vuint32_t DADDR;               /* destination address */

        vuint16_t CITERE_LINK:1;
        vuint16_t CITER:15;

        vint16_t DOFF;                 /* signed destination address offset */

        vint32_t DLAST_SGA;

        vuint16_t BITERE_LINK:1;       /* beginning ("major") iteration count */
        vuint16_t BITER:15;

        vuint16_t BWC:2;               /* bandwidth control */
        vuint16_t MAJORLINKCH:6;       /* enable channel-to-channel link */
        vuint16_t DONE:1;              /* channel done */
        vuint16_t ACTIVE:1;            /* channel active */
        vuint16_t MAJORE_LINK:1;       /* enable channel-to-channel link */
        vuint16_t E_SG:1;              /* enable scatter/gather descriptor */
        vuint16_t D_REQ:1;             /* disable ipd_req when done */
        vuint16_t INT_HALF:1;          /* interrupt on citer = (biter >> 1) */
        vuint16_t INT_MAJ:1;           /* interrupt on major loop completion */
        vuint16_t START:1;             /* explicit channel start */

    };                                 /* end of EDMA_TCD_STD_tag */

/*for "channel link" format TCD (when EDMA.TCD[x].CITER.E_LINK==BITER.E_LINK=1)*/
    struct EDMA_TCD_CHLINK_tag {

        vuint32_t SADDR;               /* source address */

        vuint16_t SMOD:5;              /* source address modulo */
        vuint16_t SSIZE:3;             /* source transfer size */
        vuint16_t DMOD:5;              /* destination address modulo */
        vuint16_t DSIZE:3;             /* destination transfer size */
        vint16_t SOFF;                 /* signed source address offset */

        union {
            vuint32_t R;
            struct {
                vuint32_t SMLOE:1;
                vuint32_t DMLOE:1;
                int32_t   MLOFF:20;
                vuint32_t NBYTES:10;
            } B;
        } NBYTESu;                     /* Region Descriptor Alternate Access Control n */
        vint32_t SLAST;                /* last destination address adjustment, or
                                          scatter/gather address (if e_sg = 1) */

        vuint32_t DADDR;               /* destination address */

        vuint16_t CITERE_LINK:1;
        vuint16_t CITERLINKCH:6;
        vuint16_t CITER:9;

        vint16_t DOFF;                 /* signed destination address offset */

        vint32_t DLAST_SGA;

        vuint16_t BITERE_LINK:1;       /* beginning ("major") iteration count */
        vuint16_t BITERLINKCH:6;
        vuint16_t BITER:9;

        vuint16_t BWC:2;               /* bandwidth control */
        vuint16_t MAJORLINKCH:6;       /* enable channel-to-channel link */
        vuint16_t DONE:1;              /* channel done */
        vuint16_t ACTIVE:1;            /* channel active */
        vuint16_t MAJORE_LINK:1;       /* enable channel-to-channel link */
        vuint16_t E_SG:1;              /* enable scatter/gather descriptor */
        vuint16_t D_REQ:1;             /* disable ipd_req when done */
        vuint16_t INT_HALF:1;          /* interrupt on citer = (biter >> 1) */
        vuint16_t INT_MAJ:1;           /* interrupt on major loop completion */
        vuint16_t START:1;             /* explicit channel start */

    };                                 /* end of EDMA_TCD_CHLINK_tag */


CC_EXTENSION struct EDMA_tag {
        union {
            vuint32_t R;
            struct {
                  vuint32_t:14;
                vuint32_t CX:1;
                vuint32_t ECX:1;
                vuint32_t GRP3PRI:2;
                vuint32_t GRP2PRI:2;
                vuint32_t GRP1PRI:2;
                vuint32_t GRP0PRI:2;
                vuint32_t EMLM:1;
                vuint32_t CLM:1;
                vuint32_t HALT:1;
                vuint32_t HOE:1;
                vuint32_t ERGA:1;
                vuint32_t ERCA:1;
                vuint32_t EDBG:1;
                vuint32_t EBW:1;
            } B;
        } CR;                          /* Control Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t VLD:1;
                  vuint32_t:14;
                vuint32_t ECX:1;
                vuint32_t GPE:1;
                vuint32_t CPE:1;
                vuint32_t ERRCHN:6;
                vuint32_t SAE:1;
                vuint32_t SOE:1;
                vuint32_t DAE:1;
                vuint32_t DOE:1;
                vuint32_t NCE:1;
                vuint32_t SGE:1;
                vuint32_t SBE:1;
                vuint32_t DBE:1;
            } B;
        } ES;                          /* Error Status Register */

        int16_t EDMA_reserved1[3];     /* (0x0E - 0x08)/2 = 0x03 */

        union {
            vuint16_t R;
            struct {
                vuint16_t ERQ15:1;
                vuint16_t ERQ14:1;
                vuint16_t ERQ13:1;
                vuint16_t ERQ12:1;
                vuint16_t ERQ11:1;
                vuint16_t ERQ10:1;
                vuint16_t ERQ09:1;
                vuint16_t ERQ08:1;
                vuint16_t ERQ07:1;
                vuint16_t ERQ06:1;
                vuint16_t ERQ05:1;
                vuint16_t ERQ04:1;
                vuint16_t ERQ03:1;
                vuint16_t ERQ02:1;
                vuint16_t ERQ01:1;
                vuint16_t ERQ00:1;
            } B;
        } ERQL;                        /* DMA Enable Request Low */

        int16_t EDMA_reserved2[3];     /* (0x16 - 0x10)/2 = 0x03 */

        union {
            vuint16_t R;
            struct {
                vuint16_t EEI15:1;
                vuint16_t EEI14:1;
                vuint16_t EEI13:1;
                vuint16_t EEI12:1;
                vuint16_t EEI11:1;
                vuint16_t EEI10:1;
                vuint16_t EEI09:1;
                vuint16_t EEI08:1;
                vuint16_t EEI07:1;
                vuint16_t EEI06:1;
                vuint16_t EEI05:1;
                vuint16_t EEI04:1;
                vuint16_t EEI03:1;
                vuint16_t EEI02:1;
                vuint16_t EEI01:1;
                vuint16_t EEI00:1;
            } B;
        } EEIL;                        /* DMA Enable Error Interrupt Low */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t SERQ:7;
            } B;
        } SERQ;                        /* DMA Set Enable Request Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t CERQ:7;
            } B;
        } CERQ;                        /* DMA Clear Enable Request Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t SEEI:7;
            } B;
        } SEEI;                        /* DMA Set Enable Error Interrupt Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t CEEI:7;
            } B;
        } CEEI;                        /* DMA Clear Enable Error Interrupt Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t CINT:7;
            } B;
        } CINT;                        /* DMA Clear Interrupt Request Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t CER:7;
            } B;
        } CERR;                        /* DMA Clear error Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t SSB:7;
            } B;
        } SSRT;                        /* Set Start Bit Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t NOP:1;
                vuint8_t CDNE:7;
            } B;
        } CDNE;                        /* Clear Done Status Bit Register */

        int16_t EDMA_reserved3[3];     /* (0x26 - 0x20)/2 = 0x03 */

        union {
            vuint16_t R;
            struct {
                vuint16_t INT15:1;
                vuint16_t INT14:1;
                vuint16_t INT13:1;
                vuint16_t INT12:1;
                vuint16_t INT11:1;
                vuint16_t INT10:1;
                vuint16_t INT09:1;
                vuint16_t INT08:1;
                vuint16_t INT07:1;
                vuint16_t INT06:1;
                vuint16_t INT05:1;
                vuint16_t INT04:1;
                vuint16_t INT03:1;
                vuint16_t INT02:1;
                vuint16_t INT01:1;
                vuint16_t INT00:1;
            } B;
        } INTL;                        /* DMA Interrupt Request Low */

        int16_t EDMA_reserved4[3];     /* (0x2E - 0x28)/2 = 0x03 */

        union {
            vuint16_t R;
            struct {
                vuint16_t ERR15:1;
                vuint16_t ERR14:1;
                vuint16_t ERR13:1;
                vuint16_t ERR12:1;
                vuint16_t ERR11:1;
                vuint16_t ERR10:1;
                vuint16_t ERR09:1;
                vuint16_t ERR08:1;
                vuint16_t ERR07:1;
                vuint16_t ERR06:1;
                vuint16_t ERR05:1;
                vuint16_t ERR04:1;
                vuint16_t ERR03:1;
                vuint16_t ERR02:1;
                vuint16_t ERR01:1;
                vuint16_t ERR00:1;
            } B;
        } ERRL;                        /* DMA Error Low */

        int16_t EDMA_reserved5[3];     /* (0x36 - 0x30)/2 = 0x03 */

        union {
            vuint16_t R;
            struct {
                vuint16_t HRS15:1;
                vuint16_t HRS14:1;
                vuint16_t HRS13:1;
                vuint16_t HRS12:1;
                vuint16_t HRS11:1;
                vuint16_t HRS10:1;
                vuint16_t HRS09:1;
                vuint16_t HRS08:1;
                vuint16_t HRS07:1;
                vuint16_t HRS06:1;
                vuint16_t HRS05:1;
                vuint16_t HRS04:1;
                vuint16_t HRS03:1;
                vuint16_t HRS02:1;
                vuint16_t HRS01:1;
                vuint16_t HRS00:1;
            } B;
        } HRSL;                        /* DMA Hardware Request Status Low */

        uint32_t edma_reserved1[50];   /* (0x100 - 0x038)/4 = 0x32 */

        union {
            vuint8_t R;
            struct {
                vuint8_t ECP:1;
                vuint8_t DPA:1;
                vuint8_t GRPPRI:2;
                vuint8_t CHPRI:4;
            } B;
        } CPR[16];                     /* Channel n Priority */

        uint32_t edma_reserved2[956];  /* (0x1000 - 0x0110)/4 = 0x3BC */

        /* Select one of the following declarations depending on the DMA mode chosen */
            struct EDMA_TCD_STD_tag TCD[16];
            /* struct EDMA_TCD_CHLINK_tag TCD[16]; */

        };                                 /* end of EDMA_tag */



#endif /* IP_EDMA_H_ */
