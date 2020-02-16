/*
 * Copyright ArcCore AB
 *
 * A simple implementation of all formatted xxprintf functionallity.
 *
 * DESIGN CRITERIA:
 *  - Reentrant
 *  - Use little stack
 *
 *  What you normally would do is to print to a buffer of some kind and then
 *  call write(). However little stack indicates that we can't place buffers
 *  on the stack and reentrant tells us that we can't have a static buffer.
 *  That leaves us with printing characters as it is ready. From a speed
 *  point of view that is less than optimal, but that the way it's got to be.
 *  (Could make a 16 long char buffer??)
 *
 *  This is just intended to be used as a replacement for newlibs implementation.
 *  Newlib porting interface have the following API:
 *    int write(  int fd, char *buf, int nbytes)
 *
 *
 *  Note that puts(), putc() are still the newlib variants....
 *
 *    printf()       -> vfprintf(stdout,) -> vsnprintf(buf,)
 *                                           write()
 *    vprintf()      -> vfprintf(stdout,) -> vsnprintf(buf,)
 *                                           write()
 *    sprintf(buf,)  ->                      vsnprintf(buf,)
 *    snprintf(buf,) ->                      vsnprintf(buf,)
 *
 * IMPLEMENTATION NOTE:
 *  - If printing more than the limit, e.g. using vsnprintf() then
 *    the emit function will only stop printing, but not interrupted
 *    (The code will get more complicated that way)
 *  - ANSI-C and POSIX, streams and POSIX filenumbers.
 *    POSIX file-numbers exist in unistd.h and are only to be used by the porting
 *    newlib interface i.e. newlib_port.c.
 *
 *
 * NEWLIB: File handles vs files
 *   This printf() family of functions does not use newlib at all.
 *   At this point the following can have happend:
 *   1. A call to any of the file functions in newlib have been called.
 *      This then calls a number of functions (sbrk, __sinit(_impure_ptr), etc ).
 *      __sinit(_impure_ptr) initializes the standard files, stdin, stdout, stderr.
 *      file->_file is the actual posix file number (stdin=0, stdout=1, stderr=2)
 *   2. No calls is done to newlib file functions. The impure_data is then empty and
 *      all fields are 0.
 *
 *  Code for checking if the newlib have initialized (or we have explicitly called __sinit(_impure_ptr)
 * 	if( _impure_ptr->__sdidinit == 1 ) {
 *	  // We use the real filenumber
 *	  arc_putchar((int)(file->_file), c);
 *	)
 *
 *
 *
 *
 *	http://ubuntuforums.org/showthread.php?t=936816
 *
 *	What we don't want to use:
 *	- Terminal stuff, tcsetattr(),etc..makes everything very complicated.
 *	- Use of select() to set to check for keyboard hit, ie
 *	  select(STDIN_FILENO+1..);
 *	  Then check FD_ISSET( STDIN_FILENO, .. )
 *
 *
 */

#if defined(__IAR_SYSTEMS_ICC__)
#define STDOUT_FILENO	1
#elif defined(__GNUC__) && defined(__DCC__)
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#if defined(USE_NEWLIB) && defined(__GNUC__)
#include "reent.h"
#endif
#include <ctype.h>

//TEMP
#if defined(CFG_ARM_V6)
#include "Uart.h"
#endif

//#define PRINTF_HOST_TEST	1

#if defined(__IAR_SYSTEMS_ICC__)
#endif

int print(FILE *file, char **buffer, size_t n, const char *format, va_list ap);
static inline int emitChar( FILE *file, char **buf, char c, int *left );


int fputc( int c, FILE *file) {
	int fd;
	char ch=(char)c;
	fd = fileno(file);
	write(fd,&ch,1);
	return c;
}


int fputs( const char *s, FILE *file ) {
	while(*s) {
		fputc(*s++,file);
	}
	return 0;
}


/**
 * Get a character from stream. Assume for now
 * that it's blocking.
 *
 *
 * @param file
 * @return
 */
int autosar_fgetc( FILE *file ) {
	char c;
	int fd;
	fd = fileno(file);

	/* Blocking read for now */
	read(fd,&c,1);

	return c;
}


/**
 * Read to EOF or newline
 *
 * @param file
 * @return
 */
int autosar_fgets( char *str, int n, FILE *file ) {
	int fd;
	fd = fileno(file);
	return read(fd,str,n);
}



int printf(const char *format, ...) {
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vfprintf(stdout, format, ap);
	va_end(ap);
	return rv;
}

int fprintf(FILE *file, const char *format, ...) {
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vfprintf(file, format, ap);
	va_end(ap);
	return rv;
}

int sprintf(char *buffer, const char *format, ...) {
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vsnprintf(buffer, ~(size_t)0, format, ap);
	va_end(ap);

	return rv;
}

int snprintf(char *buffer, size_t n, const char *format, ...) {
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vsnprintf(buffer, n, format, ap);
	va_end(ap);
	return rv;
}

int vprintf(const char *format, va_list ap) {
	return vfprintf(stdout, format, ap);
}

int vsprintf(char *buffer, const char *format, va_list ap) {
	return vsnprintf(buffer, ~(size_t)0, format, ap);
}

int vfprintf(FILE *file, const char *format, va_list ap) {
	int rv;
	/* Just print to stdout */
	rv = print(file,NULL,~(size_t)0, format,ap);
	return rv;
}


int vsnprintf(char *buffer, size_t n, const char *format, va_list ap) {
	int rv;

	rv = print(NULL, &buffer, n, format,ap);
	return rv;
}


/*
 * The integer only counterpart
 */
#if defined(USE_NEWLIB)
int iprintf(const char *format, ...) __attribute__ ((alias("printf")));
int fiprintf(FILE *file, const char *format, ...) __attribute__ ((alias("fprintf")));
int siprintf(char *buffer, const char *format, ...) __attribute__ ((alias("sprintf")));
int sniprintf(char *buffer, size_t n, const char *format, ...) __attribute__ ((alias("snprintf")));
int viprintf(const char *format, va_list ap) __attribute__ ((alias("vprintf")));
int vsiprintf(char *buffer, const char *format, va_list ap) __attribute__ ((alias("vsprintf")));
int vfiprintf(FILE *file, const char *format, va_list ap) __attribute__ ((alias("vfprintf")));	
#endif

/**
 *
 * @param file  The file to print to
 * @param buf   The buffer to print to
 * @param c		The char to print
 * @return
 */
static inline int emitChar( FILE *file, char **buf, char c, int *left ) {
	(void)file;

	if( (*left) == 1 ) {
		return 1;
	}
	--(*left);
	if( buf == NULL ) {
#if PRINTF_HOST_TEST
		putc(c, stdout);
		fflush(stdout);
#else
//TEMP-#if
#if defined(CFG_ARM_V6)
		mini_uart_send(c);
#else
		putc(c,stdout);
		#endif
#endif /* PRINTF_HOST_TEST */
	} else {
		**buf = c;
		(*buf)++;
	}
	return 1;
}


#if defined(PRINTF_HOST_TEST)
/**
 * Convert a number to a string
 *
 * @param val		The value to convert
 * @param str		Pointer to a space where to put the string
 * @param base		The base
 * @param negative	If negative or not.
 */
void xtoa( unsigned long val, char* str, int base, int negative) {
	int i;
	char *oStr = str;
	char c;

	if (negative) {
		val = -val;
	}

	if( base < 10 && base > 16 ) {
		*str = '0';
		return;
	}
    i = 0;

    do {
      str[i++] = "0123456789abcdef"[ val % base ];
	} while ((val /= base) > 0);


    if (negative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    str = &str[i]-1;
    while(str > oStr) {
    	c = *str;
    	*str-- = *oStr;
    	*oStr++=c;
    }
}
#else
extern void xtoa( unsigned long val, char* str, int base, int negative);
#endif


#define FL_NONE					(0<<0)
#define FL_ZERO					(1<<1)
#define FL_HASH					(1<<2)
#define FL_SPACE				(1<<3)
#define FL_ALIGN_LEFT			(1<<4)
#define FL_TYPE_SIGNED_INT		(1<<5)
#define FL_TYPE_UNSIGNED_INT	(1<<6)
#define FL_TYPE_POINTER			(1<<7)

static void emitString( FILE *file, char **buffer, char *string, int width, int flags, int *left) {
	char pad;
	char *str = string;
	int i;
	int strLen;
	/* padding: FL_ZERO or normal ' '
	 * align: FL_ALIGN_LEFT (left) or normal (right)
	 */
	pad = (flags & FL_ZERO) ? '0' : ' ';


	if( flags & FL_ALIGN_LEFT ) {
		for(i=0;str[i]; i++) {
			emitChar(file,buffer,str[i],left);
		}

		/* Pad the rest */
		for(;i<width;i++) {
			emitChar(file,buffer,pad,left);
		}
	} else {

		strLen = strlen(string);

		/* Pad first  */
		if( width > strLen ) {
			for(i=0;i<(width-strLen);i++) {
				emitChar(file,buffer,pad,left);
			}
		}

		for(i=0;i<strLen; i++) {
			emitChar(file,buffer,string[i],left);
		}
	}
}

void emitInt( FILE *file, char **buffer, int val, int base, int width, int flags, int *left )
{
	char lBuf[12];	// longest is base 10, 2^32
	char *str = lBuf;

	if( flags & FL_TYPE_SIGNED_INT ) {
		xtoa(val,str,base ,(val < 0));
	} else {
		xtoa((unsigned)val,str,base ,0);
	}

	emitString(file,buffer,str,width,flags,left);
}

#define PRINT_CHAR(_c)  *buffer++= (_c);


/**
 * Write formatted output to an array with a maximum number of characters.
 *
 * This is the mother of the formatted print family. The main goal here
 * is to be very small and memory efficient.
 *
 * Support:
 *   Parameter: None
 *   Flags    : '-' and '0'
 *   Width    : Normal padding is supported, '*' is not.
 *   Precision: None
 *   Length   : None
 *   C99      : None
 *   Type     : d,u,x,s,and c
 *
 * @param file    The file descriptor
 * @param buffer  A pointer to the place to store the output
 *                If NULL the output is instead
 * @param n       The maximum number of characters to write
 * @param format  The format string
 * @param ap      The va list
 * @return
 */
int print(FILE *file, char **buffer, size_t n, const char *format, va_list ap)
{
	char ch;
	int flags;
	char *str;
	int width;
	int left = n;
	char wBuff[4];

	while ( (ch = *format++) ) {

		if (ch == '%') {
			ch = *format++;

			if( ch == '%') {
				emitChar(file,buffer,ch,&left);
				continue;
			}

			/* Find flags */
			if (ch == '0')
			{
				flags = FL_ZERO;
			}
			else if (ch == ' ')
			{
				flags = FL_SPACE;
			}
			else if (ch == '-')
			{
				flags = FL_ALIGN_LEFT;
			}
			else
			{
				/* Not supported or no flag */
				flags = FL_NONE;
				format--;
			}

			ch = *format++;

			/* Width */
			if( (ch >= '0')  && (ch <= '9') ) {
				int a = 1;
				wBuff[0] = ch;

				while( (*format >= '0')  && (*format <= '9') ) {
					wBuff[a++] = *format++;
				}
				wBuff[a] = '\0';
     		    width = strtoul(wBuff,NULL,10);

				ch = *format++;
			} else {
				width = 0;
			}

			/* Length, "eat" length for now  */
			if( (ch == 'h') || (ch == 'l') || (ch == 'L') ) {
				ch = *format++;
			}

			/* Find type */
			if (ch =='c')
			{
				emitChar(file,buffer,(char )va_arg( ap, int ),&left);
			}
			else if (ch == 'd')
			{
				flags |= FL_TYPE_SIGNED_INT;
				emitInt(file,buffer,va_arg( ap, int ),10,width,flags,&left);
			}
			else if (ch == 'u')
			{
				flags |= FL_TYPE_UNSIGNED_INT;
				emitInt(file,buffer,va_arg( ap, int ),10,width,flags,&left);
			}
			else if (ch == 'x')
			{
				flags |= FL_TYPE_UNSIGNED_INT;
				emitInt(file,buffer,va_arg( ap, int ),16,width,flags,&left);
			}
			else if (ch == 'p')
			{
				flags |= FL_TYPE_POINTER;
				emitInt(file,buffer,va_arg( ap, int ),16,width,flags,&left);
			}
			else if (ch == 's')
			{
				str = (char *)va_arg( ap, int );

				if( str == NULL ) {
					str = "(null)";
				}

				emitString(file,buffer,str,width,flags,&left);
			}
			else
			{
				assert(0); // oops
			}
		} else {
			flags = FL_NONE;
			emitChar(file,buffer,ch,&left);
		}
	}
//	va_end(ap);		// Removed, TODO: Check the va_start/va_end handling (used in calling functions also).
	if(buffer!=0) {
		left = 0;
		emitChar(file,buffer,'\0',&left);
	}
	return 0; // Wrong.. but for now.
}

#if 0
int main(void) {
	char *ptr = NULL;
	char buff[30];

	printf("char: %c %c = a B\n", 'a', 66);

	printf("string: %s = (null)\n", (char *)ptr);

	printf("string: %s = foobar \n", "foobar");

	printf("string: %2s = foobar \n", "foobar");
	printf("string: \"%8s\" = \"  foobar\" \n", "foobar");
	/* Left justify */
	printf("string: \"%-8s\" = \"foobar  \" \n", "foobar");

	printf("decimal:  23 = %d \n", 23);
	printf("hex:     c23 = %x \n", 0xc23);
	printf("hex:    0c23 = %04x \n", 0xc23);
	printf("decimal with blanks:     23 = %6d  \n", 23);
	printf("decimal with zero:   000023 = %06d \n", 23);

	/* negative and large numbers */
	printf("decimal:  -23 = %d \n", -23);
	printf("decimal:  4294967273 = %u \n", -23);
	printf("decimal:  c0000000   = %x \n", 0xc0000000);

	printf("decimal:  00c000   = %06x \n", 0xc000);

	fprintf(stdout, "string: %s = foobar \n", "foobar");
	sprintf(buff, "string: %s = foobar \n", "foobar");
	printf("%s",buff);

	snprintf(buff,10, "%s\n", "12345678901234567");
	printf("\"123456789\" = \"%s\"\n",buff);

	snprintf(buff,12, "%s\n", "abcdefghijklmn");
	printf("\"abcdefghijkl\" = \"%s\"\n",buff);

	return 0;
}
#endif

