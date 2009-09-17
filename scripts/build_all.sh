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
  if [ "$os"=="Darwin" ]; then
    export CROSS_COMPILE=/opt/powerpc-eabi/bin/powerpc-eabi-
  else
    export CROSS_COMPILE=/cygdrive/c/devtools/gcc/4.1.2/powerpc-eabispe/bin/powerpc-eabispe-
  fi
fi;

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

exit 0
	








