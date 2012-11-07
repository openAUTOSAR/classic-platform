;----------------------------------------------------------------------
;
; File:         cstartup.s12
; 
; Archive:      $Id: cstartup.s12 1831 2008-04-08 17:34:04Z svn $
;
; Created:      02/Feb/2004 IHAT
;
; Description:  This module contains the M68HCS12 startup routines and
;               may be tailored to suit special hardware needs.
;		If possible, perform the initialization in the
;		__low_level_init() function at the end instead.
;
; Entries:      __program_start   Entry point
;
;----------------------------------------------------------------------


; ----------------------------------------------------------------------
;
; The starting point.  The CSTARTUP module defines the default
; entry symbol __program_start which is the starting point of program
; execution and the segment part that will require everything
; that is needed.
;
; ----------------------------------------------------------------------

        module crt0


;----------------------------------------------------------------------
;
; CSTACK - The C stack segment.
;
; Please set the stack size in the linker command file (lnkhcs12.xcl).
;
;----------------------------------------------------------------------

; Forward declaration of CSTACK
; segment so that we can use
; it below

        rseg    CSTACK:DATA:REORDER:NOROOT(0)


; ----------------------------------------------------------------------
;
; The reset vector
;
; ----------------------------------------------------------------------

	asegn	INTVEC:CONST:ROOT,0fffeH
?reset
        dc16    __program_start


;----------------------------------------------------------------------
;
; The starting point of execution
;
;----------------------------------------------------------------------

        rseg	CODE:REORDER:NOROOT(0)
	public	__program_start

	require	?reset		; Require the reset vector
        extern  main

__program_start: 
;;
;; At this step, the stack is not initialized and interrupts are masked.
;; Applications only have 64 cycles to initialize some registers.
;;
;; To have a generic/configurable startup, initialize the stack to
;; the end of some memory region.  The _stack symbol is defined by
;; the linker.
;;
		movb #0x39,0x0010 ; INITRM - map RAM
		movb #0x00,0x0011 ; INITRG - map regs
		movb #0x09,0x0012 ; INITEE - map EEPROM

        lds     #sfe(CSTACK)	; Set up the stack pointer


;----------------------------------------------------------------------
;
; Insert your own assembler initialization code here
;
;----------------------------------------------------------------------



;----------------------------------------------------------------------
;
; Jump to the code that performs the rest of the system initialization
; before calling main().
;
;----------------------------------------------------------------------

	jmp	main
        end	__program_start


/**************************************************
 *
 * This module contains the function `__low_level_init', a function
 * that is called before the `main' function of the program.  Normally
 * low-level initializations - such as setting the prefered interrupt
 * level or setting the watchdog - can be performed here.
 *
 * Note that this function is called before the data segments are
 * initialized, this means that this function can't rely on the
 * values of global or static variables.
 *
 * When this function returns zero, the startup code will inhibit the
 * initialization of the data segments.  The result is faster startup,
 * the drawback is that neither global nor static data will be
 * initialized.
 *
 * Copyright 2004 IAR Systems. All rights reserved.
 *
 * $Revision: 1831 $
 *
 **************************************************/

        NAME ?low_level_init_in_assembler

#ifdef __BANKED_MODEL__
        RTMODEL "__code_model", "banked"
#define return rtc
#define return_size 3
#else
        RTMODEL "__code_model", "normal"
#define return rts
#define return_size 2
#endif
        RTMODEL "__rt_version", "1"

        RSEG CSTACK:DATA:REORDER:NOROOT(0)

        EXTERN ?cstart_call_low_level_init

        PUBLIC __low_level_init
        FUNCTION __low_level_init,0203H
        
        RSEG CODE:CODE:REORDER:NOROOT(0)
__low_level_init:
	REQUIRE	?cstart_call_low_level_init

	/*==================================*/
	/*  Initialize hardware.            */
	/*==================================*/
  
	/*==================================*/
	/* Choose if segment initialization */
	/* should be done or not.           */
	/* Return: 0 to omit seg_init       */
	/*         1 to run seg_init        */
	/*==================================*/

	ldd	#0x1

	return

        END
