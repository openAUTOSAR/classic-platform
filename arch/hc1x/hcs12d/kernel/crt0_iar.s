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
; Entries:      _start   Entry point
;
;----------------------------------------------------------------------


; ----------------------------------------------------------------------
;
; The starting point.  The CSTARTUP module defines the default
; entry symbol _start which is the starting point of program
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
	extern	irq_pwm_shutdown
	extern	irq_ptpif
	extern	irq_can4_tx
	extern	irq_can4_rx
	extern	irq_can4_err
	extern	irq_can4_wake
	extern	irq_can3_tx
	extern	irq_can3_rx
	extern	irq_can3_err
	extern	irq_can3_wake
	extern	irq_can2_tx
	extern	irq_can2_rx
	extern	irq_can2_err
	extern	irq_can2_wake
	extern	irq_can1_tx
	extern	irq_can1_rx
	extern	irq_can1_err
	extern	irq_can1_wake
	extern	irq_can0_tx
	extern	irq_can0_rx
	extern	irq_can0_err
	extern	irq_can0_wake
	extern	irq_flash
	extern	irq_eeprom
	extern	irq_spi2
	extern	irq_spi1
	extern	irq_iic
	extern	irq_bdlc
	extern	irq_selfclk_mode
	extern	irq_pll_lock
	extern	irq_accb_overflow
	extern	irq_mccnt_underflow
	extern	irq_pthif
	extern	irq_ptjif
	extern	irq_atd1
	extern	irq_atd0
	extern	irq_sci1
	extern	irq_sci0
	extern	irq_spi0
	extern	irq_acca_input
	extern	irq_acca_overflow
	extern	irq_timer_overflow
	extern	irq_tc7
	extern	irq_tc6
	extern	irq_tc5
	extern	irq_tc4
	extern	irq_tc3
	extern	irq_tc2
	extern	irq_tc1
	extern	irq_tc0
	extern	irq_rtii
	extern	irq_irq
	extern	irq_xirq
	extern	irq_swi
	extern	irq_illegal
	extern	irq_cop_clock

	asegn	INTVEC:CONST:ROOT,0ff8cH
?vectors
	dc16	irq_pwm_shutdown
	dc16	irq_ptpif
	dc16	irq_can4_tx
	dc16	irq_can4_rx
	dc16	irq_can4_err
	dc16	irq_can4_wake
	dc16	irq_can3_tx
	dc16	irq_can3_rx
	dc16	irq_can3_err
	dc16	irq_can3_wake
	dc16	irq_can2_tx
	dc16	irq_can2_rx
	dc16	irq_can2_err
	dc16	irq_can2_wake
	dc16	irq_can1_tx
	dc16	irq_can1_rx
	dc16	irq_can1_err
	dc16	irq_can1_wake
	dc16	irq_can0_tx
	dc16	irq_can0_rx
	dc16	irq_can0_err
	dc16	irq_can0_wake
	dc16	irq_flash
	dc16	irq_eeprom
	dc16	irq_spi2
	dc16	irq_spi1
	dc16	irq_iic
	dc16	irq_bdlc
	dc16	irq_selfclk_mode
	dc16	irq_pll_lock
	dc16	irq_accb_overflow
	dc16	irq_mccnt_underflow
	dc16	irq_pthif
	dc16	irq_ptjif
	dc16	irq_atd1
	dc16	irq_atd0
	dc16	irq_sci1
	dc16	irq_sci0
	dc16	irq_spi0
	dc16	irq_acca_input
	dc16	irq_acca_overflow
	dc16	irq_timer_overflow
	dc16	irq_tc7
	dc16	irq_tc6
	dc16	irq_tc5
	dc16	irq_tc4
	dc16	irq_tc3
	dc16	irq_tc2
	dc16	irq_tc1
	dc16	irq_tc0
	dc16	irq_rtii
	dc16	irq_irq
	dc16	irq_xirq
	dc16	irq_swi
	dc16	irq_illegal
	dc16	_start
	dc16	irq_cop_clock
	dc16	_start


;----------------------------------------------------------------------
;
; The starting point of execution
;
;----------------------------------------------------------------------

        rseg	CODE:REORDER:NOROOT(0)
	public	_start

        extern  ?cmain

_start:
;;
;; At this step, the stack is not initialized and interrupts are masked.
;; Applications only have 64 cycles to initialize some registers.
;;
;; To have a generic/configurable startup, initialize the stack to
;; the end of some memory region.  The _stack symbol is defined by
;; the linker.
;;
		//movb #0x39,0x0010 ; INITRM - map RAM
		//movb #0x00,0x0011 ; INITRG - map regs
		//movb #0x09,0x0012 ; INITEE - map EEPROM

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

	jmp	?cmain
        end	_start


;----------------------------------------------------------------------
;
; File:         cmain.s12
;
; Archive:      $Id: cmain.s12 1831 2008-04-08 17:34:04Z svn $
;
; Created:      10/Jun/2004 IHAT
;
; Description:  This module contains the M68HCS12 startup routines that
;		should not be modified by the end user, do that in
;		cstartup.s12 instead.
;
; Entries:      _start   Entry point
;
; Defines:      __BANKED_MODEL__  Define it if you are using a banked system
;
;----------------------------------------------------------------------

; Macro for selecting jsr or call

xcall	macro	destination
#ifdef __BANKED_MODEL__
	call	.lwrd.(destination),.byt3.(destination)
#else
	jsr	destination
#endif
	endm

#define segment_code rseg CODE:CODE:NOROOT(0)


        module ?cmain

        extern  __low_level_init ; Initialize your hardware here
        extern  main             ; Where to begin execution
        extern  exit             ; Where to go when done


;----------------------------------------------------------------------
;
; Catch jump from cstartup.s12
;
;----------------------------------------------------------------------

	segment_code
	public	?cmain
	require	?call_main
?cmain

;----------------------------------------------------------------------
;
; Call __low_level_init to perform initialization before initializing
; segments and calling main. If the function returns 0 no segment
; initialization should take place. Note that low_level_init is
; assumed to be non-banked below.
;
; Link with your own version of __low_level_init to override the
; default action: to do nothing but return 1.
;
;----------------------------------------------------------------------

	segment_code
	public	?cstart_call_low_level_init

?cstart_call_low_level_init:
	xcall	__low_level_init
        tstb
        beq     ?skip_segment_init


;----------------------------------------------------------------------
;
; Segment initialization by copying initialized ROM:ed bytes to
; shadow RAM and clear uninitialized variables.
;
;----------------------------------------------------------------------

	segment_code
	public	?cstart_init_zero
	public	?cstart_init_copy
	extern	__segment_init

?cstart_init_zero
?cstart_init_copy
	xcall	__segment_init

	segment_code
?skip_segment_init


;----------------------------------------------------------------------
;
; Initialize global EC++/C++ objects
;
; ----------------------------------------------------------------------

	rseg	DIFUNCT:CONST		; forward declaration of segment
	segment_code
	public	?cstart_call_ctors
	extern	__call_ctors

?cstart_call_ctors
	ldy	#sfb(DIFUNCT)
	ldd	#sfe(DIFUNCT)
	xcall	__call_ctors


;----------------------------------------------------------------------
;
; Call main() with no arguments, use different calling conventions
; depending on the chosen memory model. Special treatment if you are
; using a banked system.
;
;----------------------------------------------------------------------

	segment_code
	public	?call_main
?call_main
	xcall	main

;----------------------------------------------------------------------
;
; Now when we are ready with our C program we must perform a system-
; dependent action. In this simple case we jump to exit
;
;----------------------------------------------------------------------

        xcall   exit

#ifndef _CLIB_BUILD
; Go to _exit in case exit returns (which it should not)
	extern	_exit
	jmp	_exit
#endif

        end


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
