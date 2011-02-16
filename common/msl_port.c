/*
 * msl_port.c
 *
 *  Created on: 14 feb 2011
 *      Author: mahi
 */

/*
Methods called by MW MSL libraries to perform console IO:
*/


int  InitializeUART(void)
{
	return 0;
}

int ReadUARTN( char* buf, int cnt )
{
	(void)buf;
	(void)cnt;
	return 0;
}

int WriteUARTN( char* buf, int cnt )
{
	(void)buf;
	(void)cnt;
	return 0;
}


#if 0
void __init_hardware(void)
{
}


void __flush_cache(register void *address, register unsigned int size)
{
	(void)address;
	(void)size;

}

void __init_user(void)
{

}
#endif


void exit(int exit ) {
	(void)exit;
}


void *sbrk(int inc )
{
	/* We use our own malloc */
	return (void *)(-1);
}
