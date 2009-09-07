#!/bin/bash

if [ "$1" == "" ]; then
  echo "error: T32 Installation path not supplied"
  exit 1
fi

pwd_cmd=`cygpath -d \`pwd\``
#echo $pwd_cmd
echo "cd ${pwd_cmd}" > $1/t32.cmm
cat start.cmm >> $1/t32.cmm
##dos2unix $1/t32.cmm
cp -v config_sim.t32 $1


