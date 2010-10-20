;-------------------------------------------------------------------------------
; sys_core.asm
;
; (c) Texas Instruments 2009, All rights reserved.
;

    .text
    .arm

;-------------------------------------------------------------------------------
; Initialize CPU Registers

    .global  _coreInitRegisters_
    .asmfunc

_coreInitRegisters_:

        mov   r0,         lr
        mov   r1,         #0x03D0
        mov   r2,         #0x0000
        mov   r3,         #0x0000
        mov   r4,         #0x0000
        mov   r5,         #0x0000
        mov   r6,         #0x0000
        mov   r7,         #0x0000
        mov   r8,         #0x0000
        mov   r9,         #0x0000
        mov   r10,        #0x0000
        mov   r11,        #0x0000
        mov   r12,        #0x0000
        orr   r13,        r1,     #0x0001
        msr   cpsr_cxsf,  r13
        msr   spsr_cxsf,  r13
        mov   lr,         r0
        mov   r8,         #0x0000
        mov   r9,         #0x0000
        mov   r10,        #0x0000
        mov   r11,        #0x0000
        mov   r12,        #0x0000
        orr   r13,        r1,     #0x0002
        msr   cpsr_c,     r13
        msr   spsr_cxsf,  r13
        mov   lr,         r0
        orr   r13,        r1,     #0x0007
        msr   cpsr_c,     r13
        msr   spsr_cxsf,  r13
        mov   lr,         r0
        orr   r13,        r1,     #0x000B
        msr   cpsr_c,     r13
        msr   spsr_cxsf,  r13
        mov   lr,         r0
        orr   r13,        r1,     #0x0003
        msr   cpsr_c,     r13
        msr   spsr_cxsf,  r13

	.if 1
        fmdrr d0,        r1,     r1
        fmdrr d1,        r1,     r1
        fmdrr d2,        r1,     r1
        fmdrr d3,        r1,     r1
        fmdrr d4,        r1,     r1
        fmdrr d5,        r1,     r1
        fmdrr d6,        r1,     r1
        fmdrr d7,        r1,     r1
        fmdrr d8,        r1,     r1
        fmdrr d9,        r1,     r1
        fmdrr d10,       r1,     r1
        fmdrr d11,       r1,     r1
        fmdrr d12,       r1,     r1
        fmdrr d13,       r1,     r1
        fmdrr d14,       r1,     r1
        fmdrr d15,       r1,     r1
    .endif

        bl    $+4
        bl    $+4
        bl    $+4
        bl    $+4
        bx    r0

    .endasmfunc


;-------------------------------------------------------------------------------
; Initialize Stack Pointers

    .global  _coreInitStackPointer_
    .asmfunc

_coreInitStackPointer_:

        msr   cpsr_c,   #0xD1
        ldr   sp,       fiq
        msr   cpsr_c,   #0xD2
        ldr   sp,       irq
        msr   cpsr_c,   #0xD7
        ldr   sp,       abort
        msr   cpsr_c,   #0xDB
        ldr   sp,       undef
        msr   cpsr_c,   #0xDF
        ldr   sp,       user
        msr   cpsr_c,   #0xD3
        ldr   sp,       svc
        bx    lr

user:   .word 0x08000000+0x00001000
svc:    .word 0x08000000+0x00001000+0x00000100
fiq:    .word 0x08000000+0x00001000+0x00000100+0x00000100
irq:    .word 0x08000000+0x00001000+0x00000100+0x00000100+0x00000100
abort:  .word 0x08000000+0x00001000+0x00000100+0x00000100+0x00000100+0x00000100
undef:  .word 0x08000000+0x00001000+0x00000100+0x00000100+0x00000100+0x00000100+0x00000100

    .endasmfunc


;-------------------------------------------------------------------------------
; Enable VFP Unit

    .global  _coreEnableVfp_
    .asmfunc

_coreEnableVfp_:

	.if 1
        mrc   p15,     #0x00,      r0,       c1, c0, #0x02
        orr   r0,      r0,         #0xF00000
        mcr   p15,     #0x00,      r0,       c1, c0, #0x02
        mov   r0,      #0x40000000
        fmxr  fpexc,   r0
	.endif

        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Enable Event Bus Export

    .global  _coreEnableEventBusExport_
    .asmfunc

_coreEnableEventBusExport_:

        mrc   p15, #0x00, r0,         c9, c12, #0x00
        orr   r0,  r0,    #0x10
        mcr   p15, #0x00, r0,         c9, c12, #0x00
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Disable Event Bus Export

    .global  _coreDisableEventBusExport_
    .asmfunc

_coreDisableEventBusExport_:

        mrc   p15, #0x00, r0,         c9, c12, #0x00
        bic   r0,  r0,    #0x10
        mcr   p15, #0x00, r0,         c9, c12, #0x00
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Enable RAM ECC Support

    .global  _coreEnableRamEcc_
    .asmfunc

_coreEnableRamEcc_:

        mrc   p15, #0x00, r0,         c1, c0,  #0x01
        orr   r0,  r0,    #0x0C000000
        mcr   p15, #0x00, r0,         c1, c0,  #0x01
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Disable RAM ECC Support

    .global  _coreDisableRamEcc_
    .asmfunc

_coreDisableRamEcc_:

        mrc   p15, #0x00, r0,         c1, c0,  #0x01
        bic   r0,  r0,    #0x0C000000
        mcr   p15, #0x00, r0,         c1, c0,  #0x01
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Enable Flash ECC Support

    .global  _coreEnableFlashEcc_
    .asmfunc

_coreEnableFlashEcc_:

        mrc   p15, #0x00, r0,         c1, c0,  #0x01
        orr   r0,  r0,    #0x02000000
        mcr   p15, #0x00, r0,         c1, c0,  #0x01
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Disable Flash ECC Support

    .global  _coreDisableFlashEcc_
    .asmfunc

_coreDisableFlashEcc_:

        mrc   p15, #0x00, r0,         c1, c0,  #0x01
        bic   r0,  r0,    #0x02000000
        mcr   p15, #0x00, r0,         c1, c0,  #0x01
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Enable Offset via Vic controller

    .global  _coreEnableIrqVicOffset_
    .asmfunc

_coreEnableIrqVicOffset_:

        mrc   p15, #0, r0,         c1, c0,  #0
        orr   r0,  r0,    #0x01000000
        mcr   p15, #0, r0,         c1, c0,  #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Get data fault status register

    .global  _coreGetDataFault_
    .asmfunc

_coreGetDataFault_:

        mrc   p15, #0, r0, c5, c0,  #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Clear data fault status register

    .global  _coreClearDataFault_
    .asmfunc

_coreClearDataFault_:

        mov   r0,  #0
        mcr   p15, #0, r0, c5, c0,  #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Get instruction fault status register

    .global  _coreGetInstructionFault_
    .asmfunc

_coreGetInstructionFault_:

        mrc   p15, #0, r0, c5, c0, #1
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Clear instruction fault status register

    .global  _coreClearInstructionFault_
    .asmfunc

_coreClearInstructionFault_:

        mov   r0,  #0
        mcr   p15, #0, r0, c5, c0, #1
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Get data fault address register

    .global  _coreGetDataFaultAddress_
    .asmfunc

_coreGetDataFaultAddress_:

        mrc   p15, #0, r0, c6, c0,  #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Clear data fault address register

    .global  _coreClearDataFaultAddress_
    .asmfunc

_coreClearDataFaultAddress_:

        mov   r0,  #0
        mcr   p15, #0, r0, c6, c0,  #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Get instruction fault address register

    .global  _coreGetInstructionFaultAddress_
    .asmfunc

_coreGetInstructionFaultAddress_:

        mrc   p15, #0, r0, c6, c0, #2
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Clear instruction fault address register

    .global  _coreClearInstructionFaultAddress_
    .asmfunc

_coreClearInstructionFaultAddress_:

        mov   r0,  #0
        mcr   p15, #0, r0, c6, c0, #2
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Get auxiliary data fault status register

    .global  _coreGetAuxiliaryDataFault_
    .asmfunc

_coreGetAuxiliaryDataFault_:

        mrc   p15, #0, r0, c5, c1, #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Clear auxiliary data fault status register

    .global  _coreClearAuxiliaryDataFault_
    .asmfunc

_coreClearAuxiliaryDataFault_:

        mov   r0,  #0
        mcr   p15, #0, r0, c5, c1, #0
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Get auxiliary instruction fault status register

    .global  _coreGetAuxiliaryInstructionFault_
    .asmfunc

_coreGetAuxiliaryInstructionFault_:

        mrc   p15, #0, r0, c5, c1, #1
        bx    lr

    .endasmfunc


;-------------------------------------------------------------------------------
; Clear auxiliary instruction fault status register

    .global  _coreClearAuxiliaryInstructionFault_
    .asmfunc

_coreClearAuxiliaryInstructionFault_:

        mov   r0,  #0
        mrc   p15, #0, r0, c5, c1, #1
        bx    lr

    .endasmfunc
    
    
;-------------------------------------------------------------------------------
; C++ construct table pointers

    .global __TI_PINIT_Base, __TI_PINIT_Limit
    .weak   SHT$$INIT_ARRAY$$Base, SHT$$INIT_ARRAY$$Limit

__TI_PINIT_Base:  .long SHT$$INIT_ARRAY$$Base
__TI_PINIT_Limit: .long SHT$$INIT_ARRAY$$Limit

;-------------------------------------------------------------------------------

