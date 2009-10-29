#/bin/bash
# Usage:
#   build_example <arch> <board> <example>
#
# Example: build_example ARM|PPC mpc551xsim| simple|tiny|.....
#
function quit {
	echo
	echo "### BUILD FAILED"
	echo 
	exit $1
}

SCRIPT_DIR=`dirname $0`   
ARCH=$1
source ${SCRIPT_DIR}/guess_cc.sh

export BDIR=examples/$3
make BOARDDIR=$2 clean 
make BOARDDIR=$2 all 
if [ $? -ne 0 ]; then quit 1
fi



