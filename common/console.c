/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/* ----------------------------[includes]------------------------------------*/

 /*
 *
 * Questions:
 *  - Should I use the library functions ( getc(), printf() ) to write/read
 *    characters from the terminal?
 *
 * Serial_xxx.c
 *   _Read(data,nbytes)
 *   _Write(data,nbytes)
 *   _Init()
 *
 * In the bottom of printf()...
 *   emitChar(....) - That should be buffered in printf.c
 *
 *   Now is:
 *     arc_putchar(file,char)
 *
 *   But should be
 *     - What device are we using right now... that means that somewhere in startup
 *       we do Console_SetDevice(device).
 *
 *       write() will use the the fileno as "device".
 *
 *
 *  So how does it work:
 *  fprintf( stream , ...  )
 *    fileNo = fileno(stream);
 *    write( fileNo, ... )
 *
 *
 *  Since the stream pointer is defined by each clib we must use fdopen()
 *  to get the proper stream pointer...
 *  So a mySP = fdopen(RAMLOG_FILENO, "r") will produce a stream pointer that can
 *  be used when later fprintf(mySP,... ). In the lower layers fileno() will get
 *  back the fileno/filedescriptor.
 *
 *  Assume that we keep the existing fputs(.., FILE *)... that means write( .. )
 *  will be called... then write() will have to do the write depending on device.
 *
 *  On CW you have no control over read()/write()/.... instead you have:
 *    UARTError WriteUARTN(const void* buf, unsigned long cnt)
 *    UARTError ReadUARTN(void* bytes, unsigned long length)
 *    UARTError ReadUART1(char* c)
 *
 *  Soo, for CW that means...problems?
 *
 *
 * Console <-> Vs Serial device.
 *   If we are on a console we can't hang on fgetc() or simular but we must
 *   first check for input and then read it with fgetc()/fgets()... so we need
 *   a system "any key hit" function.
 *
 *   This means unless we have an "event" system that can trigger for example fgets()
 *   to read we need to:
 *     if( testchar(stdio) ) {
 *        c = fgetc(..);
 *     }
 *
 *   Implement fgets()... this no idle CPU anymore. Shell to be periodic or event based?+
 *
 *
 *
 * fd = open("/serial/serial_t32",O_RDWR);
 * write(fd, "hejsan", 5);
 * file = fdopen(fd,"w");
 * fputs("hello",file);
 *
 *
 * file = fopen("serial_t32","r");
 * fputs("hello",file);
 *
 * !! Somewhere we must map a device with a file descriptor !!
 *
 * Use case: Set "global" console
 *   Console_SetDevice("/serial/serial_t32");
 *   fputs("hello",stdout);
 *   Console_SetDevice("/ramlog");
 *   fputs("hello",stdout);
 *
 *
 * Use case: Set console for a specific thing (for example telnet)
 *   This should set local configuration ( not global )
 *     Console_SetDevice("/serial/lwip");
 *     fputs("hello",stdout);
 *
 *
 *
 * It seems that we must assume that read() is blocking until all the characters
 * are read. This means that we must check if a character is present in some way.
 * There is select() but it's just to much....this leaves us having to have a
 * kbhit() function that goes all the way to the driver.
 *
 *
 * http://www.kegel.com/dkftpbench/nonblocking.html
 * http://pubs.opengroup.org/onlinepubs/7908799/xsh/read.html
 *
 * Sooo.... read() should always support O_NONBLOCK
 *
 *
 */

#include "device_serial.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/



typedef struct {
	uint8_t type;
} DeviceType;

typedef struct {
	uint32_t defaultDevice; /* How do I set this? */
} ConsoleType;


DeviceType *devicePtr;


/* List of devices */
DeviceSerialType deviceList[] = {
	&T32_Device,
	NULL,
};




int Console_SetDevice( const char *device)  {
	DeviceSerialType *devPtr;

	devPtr = Serial_FindDevice( device );
	if ( devPtr != 0 ) {
		Sys.ConsoleDevicePtr = devPtr;
	}
}

int Console_Init( void ) {
	/* Set default console device */
	Console_SetDevice("serial_dbg_t32");
}


int getc( FILE *file ) {
	Sys.ConsoleDevicePtr->read( )
	return fgetc(file);
}

int arc_putchar( int c, FILE *file ) {

}

/**
 * Write to device
 *
 * @param str
 * @param file
 * @return
 */
int fputs ( const char * str, FILE *file ) {
	int h = (int)file;
	deviceList[h]->write(str,strlen(str);
}

int fgetc( FILE *file ) {
	int h = (int)file;
	uint8_t data;

	/* get character from device */
	deviceList[h]->read(&data,1);
	return data;
}

int Device_Add( DeviceType *devPtr ) {

}

#if defined(CFG_TTY_T32)
#include "serial_t32.h"
#endif

extern DeviceSerialType

/**
 * In a common file
 */
void Serial_Register( void ) {

	Serial

	dev->init(dev->data);
}




int usage( void ) {
	DeviceType dev;

	Serial_Init( void );

#if defined(CFG_SERIAL_T32)
	dev.read = T32_Read;
	dev.write = T32_Write;
	Console_SetDevice(dev);
#endif

#if defined(USE_SERIAL_SCI)
	dev.read = Sci_Read;
	dev.write = T32_Write;
	Console_SetDevice(dev);
#endif



}
