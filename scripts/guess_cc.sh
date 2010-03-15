#/bin/bash
#
# Tries to get a compiler depending on SHELL/OS. On Windows it defaults
# to CodeSourcery installed at c:\devtools\Codesourcery\<arch>
#

UNAME=`uname`
echo $UNAME
echo $1

if [ "$ARCH" == "PPC" ]; then
	if test x${UNAME:0:7} = xMINGW32; then
	  COMPILER=/c/devtools/Codesourcery/freescale-4.3/bin/powerpc-eabi-
	elif test x${UNAME:0:9} = xCYGWIN_NT; then
	  COMPILER=/cygdrive/c/devtools/Codesourcery/freescale-4.3/bin/powerpc-eabi-
	elif test x${UNAME:0:5} = xDarwin; then
	  COMPILER=/opt/powerpc-eabi/bin/powerpc-eabi-
	else
	  echo No default path for $UNAME set. Quitting.
	fi
elif [ "$ARCH" == "ARM" ]; then
	if test x${UNAME:0:7} = xMINGW32; then
	  COMPILER=/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi-
	elif test x${UNAME:0:9} = xCYGWIN_NT; then
	  COMPILER=/cygdrive/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi-
	elif test x${UNAME:0:5} = xDarwin; then
	  COMPILER=/opt/arm-elf/bin/arm-none-eabi-
	else
	  echo No default path for $UNAME set. Quitting.
	fi
elif [ "$ARCH" == "HCS12" ]; then
	COMPILER=/opt/m6812-elf/bin/m6812-elf-
else
	echo baaaad
fi

if [ -f ${COMPILER}gcc ]; then
  echo Using compiler: ${COMPILER}
else
   echo ${COMPILER} does not exist...
   exit 1
fi

export CROSS_COMPILE=$COMPILER

