//Compliant, reported
(void)ConfigPtr; /*lint !e920 MISRA:STANDARDIZED_INTERFACE:<OPTIONAL PUT HERE SOME EXPLANATION>:[MISRA 2012 Rule 1.3, required] */

//Compliant, reported
/*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 1.3, required] */
(void)ConfigPtr;

//Compliant, reported
/*lint -e{533}  MISRA:FALSE_POSITIVE:PUT HERE SOME GOOD ARGUMENTS TO CONVINCE YOUR SAFETY MANAGER:[MISRA 2012 Rule 17.4, mandatory] */
function call (...) {
...code...
}

//Compliant, but not reported
/*lint -e{960} LINT:FALSE_POSITIVE:<OPTIONAL PUT HERE SOME EXPLANATION> */
Htmss_Instance.isInitiated = TRUE;

//Compliant, but not reported
/*lint -e754 LINT:HARDWARE_ACESSS:Something with the HW */
MCU.RW_REG = 1;

//Compliant, but not reported
/*lint --e{754} LINT:OTHER:HARDWARE DEFINITION */
struct bla {
  uint32 bla;
}

//Compliant, not reported
/*lint -save -e754 LINT:OTHER:The warning is caused by external library */
..code....
/*lint -restore */

//Compliant, reported
/*lint -esym(9003, Crc_32P4_Tab) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */
#if Crc_32P4_Mode == CRC_32P4_TABLE
static const uint32 Crc_32P4_Tab[] = {  0x00000000U, 0x30850FF5U, 0x610A1FEAU, 0x518F101FU, 0xC2143FD4U, 0xF2913021U, 0xA31E203EU, 0x939B2FCBU,

//Compliant, reported
/*lint -save -esym(48,10.1) MISRA:FALSE_POSITIVE:bad type:[MISRA 2012 Rule 10.1, required] */
..code....
/*lint -restore */

//Compliant, reported
/*lint -esym(9003,MMU_PageTable) MISRA:OTHER:Since MMU_PageTable is moved to section it can't be in block scope:[MISRA 2012 Rule 8.9, advisory] */
__balign(0x4000) __attribute__ ((section (".mmu_table"))) uint32 MMU_PageTable[PAGE_TABLE_CNT];

//Compliant, but not reported
/*lint --e{715} LINT:OTHER:suppress "k not referenced" */
/*lint --e{732} LINT:OTHER:suppress "loss of sign" */

Not compliant MISRA report:
    /*lint -e553  STANDARD DEV SELECT. Undefined preprocessor variable, assumed 0
     * [MISRA 2004 Rule 19.11, required], [MISRA 2012 Rule 20.9, required]*/
 
Decision: 1. Only MISRA 2012 is reported: [MISRA 2012 Rule 20.9, required]
 
Compliant MISRA report:
    /*lint -e553 MISRA:STANDARDIZED_INTERFACE:Undefined preprocessor variable, assumed 0:[MISRA 2012 Rule 20.9, required]
	
Not compliant MISRA report:
    /*lint -save -e923 HARDWARE_ACESSS. Cast from unsigned int to pointer [Encompasses MISRA 2004 Rule 11.1, required],
     * [MISRA 2004 Rule 11.3, advisory], [MISRA 2012 Rule 11.1, required], [MISRA 2012 Rule 11.4, advisory],
     * [MISRA 2012 Rule 11.6, required]*
 
Decision: 1. Only MISRA 2012 is reported: [MISRA 2012 Rule 11.1, required], [MISRA 2012 Rule 11.6, required], [MISRA 2012 Rule 11.4, advisory]
          2. Required is more severe as advisory: [MISRA 2012 Rule 11.1, required], [MISRA 2012 Rule 11.6, required]
          3. 11.6 seems to more relevant: [MISRA 2012 Rule 11.6, required]
 
 Compliant MISRA report:
    /*lint -e923 MISRA:HARDWARE_ACCESS:Cast from unsigned int to pointer:[MISRA 2012 Rule 11.6, required]	
	
Not Compliant MISRA report:
    /*lint -e923 MISRA:HARDWARE_ACESSS:Cast from unsigned int to pointer:[MISRA 2012 Rule 11.6, required]	
	
	/*lint --e{754} LINT:OTHER:HARDWARE DEFINITION */
struct TAUJ_reg {
	 struct a {
		 vuint8_t  R;
		 uint8_t   _pad[3];
	 } TAUJnCMURm[4];
}

 */
/*lint --e{923} MISRA :HARDWARE ACCESS::[MISRA 2012 Rule 11.6, required]*/
GIO_RegisterType *GPIO_ports1[] = { GIO_PORTA_BASE, GIO_PORTB_BASE, GIO_PORTC_BASE };

        /** @req 4.1.2|4.3.0/SWS_Gpt_00006 - The function Gpt_Init shall initialize the hardware timer module according to a configuration set referenced by ConfigPtr. */
        /*lint -e64 MISRA:FALSE POSITIVE:Configuration is of same type:[MISRA 2012 Rule 1.3, mandatory]*/
    	Gpt_Global.config = ConfigPtr;

		/**
 * Checks that it is possible to identify CPU and core. If not, this function will hang.
 ==============================================
File:			Mcu_Arc_Common_ppc.c
Line:			141
PC_Lint:		[718]
MISRA:			2012:Rule:17.3
Level:			mandatory
Category:		WARNING: Nothing found
Justification:	N/A

Comment: pvr = get_spr(SPR_PVR);  /*lint !e718 !e746 !e548 !e632 !e732 Defined in Cpu.h */
==============================================
 * @return
 */
		    pvr = get_spr(SPR_PVR);  /*lint !e718 !e746 !e548 !e632 !e732 Defined in Cpu.h */

			Compliant MISRA report:
			/*lint -e718 MISRA:FALSE POSITIVE:Ask your safety manager:[MISRA 2012 Rule 17.3, mandatory] */
			/*lint -e746 LINT:OTHER:Check it out */
			/*lint -e548 LINT:OTHER:Check it out */
			/*lint -e632 LINT:OTHER:Check it out */
			/*lint -e732 LINT:OTHER:Check it out */
			/*lint -e{533}  MISRA:FALSE_POSITIVE:PUT HERE SOME GOOD ARGUMENTS TO CONVINCE YOUR SAFETY MANAGER:[MISRA 2012 Rule 17.4, mandatory] */
			pvr = get_spr(SPR_PVR);