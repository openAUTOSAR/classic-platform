
MEMORY
{
  page0 (rwx) : ORIGIN = 0x0, LENGTH = 256

  /* RAM */
  data (rwx)  : ORIGIN = 0x2000, LENGTH = 8k

  eeprom (rx): ORIGIN = 0x0400, LENGTH = 3k
  text (rx)  : ORIGIN = 0x4000, LENGTH = 48k
  
  /* high fixed bank, reserve 0x100 vectors and security. */
  text_h (rx)  : ORIGIN = 0xc000, LENGTH = 16k-0x100
  vectors (rx)  : ORIGIN = 0xff80, LENGTH = 0x80
  
  /* Flash memory banks VMA:s */
  bank8  (rx)   : ORIGIN = 0x0f0000, LENGTH = 16k
  bank9  (rx)   : ORIGIN = 0x0f4000, LENGTH = 16k
  bank10 (rx)   : ORIGIN = 0x0f8000, LENGTH = 16k
  bank11 (rx)   : ORIGIN = 0x0fc000, LENGTH = 16k
  bank12 (rx)   : ORIGIN = 0x100000, LENGTH = 16k
  bank13 (rx)   : ORIGIN = 0x104000, LENGTH = 16k
  
  bank14 (rx)   : ORIGIN = 0x108000, LENGTH = 16k
  bank15 (rx)   : ORIGIN = 0x10c000, LENGTH = 16k-0x100
  
  /* Flash memory banks LMA:s */
  bank8_lma  (rx)   : ORIGIN = 0x388000, LENGTH = 16k
  bank9_lma  (rx)   : ORIGIN = 0x398000, LENGTH = 16k
  bank10_lma (rx)   : ORIGIN = 0x3a8000, LENGTH = 16k
  bank11_lma (rx)   : ORIGIN = 0x3b8000, LENGTH = 16k
  bank12_lma (rx)   : ORIGIN = 0x3c8000, LENGTH = 16k
  bank13_lma (rx)   : ORIGIN = 0x3d8000, LENGTH = 16k

  bank14_lma (rx)   : ORIGIN = 0x3e8000, LENGTH = 16k
  bank15_lma (rx)   : ORIGIN = 0x3f8000, LENGTH = 16k-0x100
  vectors_lma (rx)  : ORIGIN = 0x3fff80, LENGTH = 0x80
}

/* Setup the stack on the top of the data memory bank.  */
PROVIDE (_stack = 0x4000);


