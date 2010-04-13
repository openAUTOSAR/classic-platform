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
 *  If printing more than the limit, e.g. using vsnprintf() then
 *  the emit function will only stop printing, but not interrupted
 *  (The code will get more complicated that way)
 */

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

//#define HOST_TEST	1

#ifdef HOST_TEST
#define _STDOUT 	stdout
#define _STDIN 	stdin
#define _STDERR	stderr
#else
#define _STDOUT 	(FILE *)STDOUT_FILENO
#define _STDINT 	STDIN_FILENO
#define _STDERR	(FILE *)STDERR_FILENO
#endif



int print(FILE *file, char **buffer, size_t n, const char *format, va_list ap);

int printf(const char *format, ...) {
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vfprintf(_STDOUT, format, ap);
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
	return vfprintf(_STDOUT, format, ap);
}

int vsprintf(char *buffer, const char *format, va_list ap) {
	return vsnprintf(buffer, ~(size_t)0, format, ap);
}

int vfprintf(FILE *file, const char *format, va_list ap) {
	int rv;
	/* Just print to _STDOUT */
	rv = print(file,NULL,~(size_t)0, format,ap);
	return rv;
}

int vsnprintf(char *buffer, size_t n, const char *format, va_list ap) {
	int rv;

	rv = print(NULL, &buffer, n, format,ap);
	return rv;
}


/**
 *
 * @param file  The file to print to
 * @param buf   The buffer to print to
 * @param c		The char to print
 * @return
 */
static inline int emitChar( FILE *file, char **buf, char c, int *left ) {
	if( (*left) == 1 ) {
		return 1;
	}
	--(*left);
	if( buf == NULL ) {
#if HOST_TEST
		putc(c, _STDOUT);
		fflush(_STDOUT);
#else
		arc_putchar(file,c);
#endif
	} else {
		**buf = c;
		(*buf)++;
	}
	return 1;
}


#if defined(HOST_TEST)
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

void emitInt( FILE *file, char **buffer, int base, int width, int flags, va_list ap, int *left )
{
	char lBuf[12];	// longest is base 10, 2^32
	char *str = lBuf;
	int val;

	if( flags & FL_TYPE_SIGNED_INT ) {
		val = (int )va_arg( ap, int );
		xtoa(val,str,base ,(val < 0));
	} else {
		xtoa((unsigned)va_arg( ap, int ),str,base ,0);
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

	while ( (ch = *format++) ) {

		if (ch == '%') {
			ch = *format++;

			if( ch == '%') {
				emitChar(file,buffer,ch,&left);
				continue;
			}

			/* Find flags */
			switch (ch) {
			case '0':
				flags = FL_ZERO;
				break;
			case ' ':
				flags = FL_SPACE;
				break;
			case '-':
				flags = FL_ALIGN_LEFT;
				break;
			default:
				/* Not supported or no flag */
				flags = FL_NONE;
				format--;
				break;
			}

			ch = *format++;

			/* Width */
			if( (ch >= '0')  && (ch <= '9') ) {
				width = ch -'0';
				ch = *format++;
			} else {
				width = 0;
			}

			/* Find type */
			switch (ch) {
			case 'c':
				emitChar(file,buffer,(char )va_arg( ap, int ),&left);
				break;
			case 'd':
				flags |= FL_TYPE_SIGNED_INT;
				emitInt(file,buffer,10,width,flags,ap,&left);
				break;
			case 'u':
				flags |= FL_TYPE_UNSIGNED_INT;
				emitInt(file,buffer,10,width,flags,ap,&left);
				break;
			case 'x':
				flags |= FL_TYPE_UNSIGNED_INT;
				emitInt(file,buffer,16,width,flags,ap,&left);
				break;
			case 's':
				str = (char *)va_arg( ap, int );

				if( str == NULL ) {
					str = "(null)";
				}

				emitString(file,buffer,str,width,flags,&left);
				break;
			default:
				assert(0); // oops
				break;
			}

		} else {
			flags = FL_NONE;
			emitChar(file,buffer,ch,&left);
		}
	}
	va_end(ap);
	if(buffer!=0) {
		left = 0;
		emitChar(file,buffer,'\0',&left);
	}
	return 0; // Wrong.. but for now.
}

#if defined(HOST_TEST)
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

	fprintf(_STDOUT, "string: %s = foobar \n", "foobar");
	sprintf(buff, "string: %s = foobar \n", "foobar");
	printf("%s",buff);

	snprintf(buff,10, "%s\n", "12345678901234567");
	printf("\"123456789\" = \"%s\"\n",buff);

	snprintf(buff,12, "%s\n", "abcdefghijklmn");
	printf("\"abcdefghijkl\" = \"%s\"\n",buff);

	return 0;
}
#endif


