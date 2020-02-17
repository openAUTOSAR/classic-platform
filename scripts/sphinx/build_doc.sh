#!/bin/bash

#rm -rf _build_can/html/
#rm -rf _build_can/latex/
#rm -rf _build_can/html_mcu_mpc55xx/
#rm -rf _build_can/html_mcu_tms570/
#rm -rf _build_can/latex_mcu_mpc55xx/
#rm -rf _build_can/latex_mcu_tms570/


function check_precond() {

  # Check gcc
  if ! [ -x "$(command -v gcc)" ]; then
    echo 'Error: gcc is not installed.' >&2
    exit 1
  fi
}

# 1 - to
# 2 - from 
function create_symlink() {

	if [ -f $1 ]; then
		rm $1
	fi 
	
	ln -s $2 $1
}

function to_upper() {
	a=`echo $1 | awk '{print toupper($0)}'`
	echo $a
}

# 1 - module
# 2 - type (um or dd)
# 3 - mcu
function build_mcal_doc() {
	# Create conf.py files
	cat $cdir/conf_main.py > $rdir/mcal/$1/doc/$2/conf.py
	python create_conf.py -m $1 -t $2 -a $3 >> $rdir/mcal/$1/doc/$2/conf.py
		
	# 
	# Create symlink to arch
	#
	
	pushd $rdir/mcal/$1/doc/$2	
	
	create_symlink ug_arch.rst arch/ug_$3.rst
	create_symlink arch/docref_arch.rst arch/docref_$3.rst
		
	# Preprocess if exist...
	if [ -f intro.rstp ]; then	
		mcu_cap=$( to_upper $3 )
		gcc -E -P -x c -traditional-cpp -DCFG_$mcu_cap intro.rstp > intro.rst
	fi

	# Generate html 
	sphinx-build.exe -b html -t $3 . $rdir/_build_$1_$2_$3/html/
	
	# Generate latex
	sphinx-build.exe -b latex -t $3 . $rdir/_build_$1_$2_$3/latex/

	# Cleanup
	rm arch/docref_arch.rst
	rm ug_arch.rst
	#rm conf.py
	
	popd 
	
	# Pdf	
	pushd $rdir/_build_$1_$2_$3/latex
	pdfLatex $1_$2_$3.tex
	popd
}


# 1 - module
# 2 - type (um or dd)
# ( 3 - mcu )
function build_mcal_dd_doc() {
	# Create conf.py files
	cat $cdir/conf_main.py > $rdir/mcal/doc/$2/conf.py
	python create_conf.py -m $1 -t $2 >> $rdir/mcal/doc/$2/conf.py
			
	pushd $rdir/mcal/doc/$2	
	
	# Generate html 
	sphinx-build.exe -b html . $rdir/_build_$1_$2/html/
	
	# Generate latex
	sphinx-build.exe -b latex . $rdir/_build_$1_$2/latex/
	
	popd 
	
	# Pdf	
	pushd $rdir/_build_$1_$2/latex
	pdfLatex $1_$2.tex
	popd
}


check_precond 


export PATH=$PATH:/c/Python27:/c/devtools/miktex/texmfs/install/miktex/bin
# Get script path
#cdir=$(dirname "$(readlink -f "$0")")
cdir=$ARC_DOC_ROOT
echo $cdir
rdir=$cdir/../..
echo $rdir

# export path so extension can be found by sphinx.
#export ARC_DOC_ROOT=$cdir


#-----------------------------------
# MCAL
#-----------------------------------

# MCAL design description
#build_mcal_dd_doc MCAL dd


# MCAL modules ( um and um_<mcu> )  
#build_mcal_doc can um mpc5xxx

# MCAL module design description   
#build_mcal_doc can dd mpc5xxx 

# module safety manual   
#build_mcal_doc can sm mpc5xxx



# MCAL modules ( um and um_<mcu> )  
build_mcal_doc pwm um mpc5xxx

# MCAL module design description   
build_mcal_doc pwm dd mpc5xxx 

# module safety manual   
build_mcal_doc pwm sm mpc5xxx

#
# DIO
#

# MCAL modules ( um and um_<mcu> )  
build_mcal_doc dio um mpc5xxx

# MCAL module design description   
build_mcal_doc dio dd mpc5xxx 

# module safety manual   
build_mcal_doc dio sm mpc5xxx


#
# Port
#

# MCAL modules ( um and um_<mcu> )
build_mcal_doc Port um mpc5xxx

# MCAL module design description   
build_mcal_doc Port dd mpc5xxx 

# module safety manual   
build_mcal_doc Port sm mpc5xxx

#
# MCU
#

# MCAL modules ( um and um_<mcu> )
build_mcal_doc MCU um mpc5xxx

# MCAL module design description   
build_mcal_doc MCU dd mpc5xxx 

# module safety manual   
build_mcal_doc MCU sm mpc5xxx




















#
# ADC
#

# MCAL modules ( um and um_<mcu> ) 
build_mcal_doc ADC um mpc5xxx

# MCAL module design description   
build_mcal_doc ADC dd mpc5xxx 

# module safety manual   
build_mcal_doc ADC sm mpc5xxx




exit




