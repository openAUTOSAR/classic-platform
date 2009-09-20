#!/bin/sh
  
UNAME=`uname`
echo $UNAME
if test x$UNAME = xMINGW32_NT-5.1; then
  compiler=/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi-
elif test x$UNAME = xCYGWIN_NT-5.1; then
  compiler=/cygdrive/c/devtools/Codesourcery/arm-2009q1/bin/arm-none-eabi- 
else
  echo No default path for $UNAME set. Quitting.
fi 
  
if [ "$compiler" != "" ]; then   
  export CROSS_COMPILE=$compiler
  export BOARDDIR=et_stm32_stamp
  export BDIR=examples/tiny
  echo $CROSS_COMPILE
  echo $BOARDDIR
  echo $BDIR
fi
  
