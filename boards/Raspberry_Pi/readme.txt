This file specifies several changes for merging Raspberry Pi code to AUTOSAR4.0 code


1. boards/board_common.mk
   
   #Raspberry Pi, ARM processor, V6 architecture, 
   obj-$(CFG_ARM_V6) += crt0.o
   obj-$(CFG_ARM_V6) += led.o
   obj-$(CFG_ARM_V6) += bcm2835.o
   obj-$(CFG_ARM_V6) += Uart.o
   
   #for Pi_PWM
   obj-$(USE_PWM)-$(CFG_ARM_V6) += Pwm.o

   #for Pi_CAN
   obj-$(USE_SPI)-$(CFG_ARM_V6) += mcp2515.o 

2. scripts/rules.mk
   for output the .bin file. 
   
   # bin output
   $(build-bin-y): $(build-exe-y)
   @echo
   @echo "  >> OBJCOPY $@"   
   $(Q)$(CROSS_COMPILE)objcopy -O binary $< $@	

3. include/Os.h
   add these pieces of lines
   #if !defined(CFG_MPC5516)
   #define GetCoreID() 0
   #endif
   
4. system/kernel/isr.c
   in function void *Os_Isr( void *stack, int16_t vector )
   
   #if defined(CFG_HCS12D) || defined(CFG_ARM_V6)   // you will find it!
   	 isrPtr->constPtr->entry();
   #else
	 Irq_Enable();
	 isrPtr->constPtr->entry();
	 Irq_Disable();
   #endif
   
5: include/arm/Cpu.h

   #elif defined(CFG_ARM_V6)
   #include "bcm2835.h"
   
  
6. if you find there are hundreds unsolved MACROs, do not worry. There is a possible solution that is 
   <1> Open Eclipse preferences: “Window→Preferences”.
   <2> Navigate the left tree into “C/C++→Code Analysis” and disable everything, without doing this you will 
       see a lot of Errors and Warnings from the static analyzer mixed to the errors found by the compiler and that is very confusing.
   <3> Navigate the left tree into “C/C++→New CDT Project Wizard→Makefile Project” then select the “Discovery Options” tab. 
       Select “Automatic discovery of paths and symbols” and then “GCC per project scanner info profile”. 
       This will enable Eclipse CDT to automatically find all the projects paths when the project is first compiled.
   GOOD LUCK!
    

7. for Lwip