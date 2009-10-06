#/bin/bash

function quit {
    echo
    echo "### BUILD FAILED"
    echo 
    exit $1
}

SCRIPT_DIR=`dirname $0`   
ARCH=ARM
source ${SCRIPT_DIR}/guess_cc.sh
   	
export BDIR=examples/tiny
make BOARDDIR=et_stm32_stamp clean
make BOARDDIR=et_stm32_stamp all
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=system/kernel/testsystem
make BOARDDIR=et_stm32_stamp clean
make BOARDDIR=et_stm32_stamp all
if [ $? -ne 0 ]; then quit 1
fi
