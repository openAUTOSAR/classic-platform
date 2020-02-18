/*
 * tlb_init.c
 *
 *  Created on: 11 feb 2011
 *      Author: jcar
 */

#include "Std_Types.h"
#include "Cpu.h"
#include "mpc55xx.h"

/* size definitions */
#define SIZE_SHIFT  8
#define SIZE_1KB    (0x0<<SIZE_SHIFT)
#define SIZE_4KB    (0x1<<SIZE_SHIFT)
#define SIZE_16KB   (0x2<<SIZE_SHIFT)
#define SIZE_64KB   (0x3<<SIZE_SHIFT)
#define SIZE_256KB  (0x4<<SIZE_SHIFT)
#define SIZE_1MB    (0x5<<SIZE_SHIFT)
#define SIZE_4MB    (0x6<<SIZE_SHIFT)
#define SIZE_16MB   (0x7<<SIZE_SHIFT)
#define SIZE_64MB   (0x8<<SIZE_SHIFT)
#define SIZE_256MB  (0x9<<SIZE_SHIFT)

/* protection bits */
#define SUPER_RD    0x01
#define USER_RD     0x02
#define SUPER_WR    0x04
#define USER_WR     0x08
#define SUPER_EX    0x10
#define USER_EX     0x20

/* protection aliases */
#define FULL_USER   (USER_RD|USER_WR|USER_EX)
#define FULL_SUPER  (SUPER_RD|SUPER_WR|SUPER_EX)
#define FULL_ACCESS (FULL_USER|FULL_SUPER)

/* wigme attributes */
#define WRITE_THROUGH    0x10
#define INHIBITED        0x08
#define MEMORY_COHERENCY 0x04
#define GUARDED          0x02
#define LITTLE_ENDIAN    0x01

#define LAST_ENTRY 0xffffffff

struct tlb_entry {
    uint32_t base;
    uint32_t size;
    uint32_t prot;
    uint32_t wimge;
};


#define TMP_ENTRY 15

static struct tlb_entry tlb[] = {
   {0x00000000, SIZE_16MB, FULL_ACCESS, INHIBITED|GUARDED},         /* FLASH */
   {0xFFF00000,  SIZE_1MB,   FULL_SUPER,  INHIBITED|GUARDED},
   {0xC3F00000,  SIZE_1MB,   FULL_SUPER,  INHIBITED|GUARDED},
   {0x40000000, SIZE_64KB, FULL_ACCESS,  MEMORY_COHERENCY}, /* cached ram */
   {0x40010000, SIZE_4KB, FULL_ACCESS,  MEMORY_COHERENCY},
   {0x40011000, SIZE_4KB, FULL_ACCESS,  MEMORY_COHERENCY},
   {0x40012000, SIZE_4KB, FULL_ACCESS,  MEMORY_COHERENCY},
   {0x40013000, SIZE_4KB, FULL_ACCESS,  INHIBITED|GUARDED}, /*non cached*/
   {LAST_ENTRY, 0,0,0}                                       /* no more entries */
};


#define VALID      (1<<31)
#define NOT_VALID  (0<<31)
#define TS0        (0<<12)
#define TS1        (1<<12)
#define ESEL_SHIFT 16
#define TLB1       (0x1<<28)


static void
asm_write_tlb(uint32_t mas0, uint32_t mas1, uint32_t mas2, uint32_t mas3)
{
   set_spr(624,mas0);
   set_spr(625,mas1);
   set_spr(626,mas2);
   set_spr(627,mas3);
#if defined __GNUC__
   __asm__ __volatile__(".long 0x7C0007A4");
#elif defined __DCC__
   __asm(".long 0x7C0007A4");
#elif defined __ghs__
   __asm(" tlbwe");
#else
#error Unsupported compiler
#endif
}

static void
write_tlb1_entry(int entry, int ts, uint32_t base, uint32_t size, uint32_t prot, uint32_t wimge)
{
    asm_write_tlb(TLB1|(entry<<ESEL_SHIFT), /* mas0 */
          VALID|ts|size,            /* mas1 */
          (base&0xfffff000)|wimge,  /* mas2 */
          (base&0xfffff000)|prot);  /* mas3 */
}

static void
invalidate_tlb1_entry(int entry)
{
    asm_write_tlb(TLB1|(entry<<ESEL_SHIFT), NOT_VALID, 0, 0);
}

static void
set_l1csr0(uint32_t value)
{
   sync();
   set_spr(1010,value);
   sync();
}

static uint32_t
get_l1csr0(void)
{
   uint32_t ret;
   sync();
   ret = get_spr(1010);
   sync();
   return ret;
}

#define L1CSRX_CE         0x00000001 /* L1 Cache Enabled */
#define L1CSRX_CFI        0x00000002 /* L1 Cache Flash Invalidate */

void
init_mmu(void)
{
    uint32 msr;
    int n;

    /* switch to ts == 1 if ts == 0 else bail*/
    msr = get_msr();
    if (msr & (MSR_IS|MSR_DS)) {
    return ;
    }

    /* setup a valid ts1 entry and switch ts*/
    write_tlb1_entry(TMP_ENTRY, TS1, 0x00000000, SIZE_256MB, FULL_SUPER, 0);
    write_tlb1_entry(TMP_ENTRY+1, TS1, 0x40000000, SIZE_256KB, FULL_SUPER, 0);
    sync();
    set_msr(msr | MSR_IS | MSR_DS);
    isync();

    for (n = 0; tlb[n].base != LAST_ENTRY; n++) {
        write_tlb1_entry(n, TS0, tlb[n].base, tlb[n].size, tlb[n].prot, tlb[n].wimge);
    }
    /* invalidate the rest of the entries */
    for (; n < 16; n++) {
        if (n != TMP_ENTRY) {
            invalidate_tlb1_entry(n);
        }
    }
    /* switch back to ts == 0 */
    sync();
    set_msr(msr);
    isync();
    invalidate_tlb1_entry(TMP_ENTRY);
    invalidate_tlb1_entry(TMP_ENTRY+1);
    sync();

     /* invalidate cache */
     set_l1csr0(L1CSRX_CFI);
     while (get_l1csr0() & L1CSRX_CFI)
     {
        ;
     }
     /* enable */
     set_l1csr0(L1CSRX_CE);
}
