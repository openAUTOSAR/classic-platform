
OSAL Design Description for |mcu|
==============================================
 
Document Information
======================

.. list-table:: Document Information
  :widths: 10 10 10 10 10
  :header-rows: 1
  :align: left

  * - Document No
    - Responsible
    - Approver
    - Access Rights
    - Release 
  * - 14-05-569-18010 
    - Marten Hildell 
    - 
    - Project
    - |release|


Document History
===================

History
-----------------

.. list-table:: Document History
  :widths: 10 30 20 40
  :header-rows: 1
  :align: left

  * - Version
    - Author
    - Date
    - Description
  * - 0.1
    - mahi 
    - 2019-04-04
    - First draft
  * - 1.0
    - mahi 
    - 2019-04-10
    - First draft


.. [PPC-EREF] EREF: A Programmer’s Reference Manual for Freescale Power Architecture Processors, EREF_RM Rev. 1 (EIS 2.1) 06/2014

.. [PPC-E500ABI] PowerPC™ e500 Application Binary Interface User’s Guide


.. only:: mpc5744p
   
   .. [PPC-E200ZX] e200z4 Power Architecture™ Core Reference Manual

.. only:: mpc560x

  .. [PPC-E200ZX] e200z1 Power Architecture Core Regerence Manual, 200z1RM, Rev. 0, 09/2008



Introduction 
==============

This document describes the OSAL layer for |mcu|. The document starts with hardware architecture to get a background to the more technical information.
That is followed by the design of the OSAL.

Limitations
==============
No known limitations. 
 

Hardware Architecture
==================================================

This chapter gives a brief overview of the e200 core and gives a background to the following chapters. 

The technical reference manuals for the e200 start from a high description level and down to the more device specific documents.

Example for MPC5744P (that contains a e200z4 core):

- EREF: A Programmer’s Reference Manual for Freescale Power Architecture Processors
- e200z4 Power Architecture™ Core Reference Manual
- The RM for the MPC5744P



EREF and e200 core
------------------------------------------------------------

Registers
^^^^^^^^^^^^^^^^

Core Registers
""""""""""""""""""""""

The processor registers are as follows:


.. list-table:: Register
  :widths: 10 30 20
  :header-rows: 1
  :align: left

  * - Registers
    - Volatility
    - Description
  * - R0
    - Dedicated 
    - Stack Pointer
  * - R1
    - Dedicated 
    - Stack Pointer
  * - R2
    - Dedicated 
    - Small data pointer 2
  * - R3-R12	
    - Volatile	
    - General Purpose
  * - R13	
    - Dedicated	
    - Small data pointer
  * - R14-R31	
    - Nonvolatile	
    - General Purpose
  * - CR	
    - Mixed	
    - Condition register
  * - LR (SPR 8)	
    - Volatile	
    - Program Counter
  * - CTR (SPR 9)	
    - Volatile	
    - Count register
  * - XER (SPR 1)	
    - Volatile	
    - Integer exception register
  * - SPEFSCR	
    - Limited-access	
    - SIMD/FPU status and control
  * - ACC	
    - Volatile	
    - SPE accumulator

		
Volatile - Registers NOT preserved by function call
Nonvolatile - Register that should be preserved by function.


For more information on this see [PPC-E500ABI]_


.. only:: mpc5744p 

   Floating Point and SIMD extensions
   """""""""""""""""""""""""""""""""""""""""""""""
   
   This is rather complex topic for historical reasons, so only e200z4 will be covered here.
   
   The e200z4 have an EFPU2 that implements scalar and vector single-precision operations. 
   Vector operations will use 64-bit GPR and scalar operations 32-bit GPR.
   
   The SIMD (SPE) instructions uses the 64-bit GPR
   
.. only:: mpc560x
   
   Floating Point
   """""""""""""""""""""""""""""""""""""""""""""""
   
   The e200z1 does not have a floating point unit.
   
   
Machine State Register (MSR)
"""""""""""""""""""""""""""""""""""

The machine state register defines the state of the CPU. It can be manipulated only by special instructions. Most notable bits:


.. list-table:: Register
  :widths: 10 10 80
  :header-rows: 1
  :align: left

  * - Number
    - Name
    - Description
  * - 16	
    - EE	
    - External Interrupt Enable
      0 - Interrupts disabled
      1- Interrupts enabled.
  * - 17	
    - PR	
    - Problem State
      0 - CPU is in supervisor mode
      1 - CPU is in user mode


Special-Purpose Registers (SPRs)
"""""""""""""""""""""""""""""""""""

There a lot of SPR that is used for various purposes (MMU, core timers, etc). The register covered here are the ones related to the OS (exception handling and MMU)


.. list-table:: SPR registers
  :widths: 10 30
  :header-rows: 1
  :align: left

  * - Register
    - Description
  * - SRR0
    - Save/restore register 0 - On non-critical interrupts,
      stores either the address of the instruction causing the exception.
  * - SRR1	
    - Save/restore register 1 - On non-critical interrupts, MSR is saved here.
  * - CSRR0	
    - Same as for SRR0 but for critical interrupts.
  * - CSRR1	
    - Same as for SRR1 but for critical interrupts.
  * - MCSRR0	
    - Same as for SRR0 but for machine check interrupts.
  * - MCSRR1	
    - Same as for SRR1 but for machine check interrupts.
  * - MAS0-MAS6	
    - MMU assist registers. Used to update the MMU.

Processor Modes
^^^^^^^^^^^^^^^^
The CPU always execute in either supervisor or user mode. Supervisor mode is the privileged mode and user mode is the unprivileged mode.

Instruction set
^^^^^^^^^^^^^^^^
Two instruction sets exist, the traditional 32-bit instructions (Book-E) and the more size optimized VLE. CPU's that implements only Book-E, both Book-E and VLE and only VLE exist.

.. only:: mpc560x
   The mpc560x implements only the VLE instruction set.


Exception Handling
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There will be no details here about the exception handling more than the exception table and for easy reference.
For more information see [PPC-E200ZX]_ and the implementation in this document.

.. list-table:: Exceptions
  :widths: 10 30 60
  :header-rows: 1
  :align: left

  * - Number
    - Short Name
    - Description
  * - 0	
    - Critical input
    - Implementation specific interrupt. eNMI on some CPUs
  * - 1	
    - Machine check	
    - Something bad happened during execution of the last instruction (no external termination on the bus, etc)
  * - 2	
    - Data storage	
    - Related to access control
  * - 3	
    - Instruction storage	
    - Unaligned instruction fetch, etc.
  * - 4	
    - External input	
    - External input to the CPU. Normally an interrupt controller is connected to this pin.
  * - 5
    - Alignment	
    - Violation of a specific instructions alignment requirements.
  * - 6	
    - Program	
    - Caused by illegal instruction, privilege violation, etc
  * - 8	
    - System Call	
    - Invoked when a system call instruction is called.
  * - 13	
    - Data TLB error
    - Data translation lookup miss
  * - 14	
    - Instruction TLB error	
    - Instruction translation lookup miss


Exception Entry/Exit
"""""""""""""""""""""""""""""""""""

When the CPU takes an exception the address of the instruction causing the exception and the MSR register will be saved to interrupts registers. The interrupt registers are either non-critical (SRR0/SRR1), critical (CSRR0/CSRR1) or machine check (MCSRR0/MCSRR1) depending on the exception and on the state of the MSR[ME] and MSR[CE] bits.

Exit from en exception is done through an instruction where the interrupt registers are restored to PC and MSR.

For more information see [PPC-E200ZX]_



OSAL design for core e200 and Interrupt Controller
==================================================

CPU modes
---------------------------

The mode for Tasks and ISR2 depend on the configuration:


.. list-table:: Register
  :widths: 10 30 30
  :header-rows: 1
  :align: left

  * - Type
    - SC1 and SC2
    - SC3 and SC4
  * - Startup
    - When the CPU comes out of reset it does so in supervisor mode. The system initialization (until the first task is swapped in) will be done in supervisor mode.
    - See SC1 and SC2 
  * - Task	
    - supervisor mode
    - Tasks that belong to an Application that is:

      - Trusted: supervisor mode.
      -  Not-Trusted: user mode.

  * - ISR2
    - supervisor mode	
    - ISR2 that belong to an Application that is:

       - Trusted: supervisor mode.
       - Not-Trusted: user mode.

Exception Handling
---------------------------

The exception handling is a little bit different depending on what kind of exception it is.

Some exceptions are part of the normal program execution, i.e. interrupts, "System Call", while some are fatal errors, e.g. "Alignment", "Program" exceptions.

Bringing up the exception table again with more information about how the OS handles the exceptions.

.. list-table:: Register
  :widths: 7 20 10 40
  :header-rows: 1
  :align: left

  * - Name
    - Description
    - Type
    - Protection Hook
  * - 0	
    - Critical input	
    - Fatal	
    - E_OS_PROTECTION_EXCEPTION
  * - 1	
    - Machine check
    - Fatal 
    - E_OS_PROTECTION_MEMORY
  * - 2 
    - Data storage	
    - Fatal 
    - E_OS_PROTECTION_MEMORY
  * - 3	
    - Instruction storage
    - Fatal
    - E_OS_PROTECTION_EXCEPTION
  * - 4	
    - External input
    - Normal
    - 	
  * - 5	
    - Alignment	
    - Fatal	
    - E_OS_PROTECTION_EXCEPTION
  * - 6	
    - Program	
    - Fatal	
    - E_OS_PROTECTION_EXCEPTION
  * - 8	
    - System Call	
    - Normal
    -   	
  * - 13	
    - Data TLB error	
    - Fatal	
    - E_OS_PROTECTION_MEMORY
  * - 14	
    - Instruction TLB error	
    - Fatal	
    -  E_OS_PROTECTION_EXCEPTION


All fatal exceptions calls ShutdownOS(E_OS_PANIC) except when the protection hook is configured and returns PRO_IGNORE.

Interrupt Controller
^^^^^^^^^^^^^^^^^^^^^^^^

The core e200 specifications does not specify what interrupt controller
should be used, however all currently known MCUs from NXP uses the INTC
interrupt controller.

Most notable features of the INTC are:

-  Priority-based preemtive scheduling. 16 priorities supported.
   Priority 0 - lowest priority
   Priority 15 - highest priority 
-  Lots of interrupt sources (700+ for the bigger MCUs)
-  2 different operating modes, software vector mode and hardware vector
   mode.

The implementation uses software vector mode to determine the vector.


Exception and Interrupt Entry/Exit
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


Exception Entry/Exit is partly common for all exceptions and unique for other parts.

Exceptions, common part
"""""""""""""""""""""""""""""""""""

This part is common to all exception handlers and save the the state of
some CPU registers to the stack.

Depending on the exception other actions are taken, e.g. if its a normal
peripheral interrupt it will be handled by the "External Input"
exception handling,


.. list-table:: Register
  :widths: 10 30
  :header-rows: 1
  :align: left

  * - Phase
    - Description
  * - Exception Entry 
    - The Core or OS will: 
    
      - CPU hardware swaps to supervisor mode regardless of previous CPU mode. Return address is save in xRR0 and MSR in xRR1.
      -  The OS push

         -  xSRR0/xSRR1
         -  SPR status registers (LR,CTR,XER).
         -  GPRs registers r0, r3-12 are pushed.

  * - Exception dependent 
    - The behavior depends on the type of exception
  * - Exception Exit      
    - Os pops the following registers from stack

      -  GPRs registers r0, r3-12
      -  SPR status registers (LR,CTR,XER).
      -  xSRR0/xSRR1

      Execute return from exception instruction that restores PC and MSR (from xSRR0 and xSRR1)


External Input, specific part
"""""""""""""""""""""""""""""""""""

External input exception is connected to the interrupt controller, so
all peripheral interrupts comes in this way.

.. list-table:: Exception entry/exit
  :widths: 10 30
  :header-rows: 1
  :align: left

  * - Phase
    - Description
  * - Exception Entry     
    - See Exception
  * - | More registers saved 
      | and call to interrupt 
      | handler

    - -  Registers r14-r31 are pushed.
      -  If NOT nested interrupt, swap to interrupt stack
      -  Call Irq_Entry(stack) with the current stack as argument
  * - Interrupt Controller
    - OS will: 
      -  Read some register in the interrupt controller to get the interrupt vector
      -  Call generic Os_Isr(stack, vector) with current stack and the extracted vector. 
  * - Generic
    - See the general design
  * - Exit
    -  Os pops the following registers from stack 

       -  Registers r14-r31 are pop:ed from stack
       -  Call
       -  Pop R0-R12 from stack  -  xSRR0/xSRR1
       -  Pop CPSR and PC from stack ("rfe" instruction)  -  SPR status registers (LR,CTR,XER).
       -  GPRs registers r0, r3-12 are pushed.

  * - Exception exit
    - See Exception

System Call, specific part
"""""""""""""""""""""""""""""""""""

The System Call is seen as a normal function call for the caller of System Call functions, hence only registers r14-r31 needs to be preserved along with work registers and registers that is needed by the exception itself (SRR0 and SRR1).

MPU
---

.. only:: mpc5744p 
   
   The MPUs used by NXP PowerPC MCUs usually have the same features.
   
   -  24 entry region description table
   -  Range from 1 byte to 4GB
   -  Memory attributes
   -  Process ID aware
   -  etc
   
   The descriptor table consist of 6 instruction regions, 12 data regions
   and 6 entries that are shared between instruction and data.
   
   Access to the different regions are controlled through MAS0 register
   bits SHD and INST.
   
   .. container:: table-wrap
   
      ======== ==========================
      SHD+INST Access
      ======== ==========================
      00       data access
      01       instruction access
      10       Shared, data access
      11       Shared, instruction access
      ======== ==========================
   
   Regions can be overlapping and regions that have granting permissions
   have higher priority over denying access to those regions.
   
   As a template the following should be used:
   
   
   
   .. list-table:: Register
     :widths: 10 30 20 10 30
     :header-rows: 1
     :align: left
   
     * - SHD+INST
       - ESEL
       - #
       - Memory
       - Notes
     * - 01       
       - 0    
       - SX+UX   
       - Flash
       - 
     * - 01       
       - 1    
       - SX+UX   
       - System RAM           
       - TODO: Why would we like to, execute from RAM?
     * - 00       
       - 0    
       - SRW+URW 
       - System RAM, DATA     
       - TODO: Why allow write here?
     * - 00       
       - 1    
       - SRW+URW 
       - System Flash, DATA
       - 
     * - 00       
       - 2    
       - SRW+URW 
       - Peripheral
       - 
     * - 00       
       - 3    
       - SRW+URO 
       - Local RAM
       - 
     * - 00       
       - 4    
       - SRW+URW 
       - Shared BSS
       -
     * - 00       
       - 5    
       - SRW+URW 
       - Shared DATA
       - 
     * - 00       
       - 6    
       - SRW+URW                       
       - Dynamic
       - 
     * - 00       
       - 7    
       - SRW+URW                       
       - Dynamic
       -

.. only:: mpc560x
   
   The MPU in the mpc560x is not located within the e200z1 core, instead it's an external peripheral.
   
   Main features the MPU:
   
   -  8 entry region description table
   -  A region defines a modulo-32 byte space.
   -  Memory attributes
   
   Since the MPU only has 8 entries, it may not fit all applications.
   
   The following abbreviations is used below
   
   - S - Supervisor
   - U - User 
   - R - Read
   - W - Write
   - X - Execute.
   
   Example: SRWX, supervisor will have read + write + exectute access. 
   
   
   .. list-table:: MPU allocation
     :widths: 10 10 20 20 30
     :header-rows: 1
     :align: left
   
     * - Region
       - Attr 
       - Addr Start
       - Addr Stop
       - Notes
     * - 0
       - SRWX
       - 0x0000.0000 
       - 0xffff.ffff
       - Supervisor Read/Write for whole memory space.
     * - 1
       - URX
       - 0x0000.0000
       - per. space. 
       - U Read Only for Flash + RAM
     * - 2
       - URW
       - Shared .data start 
       - Shared .bss stop 
       - Shared .data and .bss
     * - 3
       - URW
       - App .data start [#app]_
       - App .data start [#app]_
       - Application data
     * - 3
       - URW
       - App .bss start [#app]_
       - App .bss start [#app]_
       - Application bss
     * - 5
       - URW
       - Per. start [#per]_ 
       - Per. stop [#per]_ 
       - Peripheral region
     * - 6
       - URW
       - Per. start [#per]_
       - Per. stop [#per]_
       - Peripheral region 
     * - 7
       - URW
       - Per. start [#per]_
       - Per. stop [#per]_
       - Peripheral region 

.. rubric:: Footnotes

.. [#per] Peripheral I/O region
.. [#app] Application region

OSAL design for |mcu|
========================


Timers
------

The system timer is implemented with the STM0 peripheral and the
following hardware resources are reserved by the OS for OSTick.


  ======================================= ====================================================
     Timer                                   Description
  ======================================= ====================================================
  STM0                                    Compare unit 0 is used to generate the interrupt to 
                                          drive the system counter
  ======================================= ====================================================

Memory and MPU
--------------

There is nothing special with the |mcu| in this regard, see `OSAL design for core e200 and Interrupt Controller`_
design chapter.


MPU design
----------

There is nothing special with the |mcu| in this regard, see `OSAL design for core e200 and Interrupt Controller`_.
design chapter.


Exception Handling
------------------

There is nothing special with the |mcu| in this regard, see `OSAL design for core e200 and Interrupt Controller`_
design chapter.





	
	
