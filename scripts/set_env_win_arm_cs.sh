#!/bin/sh
  
UNAME=`uname`
echo $UNAME

#if expr match "$UNAME" 'MINGW32_NT-.*' > 6 ]; then
#  compiler=/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi-
#elif if expr match "$UNAME" 'CYGWIN_NT_NT-.*'; then
#  compiler=/cygdrive/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi- 
#else
#  echo No default path for $UNAME set. Quitting.
#fi 


if test x${UNAME:0:7} = xMINGW32; then
  compiler=/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi-
elif test x${UNAME:0:9} = xCYGWIN_NT; then
  compiler=/cygdrive/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi- 
else
  echo No default path for $UNAME set. Quitting.
fi 

#if test x$UNAME = xMINGW32_NT-5.1; then
#  compiler=/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi-
#elif test x$UNAME = xCYGWIN_NT-5.1; then
#  compiler=/cygdrive/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi- 
#else
#  echo No default path for $UNAME set. Quitting.
#fi 
  
if [ "$compiler" != "" ]; then   
  export CROSS_COMPILE=$compiler
  export BOARDDIR=et_stm32_stamp
  export BDIR=examples/tiny
  echo $CROSS_COMPILE
  echo $BOARDDIR
  echo $BDIR
fi
  
