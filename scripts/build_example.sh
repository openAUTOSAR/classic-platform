#/bin/bash
# Usage:
#   build_example <all|clean> <arch> <board> <example>
#
# Example: build_example all|clean ARM|PPC mpc551xsim| simple|tiny|.....
#
function quit {
	echo
	echo "### BUILD FAILED"
	echo 
	exit $1
}

if [ $# -ne 4 ]; then
echo "usage:"  
echo " build_example <build_target> <arch> <board> <example>"
echo "   <build target> - all | clean"
echo "   <arch>         - ARM | PPC"
echo "   <board>        - mpc551xsim | ..."
echo "   <example>      - simple | tiny | ..."
echo ""
echo "example:"
echo " $build_example all PPC mpc551xsim simple"
quit 1;
fi 

SCRIPT_DIR=`dirname $0`   
ARCH=$2
source ${SCRIPT_DIR}/guess_cc.sh

export BDIR=examples/$4
make BOARDDIR=$3 $1 
if [ $? -ne 0 ]; then quit 1
fi



