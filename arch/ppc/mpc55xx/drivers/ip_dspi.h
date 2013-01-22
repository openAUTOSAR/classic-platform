/*
 * ip_dspi.h
 *
 *  Created on: 31 maj 2011
 *      Author: mahi
 */

#ifndef IP_DSPI_H_
#define IP_DSPI_H_

 CC_EXTENSION struct DSPI_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MSTR:1;
                vuint32_t CONT_SCKE:1;
                vuint32_t DCONF:2;
                vuint32_t FRZ:1;
                vuint32_t MTFE:1;
                vuint32_t PCSSE:1;
                vuint32_t ROOE:1;
                  vuint32_t:2;
                vuint32_t PCSIS5:1;
                vuint32_t PCSIS4:1;
                vuint32_t PCSIS3:1;
                vuint32_t PCSIS2:1;
                vuint32_t PCSIS1:1;
                vuint32_t PCSIS0:1;
                  vuint32_t:1;
                vuint32_t MDIS:1;
                vuint32_t DIS_TXF:1;
                vuint32_t DIS_RXF:1;
                vuint32_t CLR_TXF:1;
                vuint32_t CLR_RXF:1;
                vuint32_t SMPL_PT:2;
                  vuint32_t:7;
                vuint32_t HALT:1;
            } B;
        } MCR;                         /* Module Configuration Register */

        uint32_t dspi_reserved1;

        union {
            vuint32_t R;
            struct {
                vuint32_t SPI_TCNT:16;
                  vuint32_t:16;
            } B;
        } TCR;                         /* Transfer Count Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t DBR:1;
                vuint32_t FMSZ:4;
                vuint32_t CPOL:1;
                vuint32_t CPHA:1;
                vuint32_t LSBFE:1;
                vuint32_t PCSSCK:2;
                vuint32_t PASC:2;
                vuint32_t PDT:2;
                vuint32_t PBR:2;
                vuint32_t CSSCK:4;
                vuint32_t ASC:4;
                vuint32_t DT:4;
                vuint32_t BR:4;
            } B;
        } CTAR[8];                     /* Clock and Transfer Attributes Registers */

        union {
            vuint32_t R;
            struct {
                vuint32_t TCF:1;
                vuint32_t TXRXS:1;
                  vuint32_t:1;
                vuint32_t EOQF:1;
                vuint32_t TFUF:1;
                  vuint32_t:1;
                vuint32_t TFFF:1;
                  vuint32_t:5;
                vuint32_t RFOF:1;
                  vuint32_t:1;
                vuint32_t RFDF:1;
                  vuint32_t:1;
                vuint32_t TXCTR:4;
                vuint32_t TXNXTPTR:4;
                vuint32_t RXCTR:4;
                vuint32_t POPNXTPTR:4;
            } B;
        } SR;                          /* Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TCF_RE:1;
                  vuint32_t:2;
                vuint32_t EOQF_RE:1;
                vuint32_t TFUF_RE:1;
                  vuint32_t:1;
                vuint32_t TFFF_RE:1;
                vuint32_t TFFF_DIRS:1;
                  vuint32_t:4;
                vuint32_t RFOF_RE:1;
                  vuint32_t:1;
                vuint32_t RFDF_RE:1;
                vuint32_t RFDF_DIRS:1;
                  vuint32_t:16;
            } B;
        } RSER;                        /* DMA/Interrupt Request Select and Enable Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CONT:1;
                vuint32_t CTAS:3;
                vuint32_t EOQ:1;
                vuint32_t CTCNT:1;
#if defined(CFG_MPC5516) || defined(CFG_MPC5517)
                vuint32_t:4;
  			    vuint32_t PCS5:1;
				vuint32_t PCS4:1;
				vuint32_t PCS3:1;
#elif defined(CFG_MPC560X) || defined(CFG_MPC563XM)
                  vuint32_t:7;
#else
#error CPU run with this header file
#endif
                vuint32_t PCS2:1;
                vuint32_t PCS1:1;
                vuint32_t PCS0:1;
                vuint32_t TXDATA:16;
            } B;
        } PUSHR;                       /* PUSH TX FIFO Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t RXDATA:16;
            } B;
        } POPR;                        /* POP RX FIFO Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TXCMD:16;
                vuint32_t TXDATA:16;
            } B;
        } TXFR[4];                     /* Transmit FIFO Registers */

        vuint32_t DSPI_reserved_txf[12];

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t RXDATA:16;
            } B;
        } RXFR[4];                     /* Transmit FIFO Registers */

        vuint32_t DSPI_reserved_rxf[12];

#if defined(CFG_MPC5516) || defined(CFG_MPC5517)

        union {
                   vuint32_t R;
                   struct {
                       vuint32_t MTOE:1;
                         vuint32_t:1;
                       vuint32_t MTOCNT:6;
                         vuint32_t:4;
                       vuint32_t TXSS:1;
                       vuint32_t TPOL:1;
                       vuint32_t TRRE:1;
                       vuint32_t CID:1;
                       vuint32_t DCONT:1;
                       vuint32_t DSICTAS:3;
                         vuint32_t:6;
                       vuint32_t DPCS5:1;
                       vuint32_t DPCS4:1;
                       vuint32_t DPCS3:1;
                       vuint32_t DPCS2:1;
                       vuint32_t DPCS1:1;
                       vuint32_t DPCS0:1;
                   } B;
               } DSICR;                /* DSI Configuration Register */

               union {
                   vuint32_t R;
                   struct {
                       vuint32_t:16;
                       vuint32_t SER_DATA:16;
                   } B;
               } SDR;                  /* DSI Serialization Data Register */

               union {
                   vuint32_t R;
                   struct {
                       vuint32_t:16;
                       vuint32_t ASER_DATA:16;
                   } B;
               } ASDR;                 /* DSI Alternate Serialization Data Register */

               union {
                   vuint32_t R;
                   struct {
                       vuint32_t:16;
                       vuint32_t COMP_DATA:16;
                   } B;
               } COMPR;                /* DSI Transmit Comparison Register */

               union {
                   vuint32_t R;
                   struct {
                       vuint32_t:16;
                       vuint32_t DESER_DATA:16;
                   } B;
               } DDR;                  /* DSI deserialization Data Register */
#endif

    };                                 /* end of DSPI_tag */
#endif /* IP_DSPI_H_ */
