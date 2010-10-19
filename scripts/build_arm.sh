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
make BOARDDIR=stm32_stm3210c clean
make BOARDDIR=stm32_stm3210c all
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=examples/simple 
make BOARDDIR=stm32_stm3210c clean
make BOARDDIR=stm32_stm3210c all
if [ $? -ne 0 ]; then quit 1
fi

# Problems memory..
#export BDIR=system/kernel/testsystem/suite_01,system/kernel/testsystem/suite_02, system/kernel/testsystem/suite_03
#make BOARDDIR=stm32_stm3210c clean
#make BOARDDIR=stm32_stm3210c all
#if [ $? -ne 0 ]; then quit 1
#fi


