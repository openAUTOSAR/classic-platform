
#include "Can.h"

void Can_MainFunction_Write( void ) {
	// Polled can not supported by this driver.
}
void Can_MainFunction_Read( void ) {
	// Polled can not supported by this driver.
}
void Can_MainFunction_BusOff( void ) {
	// Bus off not supported by this driver.
}
void Can_MainFunction_Wakeup( void ) {
	// Wakeup not supported by this driver.
}




/* Global and Static Variables */

#ifndef _little_endian__
    static const unsigned s_canByteOrder[] = {3U, 2U, 1U, 0U, 7U, 6U, 5U, 4U};
#endif


void canInit(void)
{
/* USER CODE BEGIN (4) */
/* USER CODE END */
    /** @b Initialize @b DCAN1: */

    /** - Setup control register
    *     - Disable automatic wakeup on bus activity
    *     - Local power down mode disabled
    *     - Disable DMA request lines
    *     - Enable global Interrupt Line 0 and 1
    *     - Disable debug mode
    *     - Release from software reset
    *     - Disable parity and ECC
    *     - Enable/Disable auto bus on timer
    *     - Setup message completion before entering debug state
    *     - Setup normal operation mode
    *     - Request write access to the configuration registers
    *     - Setup automatic retransmission of messages
    *     - Disable error interrups
    *     - Disable status interrupts
    *     - Enter initialization mode
    */
    canREG1->CTL = 0x00000000U | 0x00000000U | 0x00021443U;

    /** - Clear all pending error flags and reset current status */
    canREG1->ES = 0x0000031FU;

    /** - Assign interrupt level for messages */
    canREG1->INTMUXx[0U] = 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U;

    canREG1->INTMUXx[1U] = 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U;

    /** - Setup auto bus on timer pewriod */
    canREG1->ABOTR = 0U;

    /** - Initialize message 1 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((1U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 1;

    /** - Initialize message 2 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((2U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 2;

    /** - Initialize message 3 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((3U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 3;

    /** - Initialize message 4 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((4U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 4;

    /** - Initialize message 5 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((5U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 5;

    /** - Initialize message 6 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((6U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 6;

    /** - Initialize message 7 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((7U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 7;

    /** - Initialize message 8 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((8U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 8;

    /** - Initialize message 9 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((9U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 9;

    /** - Initialize message 10 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((10U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 10;

    /** - Initialize message 11 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((11U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 11;

    /** - Initialize message 12 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((12U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 12;

    /** - Initialize message 13 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((13U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 13;

    /** - Initialize message 14 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((14U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 14;

    /** - Initialize message 15 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((15U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 15;

    /** - Initialize message 16 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((16U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 16;

    /** - Initialize message 17 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((17U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 17;

    /** - Initialize message 18 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((18U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 18;

    /** - Initialize message 19 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((19U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 19;

    /** - Initialize message 20 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((20U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 20;

    /** - Initialize message 21 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((21U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 21;

    /** - Initialize message 22 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((22U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 22;

    /** - Initialize message 23 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((23U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 23;

    /** - Initialize message 24 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((24U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 24;

    /** - Initialize message 25 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((25U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 25;

    /** - Initialize message 26 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((26U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 26;

    /** - Initialize message 27 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((27U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 27;

    /** - Initialize message 28 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((28U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 28;

    /** - Initialize message 29 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((29U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 29;

    /** - Initialize message 30 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((30U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 30;

    /** - Initialize message 31 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((31U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 31;

    /** - Initialize message 32 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((32U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 32;

    /** - Initialize message 33 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((33U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 33;

    /** - Initialize message 34 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((34U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 34;

    /** - Initialize message 35 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((35U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 35;

    /** - Initialize message 36 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((36U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 36;

    /** - Initialize message 37 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((37U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 37;

    /** - Initialize message 38 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((38U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 38;

    /** - Initialize message 39 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((39U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 39;

    /** - Initialize message 40 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((40U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 40;

    /** - Initialize message 41 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((41U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 41;

    /** - Initialize message 42 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((42U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 42;

    /** - Initialize message 43 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((43U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 43;

    /** - Initialize message 44 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((44U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 44;

    /** - Initialize message 45 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((45U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 45;

    /** - Initialize message 46 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((46U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 46;

    /** - Initialize message 47 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((47U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 47;

    /** - Initialize message 48 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((48U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 48;

    /** - Initialize message 49 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((49U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 49;

    /** - Initialize message 50 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((50U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 50;

    /** - Initialize message 51 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((51U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 51;

    /** - Initialize message 52 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((52U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 52;

    /** - Initialize message 53 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((53U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 53;

    /** - Initialize message 54 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((54U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 54;

    /** - Initialize message 55 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((55U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 55;

    /** - Initialize message 56 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((56U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 56;

    /** - Initialize message 57 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((57U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 57;

    /** - Initialize message 58 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((58U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 58;

    /** - Initialize message 59 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((59U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 59;

    /** - Initialize message 60 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((60U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 60;

    /** - Initialize message 61 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((61U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 61;

    /** - Initialize message 62 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((62U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 62;

    /** - Initialize message 63 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((63U & 0x1FFFFFFFU) << 0U);
    canREG1->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF1CMD  = 0xF8;
    canREG1->IF1NO   = 63;

    /** - Initialize message 64 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG1->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((64U & 0x1FFFFFFFU) << 0U);
    canREG1->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG1->IF2CMD  = 0xF8;
    canREG1->IF2NO   = 64;

    /** - Setup IF1 for data transmission 
    *     - Wait until IF1 is ready for use 
    *     - Set IF1 control byte
    */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1CMD  = 0x87;

    /** - Setup IF2 for reading data
    *     - Wait until IF1 is ready for use 
    *     - Set IF1 control byte
    */
    while (canREG1->IF2STAT & 0x80);

    canREG1->IF2CMD = 0x17;

    /** - Setup bit timing 
    *     - Setup baud rate prescaler extension
    *     - Setup TSeg2
    *     - Setup TSeg1
    *     - Setup sample jump width
    *     - Setup baud rate prescaler
    */
    canREG1->BTR = (0U << 16U) |
                   ((4U - 1U) << 12U) |
                   (((5U + 4U) - 1U) << 8U) |
                   ((4U - 1U) << 6U) |
                   9U;

    /** - Setup TX pin to functional output */
    canREG1->TIOC = 0x0000004CU;

    /** - Setup RX pin to functional input */
    canREG1->RIOC = 0x00000048U;

    /** - Leave configuration and initialization mode  */
    canREG1->CTL &= ~0x00000041U;

    /** @b Initialize @b DCAN2: */

    /** - Setup control register
    *     - Disable automatic wakeup on bus activity
    *     - Local power down mode disabled
    *     - Disable DMA request lines
    *     - Enable global Interrupt Line 0 and 1
    *     - Disable debug mode
    *     - Release from software reset
    *     - Disable parity and ECC
    *     - Enable/Disable auto bus on timer
    *     - Setup message completion before entering debug state
    *     - Setup normal operation mode
    *     - Request write access to the configuration registers
    *     - Setup automatic retransmission of messages
    *     - Disable error interrups
    *     - Disable status interrupts
    *     - Enter initialization mode
    */
    canREG2->CTL = 0x00000000U | 0x00000000U | 0x00021443U;

    /** - Clear all pending error flags and reset current status */
    canREG2->ES = 0x0000031FU;

    /** - Assign interrupt level for messages */
    canREG2->INTMUXx[0U] = 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U;

    canREG2->INTMUXx[1U] = 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U;


    /** - Setup auto bus on timer pewriod */
    canREG2->ABOTR = 0U;

    /** - Initialize message 1 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((1U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 1;

    /** - Initialize message 2 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((2U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 2;

    /** - Initialize message 3 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((3U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 3;

    /** - Initialize message 4 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((4U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 4;

    /** - Initialize message 5 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((5U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 5;

    /** - Initialize message 6 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((6U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 6;

    /** - Initialize message 7 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((7U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 7;

    /** - Initialize message 8 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((8U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 8;

    /** - Initialize message 9 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((9U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 9;

    /** - Initialize message 10 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((10U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 10;

    /** - Initialize message 11 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((11U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 11;

    /** - Initialize message 12 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((12U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 12;

    /** - Initialize message 13 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((13U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 13;

    /** - Initialize message 14 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((14U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 14;

    /** - Initialize message 15 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((15U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 15;

    /** - Initialize message 16 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((16U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 16;

    /** - Initialize message 17 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((17U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 17;

    /** - Initialize message 18 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((18U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 18;

    /** - Initialize message 19 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((19U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 19;

    /** - Initialize message 20 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((20U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 20;

    /** - Initialize message 21 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((21U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 21;

    /** - Initialize message 22 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((22U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 22;

    /** - Initialize message 23 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((23U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 23;

    /** - Initialize message 24 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((24U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 24;

    /** - Initialize message 25 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((25U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 25;

    /** - Initialize message 26 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((26U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 26;

    /** - Initialize message 27 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((27U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 27;

    /** - Initialize message 28 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((28U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 28;

    /** - Initialize message 29 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((29U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 29;

    /** - Initialize message 30 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((30U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 30;

    /** - Initialize message 31 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((31U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 31;

    /** - Initialize message 32 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((32U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 32;

    /** - Initialize message 33 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((33U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 33;

    /** - Initialize message 34 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((34U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 34;

    /** - Initialize message 35 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((35U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 35;

    /** - Initialize message 36 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((36U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 36;

    /** - Initialize message 37 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((37U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 37;

    /** - Initialize message 38 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((38U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 38;

    /** - Initialize message 39 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((39U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 39;

    /** - Initialize message 40 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((40U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 40;

    /** - Initialize message 41 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((41U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 41;

    /** - Initialize message 42 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((42U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 42;

    /** - Initialize message 43 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((43U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 43;

    /** - Initialize message 44 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((44U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 44;

    /** - Initialize message 45 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((45U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 45;

    /** - Initialize message 46 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((46U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 46;

    /** - Initialize message 47 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((47U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 47;

    /** - Initialize message 48 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((48U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 48;

    /** - Initialize message 49 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((49U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 49;

    /** - Initialize message 50 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((50U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 50;

    /** - Initialize message 51 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((51U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 51;

    /** - Initialize message 52 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((52U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 52;

    /** - Initialize message 53 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((53U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 53;

    /** - Initialize message 54 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((54U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 54;

    /** - Initialize message 55 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((55U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 55;

    /** - Initialize message 56 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((56U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 56;

    /** - Initialize message 57 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((57U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 57;

    /** - Initialize message 58 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((58U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 58;

    /** - Initialize message 59 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((59U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 59;

    /** - Initialize message 60 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((60U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 60;

    /** - Initialize message 61 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((61U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 61;

    /** - Initialize message 62 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((62U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 62;

    /** - Initialize message 63 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((63U & 0x1FFFFFFFU) << 0U);
    canREG2->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF1CMD  = 0xF8;
    canREG2->IF1NO   = 63;

    /** - Initialize message 64 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG2->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((64U & 0x1FFFFFFFU) << 0U);
    canREG2->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG2->IF2CMD  = 0xF8;
    canREG2->IF2NO   = 64;

    /** - Setup IF1 for data transmission 
    *     - Wait until IF1 is ready for use 
    *     - Set IF1 control byte
    */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1CMD  = 0x87;

    /** - Setup IF2 for reading data
    *     - Wait until IF1 is ready for use 
    *     - Set IF1 control byte
    */
    while (canREG2->IF2STAT & 0x80);

    canREG2->IF2CMD = 0x17;

    /** - Setup bit timing 
    *     - Setup baud rate prescaler extension
    *     - Setup TSeg2
    *     - Setup TSeg1
    *     - Setup sample jump width
    *     - Setup baud rate prescaler
    */
    canREG2->BTR = (0U << 16U) |
                   ((4U - 1U) << 12U) |
                   (((5U + 4U) - 1U) << 8U) |
                   ((4U - 1U) << 6U) |
                   9U;

    /** - Setup TX pin to functional output */
    canREG2->TIOC = 0x0000004CU;

    /** - Setup RX pin to functional input */
    canREG2->RIOC = 0x00000048U;

    /** - Leave configuration and initialization mode  */
    canREG2->CTL &= ~0x00000041U;

#if 0
    /** @b Initialize @b DCAN3: */

    /** - Setup control register
    *     - Disable automatic wakeup on bus activity
    *     - Local power down mode disabled
    *     - Disable DMA request lines
    *     - Enable global Interrupt Line 0 and 1
    *     - Disable debug mode
    *     - Release from software reset
    *     - Disable parity and ECC
    *     - Enable/Disable auto bus on timer
    *     - Setup message completion before entering debug state
    *     - Setup normal operation mode
    *     - Request write access to the configuration registers
    *     - Setup automatic retransmission of messages
    *     - Disable error interrups
    *     - Disable status interrupts
    *     - Enter initialization mode
    */
    canREG3->CTL = 0x00000000U | 0x00000000U | 0x00021443U;

    /** - Clear all pending error flags and reset current status */
    canREG3->ES = 0x0000031FU;

    /** - Assign interrupt level for messages */
    canREG3->INTMUXx[0U] = 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U
                         | 0x00000000U;

    /** - Setup auto bus on timer pewriod */
    canREG3->ABOTR = 0U;

    /** - Initialize message 1 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((1U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 1;

    /** - Initialize message 2 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((2U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 2;

    /** - Initialize message 3 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((3U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 3;

    /** - Initialize message 4 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((4U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 4;

    /** - Initialize message 5 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((5U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 5;

    /** - Initialize message 6 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((6U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 6;

    /** - Initialize message 7 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((7U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 7;

    /** - Initialize message 8 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((8U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 8;

    /** - Initialize message 9 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((9U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 9;

    /** - Initialize message 10 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((10U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 10;

    /** - Initialize message 11 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((11U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 11;

    /** - Initialize message 12 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((12U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 12;

    /** - Initialize message 13 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((13U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 13;

    /** - Initialize message 14 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((14U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 14;

    /** - Initialize message 15 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((15U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 15;

    /** - Initialize message 16 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((16U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 16;

    /** - Initialize message 17 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((17U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 17;

    /** - Initialize message 18 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((18U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 18;

    /** - Initialize message 19 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((19U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 19;

    /** - Initialize message 20 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((20U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 20;

    /** - Initialize message 21 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((21U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 21;

    /** - Initialize message 22 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((22U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 22;

    /** - Initialize message 23 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((23U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 23;

    /** - Initialize message 24 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((24U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 24;

    /** - Initialize message 25 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((25U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 25;

    /** - Initialize message 26 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((26U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 26;

    /** - Initialize message 27 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((27U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 27;

    /** - Initialize message 28 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((28U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 28;

    /** - Initialize message 29 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((29U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 29;

    /** - Initialize message 30 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((30U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 30;

    /** - Initialize message 31 
    *     - Wait until IF1 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF1 control byte
    *     - Set IF1 message number
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF1ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((31U & 0x1FFFFFFFU) << 0U);
    canREG3->IF1MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF1CMD  = 0xF8;
    canREG3->IF1NO   = 31;

    /** - Initialize message 32 
    *     - Wait until IF2 is ready for use 
    *     - Set message mask
    *     - Set message control word
    *     - Set message arbitration
    *     - Set IF2 control byte
    *     - Set IF2 message number
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2MSK  = 0xC0000000U | ((0x000007FFU & 0x1FFFFFFFU) << 0U);
    canREG3->IF2ARB  = 0x00000000U | 0x40000000U | 0x20000000U | ((32U & 0x1FFFFFFFU) << 0U);
    canREG3->IF2MCTL = 0x00001080U | 0x00000000U | 8U;
    canREG3->IF2CMD  = 0xF8;
    canREG3->IF2NO   = 32;

    /** - Setup IF1 for data transmission 
    *     - Wait until IF1 is ready for use 
    *     - Set IF1 control byte
    */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1CMD  = 0x87;

    /** - Setup IF2 for reading data
    *     - Wait until IF1 is ready for use 
    *     - Set IF1 control byte
    */
    while (canREG3->IF2STAT & 0x80);

    canREG3->IF2CMD = 0x17;

    /** - Setup bit timing 
    *     - Setup baud rate prescaler extension
    *     - Setup TSeg2
    *     - Setup TSeg1
    *     - Setup sample jump width
    *     - Setup baud rate prescaler
    */
    canREG3->BTR = (0U << 16U) |
                   ((4U - 1U) << 12U) |
                   (((5U + 4U) - 1U) << 8U) |
                   ((4U - 1U) << 6U) |
                   9U;

    /** - Setup TX pin to functional output */
    canREG3->TIOC = 0x0000004CU;

    /** - Setup RX pin to functional input */
    canREG3->RIOC = 0x00000048U;

    /** - Leave configuration and initialization mode  */
    canREG3->CTL &= ~0x00000041U;
#endif

    /**   @note This function has to be called before the driver can be used.\n
    *           This function has to be executed in priviledged mode.\n
    */

/* USER CODE BEGIN (5) */
/* USER CODE END */
}


/** @fn unsigned canTransmit(canBASE_t *node, unsigned messageBox, const unsigned char *data)
*   @brief Transmits a CAN message
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @param[in] data Pointer to CAN TX data
*   @return The function will return:
*           - 0: When the setup of the TX message box wasn't successful   
*           - 1: When the setup of the TX message box was successful   
*
*   This function writes a CAN message into a CAN message box.
*
*/

/* USER CODE BEGIN (6) */
/* USER CODE END */

unsigned canTransmit(canBASE_t *node, unsigned messageBox, const unsigned char *data)
{
    unsigned i;
    unsigned success  = 0U;
    unsigned regIndex = (messageBox - 1U) >> 5U;
    unsigned bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (7) */
/* USER CODE END */

    /** - Check for pending message:
    *     - pending message, return 0
    *     - no pending message, start new transmission 
    */
    if (node->TXRQx[regIndex] & bitIndex)
    {
        return success;
    }

    /** - Wait until IF1 is ready for use */
    while (node->IF1STAT & 0x80);

    /** - Copy TX data into IF1 */
    for (i = 0U; i < 8U; i++)
    {
#ifdef _little_endian__
        node->IF1DATx[i] = *data++;
#else
        node->IF1DATx[s_canByteOrder[i]] = *data++;
#endif
    }

    /** - Copy TX data into mesasge box */
    node->IF1NO = messageBox;

    success = 1U; 

    /**   @note The function canInit has to be called before this function can be used.\n
    *           The user is responsible to initialize the message box.
    */

/* USER CODE BEGIN (8) */
/* USER CODE END */

    return success;
}


/** @fn unsigned canGetData(canBASE_t *node, unsigned messageBox, unsigned char * const data)
*   @brief Gets received a CAN message
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @param[out] data Pointer to store CAN RX data
*   @return The function will return:
*           - 0: When RX message box hasn't received new data   
*           - 1: When RX data are stored in the data buffer   
*           - 3: When RX data are stored in the data buffer and a message was lost   
*
*   This function writes a CAN message into a CAN message box.
*
*/


/* USER CODE BEGIN (9) */
/* USER CODE END */

unsigned canGetData(canBASE_t *node, unsigned messageBox, unsigned char * const data)
{
    unsigned       i;
    unsigned       size;
    unsigned char *pData    = (unsigned char *)data;
    unsigned       success  = 0U;
    unsigned       regIndex = (messageBox - 1U) >> 5U;
    unsigned       bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (10) */
/* USER CODE END */

    /** - Check if new data have been arrived:
    *     - no new data, return 0
    *     - new data, get received message 
    */
    if (!(node->NWDATx[regIndex] & bitIndex))
    {
        return success;
    }

    /** - Wait until IF2 is ready for use */
    while (node->IF2STAT & 0x80);

    /** - Copy data into IF2 */
    node->IF2NO = messageBox;

    /** - Wait until data are copied into IF2 */
    while (node->IF2STAT & 0x80);

    /** - Get number of received bytes */
    size = node->IF2MCTL & 0xFU;

    /** - Copy RX data into destination buffer */
    for (i = 0U; i < size; i++)
    {
#ifdef _little_endian__
        *pData++ = node->IF2DATx[i];
#else
        *pData++ = node->IF2DATx[s_canByteOrder[i]];
#endif
    }

    success = 1U;

    /** - Check if data have been lost:
    *     - no data lost, return 1
    *     - data lost, return 3 
    */
    if (node->IF2MCTL & 0x4000U)
    {
        success = 3U;
    }

    /**   @note The function canInit has to be called before this function can be used.\n
    *           The user is responsible to initialize the message box.
    */

/* USER CODE BEGIN (11) */
/* USER CODE END */

    return success;
}


/** @fn unsigned canIsTxMessagePending(canBASE_t *node, unsigned messageBox)
*   @brief Gets Tx message box transmission status
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @return The function will return the tx request flag
*
*   Checks to see if the Tx message box has a pending Tx request, returns
*   0 is flag not set otherwise will return the Tx request flag itself.
*/


/* USER CODE BEGIN (12) */
/* USER CODE END */

unsigned canIsTxMessagePending(canBASE_t *node, unsigned messageBox)
{
    unsigned flag;
    unsigned regIndex = (messageBox - 1U) >> 5U;
    unsigned bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (13) */
/* USER CODE END */

    /** - Read Tx request reigster */
    flag = node->TXRQx[regIndex] & bitIndex;

/* USER CODE BEGIN (14) */
/* USER CODE END */

    return flag;
}


/** @fn unsigned canIsRxMessageArrived(canBASE_t *node, unsigned messageBox)
*   @brief Gets Rx message box reception status
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @return The function will return the new data flag
*
*   Checks to see if the Rx message box has pending Rx data, returns
*   0 is flag not set otherwise will return the Tx request flag itself.
*/


/* USER CODE BEGIN (15) */
/* USER CODE END */

unsigned canIsRxMessageArrived(canBASE_t *node, unsigned messageBox)
{
    unsigned flag;
    unsigned regIndex = (messageBox - 1U) >> 5U;
    unsigned bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (16) */
/* USER CODE END */

    /** - Read Tx request register */
    flag = node->NWDATx[regIndex] & bitIndex;

/* USER CODE BEGIN (17) */
/* USER CODE END */

    return flag;
}


/** @fn unsigned canIsMessageBoxValid(canBASE_t *node, unsigned messageBox)
*   @brief Chechs if message box is valid
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @return The function will return the new data flag
*
*   Checks to see if the message box is valid for operation, returns
*   0 is flag not set otherwise will return the validation flag itself.
*/


/* USER CODE BEGIN (18) */
/* USER CODE END */

unsigned canIsMessageBoxValid(canBASE_t *node, unsigned messageBox)
{
    unsigned flag;
    unsigned regIndex = (messageBox - 1U) >> 5U;
    unsigned bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (19) */
/* USER CODE END */

    /** - Read Tx request register */
    flag = node->MSGVALx[regIndex] & bitIndex;

/* USER CODE BEGIN (20) */
/* USER CODE END */

    return flag;
}


/** @fn unsigned canGetLastError(canBASE_t *node)
*   @brief Gets last RX/TX-Error of CAN message traffic
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @return The function will return:
*           - canERROR_OK (0): When no CAN error occured   
*           - canERROR_STUFF (1): When a stuff error occured on RX message    
*           - canERROR_FORMAT (2): When a form/format error occured on RX message   
*           - canERROR_ACKNOWLEDGE (3): When a TX message wasn't acknowledged  
*           - canERROR_BIT1 (4): When a TX message monitored dominant level where recessive is expected   
*           - canERROR_BIT0 (5): When a TX message monitored recessive level where dominant is expected   
*           - canERROR_CRC (6): When a RX message has wrong CRC value   
*           - canERROR_NO (7): When no error occured since last call of this function   
*
*   This function returns the last occured error code of an RX or TX message,
*   since the last call of this function.
*
*/


/* USER CODE BEGIN (21) */
/* USER CODE END */

unsigned canGetLastError(canBASE_t *node)
{
    unsigned errorCode;

/* USER CODE BEGIN (22) */
/* USER CODE END */

    /** - Get last error code */
    errorCode = node->ES & 7U;

    /**   @note The function canInit has to be called before this function can be used. */

/* USER CODE BEGIN (23) */
/* USER CODE END */

    return errorCode;
}


/** @fn unsigned canGetErrorLevel(canBASE_t *node)
*   @brief Gets error level of a CAN node
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @return The function will return:
*           - canLEVEL_ACTIVE (0x00): When RX- and TX error counters are below 96   
*           - canLEVEL_WARNING (0x40): When RX- or TX error counter are between 96 and 127     
*           - canLEVEL_PASSIVE (0x20): When RX- or TX error counter are between 128 and 255     
*           - canLEVEL_BUS_OFF (0x80): When RX- or TX error counter are above 255     
*
*   This function returns the current error level of a CAN node.
*
*/


/* USER CODE BEGIN (24) */
/* USER CODE END */

unsigned canGetErrorLevel(canBASE_t *node)
{
    unsigned errorLevel;

/* USER CODE BEGIN (25) */
/* USER CODE END */

    /** - Get error level */
    errorLevel = node->ES & 0xE0U;

    /**   @note The function canInit has to be called before this function can be used. */

/* USER CODE BEGIN (26) */
/* USER CODE END */

    return errorLevel;
}


/** @fn void canEnableErrorNotification(canBASE_t *node)
*   @brief Enable error notification
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*
*   This function will enable the notification for the reaching the error levels warning, passive and bus off.
*/

/* USER CODE BEGIN (27) */
/* USER CODE END */

void canEnableErrorNotification(canBASE_t *node)
{
/* USER CODE BEGIN (28) */
/* USER CODE END */

    node->CTL |= 8U;

    /**   @note The function canInit has to be called before this function can be used. */

/* USER CODE BEGIN (29) */
/* USER CODE END */
}


/** @fn void canDisableErrorNotification(canBASE_t *node)
*   @brief Disable error notification
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*
*   This function will disable the notification for the reaching the error levels warning, passive and bus off.
*/

/* USER CODE BEGIN (30) */
/* USER CODE END */

void canDisableErrorNotification(canBASE_t *node)
{
/* USER CODE BEGIN (31) */
/* USER CODE END */

    node->CTL &= ~8U;

    /**   @note The function canInit has to be called before this function can be used. */

/* USER CODE BEGIN (32) */
/* USER CODE END */
}


/** @fn unsigned canEnableMessageNotification(canBASE_t *node, unsigned messageBox)
*   @brief Enable message notification
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @return The function will return:
*           - 0: When the interrupts are not enable (message is pending)   
*           - 1: When the are enabled   
*
*   This function will enable the notification for sucessfully receiving or transmitting a message.
*/

/* USER CODE BEGIN (33) */
/* USER CODE END */

unsigned canEnableMessageNotification(canBASE_t *node, unsigned messageBox)
{
    unsigned success  = 0U;
    unsigned regIndex = (messageBox - 1U) >> 5U;
    unsigned bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (34) */
/* USER CODE END */

    /** - Check for pending message:
    *     - pending message, return 0
    *     - no pending message, enable interrupts
    */
    if ((node->NWDATx[regIndex] & bitIndex) || (node->TXRQx[regIndex] & bitIndex))
    {
        return success;
    }

    /** - Wait until IF2 is ready for use */
    while (node->IF2STAT & 0x80);

    /** - Copy data into IF2 */
    node->IF2NO = messageBox;

    /** - Wait until data are copied into IF2 */
    while (node->IF2STAT & 0x80);

    /** - Wait until IF1 is ready for use */
    while (node->IF1STAT & 0x80);

    /** - Adapt IF1 command to enable the interrupts */
    node->IF1CMD = 0x90;

    /** - Enable message interrupt */
    node->IF1MCTL = node->IF2MCTL | 0xC00U;

    /** - Copy data into message box */
    node->IF1NO = messageBox;

    /** - Wait until IF1 is ready for use */
    while (node->IF1STAT & 0x80);

    /** - Reset IF1 command to normal operation */
    node->IF1CMD = 0x87;

    success = 1U;

    /**   @note The function canInit has to be called before this function can be used.\n 
    *           The function returns with 0 if the message box has a pending data.
    */

/* USER CODE BEGIN (35) */
/* USER CODE END */

    return success;
}


/** @fn unsigned canDisableMessageNotification(canBASE_t *node, unsigned messageBox)
*   @brief Disable message notification
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*   @return The function will return:
*           - 0: When the interrupts are not enable (message is pending)   
*           - 1: When the are enabled   
*
*   This function will disable the notification for sucessfully receiving or transmitting a message.
*/

/* USER CODE BEGIN (36) */
/* USER CODE END */

unsigned canDisableMessageNotification(canBASE_t *node, unsigned messageBox)
{
    unsigned success  = 0U;
    unsigned regIndex = (messageBox - 1U) >> 5U;
    unsigned bitIndex = 1U << ((messageBox - 1U) & 0x1FU);

/* USER CODE BEGIN (37) */
/* USER CODE END */

    /** - Check for pending message:
    *     - pending message, return 0
    *     - no pending message, enable interrupts
    */
    if ((node->NWDATx[regIndex] & bitIndex) || (node->TXRQx[regIndex] & bitIndex))
    {
        return success;
    }

    /** - Wait until IF2 is ready for use */
    while (node->IF2STAT & 0x80);

    /** - Copy data into IF2 */
    node->IF2NO = messageBox;

    /** - Wait until data are copied into IF2 */
    while (node->IF2STAT & 0x80);

    /** - Wait until IF1 is ready for use */
    while (node->IF1STAT & 0x80);

    /** - Adapt IF1 command to disable the interrupts */
    node->IF1CMD = 0x90;

    /** - Disable message interrupt */
    node->IF1MCTL = node->IF2MCTL & ~0xC00U;

    /** - Copy data into message box */
    node->IF1NO = messageBox;

    /** - Wait until IF1 is ready for use */
    while (node->IF1STAT & 0x80);

    /** - Reset IF1 command to normal operation */
    node->IF1CMD = 0x87;

    success = 1U;

    /**   @note The function canInit has to be called before this function can be used.\n 
    *           The function returns with 0 if the message box has a pending data.
    */

/* USER CODE BEGIN (38) */
/* USER CODE END */

    return success;
}


/** @fn void can1HighLevelInterrupt(void)
*   @brief DCAN1 High Level Interrupt Handler
*/

/* USER CODE BEGIN (39) */
/* USER CODE END */

#if 0
    #pragma INTERRUPT(can1HighLevelInterrupt, IRQ)
#endif

void can1HighLevelInterrupt(void)
{
    unsigned value = canREG1->INT;

/* USER CODE BEGIN (40) */
/* USER CODE END */

    if (value == 0x8000U)
    {
        canErrorNotification(canREG1, canREG1->ES & 0xE0U);
        return;
    }

    /** - Setup IF1 for clear pending interrupt flag */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1CMD = 0x08;
    canREG1->IF1NO  = value;

    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1CMD = 0x87;

    canMessageNotification(canREG1, value);

/* USER CODE BEGIN (41) */
/* USER CODE END */
}


/** @fn void can1LowLevelInterrupt(void)
*   @brief DCAN1 Low Level Interrupt Handler
*/

/* USER CODE BEGIN (42) */
/* USER CODE END */

#if 0
    #pragma INTERRUPT(can1LowLevelInterrupt, IRQ)
#endif

void can1LowLevelInterrupt(void)
{
    unsigned messageBox = canREG1->INT >> 16U;

/* USER CODE BEGIN (43) */
/* USER CODE END */

    /** - Setup IF1 for clear pending interrupt flag */
    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1CMD = 0x08;
    canREG1->IF1NO  = messageBox;

    while (canREG1->IF1STAT & 0x80);

    canREG1->IF1CMD = 0x87;

    canMessageNotification(canREG1, messageBox);

/* USER CODE BEGIN (44) */
/* USER CODE END */
}


/** @fn void can2HighLevelInterrupt(void)
*   @brief DCAN2 High Level Interrupt Handler
*/

/* USER CODE BEGIN (45) */
/* USER CODE END */

#if 0
    #pragma INTERRUPT(can2HighLevelInterrupt, IRQ)
#endif

void can2HighLevelInterrupt(void)
{
    unsigned value = canREG2->INT;

/* USER CODE BEGIN (46) */
/* USER CODE END */

    if (value == 0x8000U)
    {
        canErrorNotification(canREG2, canREG2->ES & 0xE0U);
        return;
    }

    /** - Setup IF1 for clear pending interrupt flag */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1CMD = 0x08;
    canREG2->IF1NO  = value;

    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1CMD = 0x87;

    canMessageNotification(canREG2, value);

/* USER CODE BEGIN (47) */
/* USER CODE END */
}


/** @fn void can2LowLevelInterrupt(void)
*   @brief DCAN2 Low Level Interrupt Handler
*/

/* USER CODE BEGIN (48) */
/* USER CODE END */

#if 0
    #pragma INTERRUPT(can2LowLevelInterrupt, IRQ)
#endif

void can2LowLevelInterrupt(void)
{
    unsigned messageBox = canREG2->INT >> 16U;

/* USER CODE BEGIN (49) */
/* USER CODE END */

    /** - Setup IF1 for clear pending interrupt flag */
    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1CMD = 0x08;
    canREG2->IF1NO  = messageBox;

    while (canREG2->IF1STAT & 0x80);

    canREG2->IF1CMD = 0x87;

    canMessageNotification(canREG2, messageBox);

/* USER CODE BEGIN (50) */
/* USER CODE END */
}


#if 0
/** @fn void phantomInterrupt(void)
*   @brief DCAN3 High Level Interrupt Handler
*/

/* USER CODE BEGIN (51) */
/* USER CODE END */

#if 0
    #pragma INTERRUPT(phantomInterrupt, IRQ)
#endif

void phantomInterrupt(void)
{
    unsigned value = canREG3->INT;

/* USER CODE BEGIN (52) */
/* USER CODE END */

    if (value == 0x8000U)
    {
        canErrorNotification(canREG3, canREG3->ES & 0xE0U);
        return;
    }

    /** - Setup IF1 for clear pending interrupt flag */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1CMD = 0x08;
    canREG3->IF1NO  = value;

    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1CMD = 0x87;

    canMessageNotification(canREG3, value);

/* USER CODE BEGIN (53) */
/* USER CODE END */
}


/** @fn void phantomInterrupt(void)
*   @brief DCAN3 Low Level Interrupt Handler
*/

/* USER CODE BEGIN (54) */
/* USER CODE END */

#if 0
    #pragma INTERRUPT(phantomInterrupt, IRQ)
#endif

void phantomInterrupt(void)
{
    unsigned messageBox = canREG3->INT >> 16U;

/* USER CODE BEGIN (55) */
/* USER CODE END */

    /** - Setup IF1 for clear pending interrupt flag */
    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1CMD = 0x08;
    canREG3->IF1NO  = messageBox;

    while (canREG3->IF1STAT & 0x80);

    canREG3->IF1CMD = 0x87;

    canMessageNotification(canREG3, messageBox);

/* USER CODE BEGIN (56) */
/* USER CODE END */
}
#endif

#endif
