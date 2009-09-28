#/bin/bash

function quit {
    echo
    echo "### BUILD FAILED"
    echo 
    exit $1
}

oo="aa $CROSS_COMPILE aa";
echo "oo = $oo";
if [ "aa $CROSS_COMPILE aa" == "aa  aa" ]; then
  os=`uname`
  echo $os
  if [ "$os" == "Darwin" ]; then
    export CROSS_COMPILE=/opt/arm-elf/bin/arm-elf-
  else  
    export CROSS_COMPILE=/cygdrive/c/devtools/CodeSourcery/Lite/bin/arm-none-eabi-
  fi
fi;

#export BDIR=system/kernel/testsystem
export BDIR=examples/tiny
make BOARDDIR=et_stm32_stamp clean
make BOARDDIR=et_stm32_stamp all
if [ $? -ne 0 ]; then quit 1
fi
