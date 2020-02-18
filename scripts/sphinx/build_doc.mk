
# Invokation:
# From any directory
# - make -f ../../../../scripts/sphinx/build_doc.mk quick-start
# 
# There should always be a index.rst file
#


#.PHONY: build_doc

# Some useful variables..
comma:= ,
empty:=
space:= $(empty) $(empty)
split = $(subst $(comma), ,$(1))

$(call split,$(BDIR))

# This dir.
#export ARC_DOC_ROOT=$(CURDIR)
# Root of the core directory
export ROOTDIR=$(CURDIR)/../.. 
export PYTHON_ROOT=/c/Python27


export MIKTEX_ROOT?=/c/devtools/miktex/texmfs/install/miktex/bin
export PATH:=$(PATH):$(PYTHON_ROOT):$(MIKTEX_ROOT):$(PYTHON_ROOT)/Scripts

SPHINXBUILD=python -msphinx
SPHINXQUICKSTART=sphinx-quickstart.exe


mkfile_path := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export ARC_DOC_ROOT=$(mkfile_path)

$(info $(doc_module))
$(info $(doc_type))
$(info $(doc_mcu))

ifdef doc_mcu
doc_name=$(doc_module)_$(doc_type)_$(doc_mcu)
conf_opt=-m $(doc_module) -t $(doc_type) -a $(doc_mcu)
tag_opt=-t $(doc_mcu)
else
doc_name=$(doc_module)_$(doc_type)
conf_opt=-m $(doc_module) -t $(doc_type)
endif

%.rst: %.rstp
	gcc -E -P -x c -traditional-cpp -Dcfg_$(doc_mcu) $< > $@

# Create conf.py from template (conf_main.py)
conf.py: $(SPHINXDIR)/conf_main.py $(SPHINXDIR)/create_conf.py
	$(Q)echo Creating $<
	cp $(SPHINXDIR)/conf_main.py conf.py
	python $(SPHINXDIR)/create_conf.py $(conf_opt) >> conf.py

   
all: conf.py $(rstp-y)
	$(SPHINXBUILD) $(tag_opt) -b html . obj_doc/html
	$(SPHINXBUILD) $(tag_opt) -b latex . obj_doc/latex
	#echo $doc_module_$doc_type
	pushd obj_doc/latex && pdfLatex $(doc_name).tex && pdfLatex $(doc_name).tex   
	
clean:
	rm -rf obj_doc/* conf.py

quick-start:
	$(SPHINXQUICKSTART)
