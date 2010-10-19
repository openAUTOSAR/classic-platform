#/bin/bash
# Usage:
#
# build_ppc
#
# This script file is quite braindead... remake when time..


function quit {
	echo
	echo "### BUILD FAILED"
	echo 
	exit $1
}

SCRIPT_DIR=`dirname $0`   
ARCH=PPC
source ${SCRIPT_DIR}/guess_cc.sh
	  	  	  
export BDIR=system/kernel/testsystem/suite_01,system/kernel/testsystem/suite_02,,system/kernel/testsystem/suite_03
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

export examples/blinker_node 
make BOARDDIR=mpc5567qrtech clean 
make BOARDDIR=mpc5567qrtech all 
if [ $? -ne 0 ]; then quit 1
fi
	
	
# Build the examples...

export BDIR=examples/blinker_node
make BOARDDIR=mpc5516it clean 
make BOARDDIR=mpc5516it all 
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=examples/pwm_node
make BOARDDIR=mpc5516it clean 
make BOARDDIR=mpc5516it all 
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=examples/pwm_node2
make BOARDDIR=mpc5516it clean 
make BOARDDIR=mpc5516it all 
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=examples/simple
make BOARDDIR=mpc551xsim clean 
make BOARDDIR=mpc551xsim all 
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=examples/switch_node
make BOARDDIR=mpc5516it clean 
make BOARDDIR=mpc5516it all 
if [ $? -ne 0 ]; then quit 1
fi

export BDIR=examples/tiny
make BOARDDIR=mpc5516it clean 
make BOARDDIR=mpc5516it all 
if [ $? -ne 0 ]; then quit 1
fi






	
	








