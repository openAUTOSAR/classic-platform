/*
 * TODO: Should keep the names for the different arch on same name.
 * On PPC this is Mcu_excepions.c and on ARM it's in startup_XXX.s
 */


	.section .text
_bad_int:
	b _bad_int

/* Arg , reverse order, 0xFF80-> */
 	.section	.vectors,"a",%progbits
 	.space 0x30,0			/* 0xff80 */
 	.short  _bad_int		/* 0xffb0 */
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int		/* 0xffc0 */
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int		/* 0xffd0 */
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int		/* 0xffe0 */
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int     	/* 0xfff0 */
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _bad_int
 	.short  _start

