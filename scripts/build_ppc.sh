#/bin/bash
# Usage:
#
# build_ppc
#

function quit {
	echo
	echo "### BUILD FAILED"
	echo 
	exit $1
}

SCRIPT_DIR=`dirname $0`   
ARCH=PPC
source ${SCRIPT_DIR}/guess_cc.sh
	  
export BDIR=system/kernel/testsystem
make BOARDDIR=mpc5554sim clean 
make BOARDDIR=mpc5554sim all 
if [ $? -ne 0 ]; then quit 1
fi

make BOARDDIR=mpc551xsim clean 
make BOARDDIR=mpc551xsim all 
if [ $? -ne 0 ]; then quit 1
fi

make BOARDDIR=mpc5516it clean 
make BOARDDIR=mpc5516it all
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=system/kernel/testsystem,examples/blinker_node 
make BOARDDIR=mpc5567qrtech clean 
make BOARDDIR=mpc5567qrtech all 
if [ $? -ne 0 ]; then quit 1
fi
	








