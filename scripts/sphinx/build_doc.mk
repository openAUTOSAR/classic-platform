
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
export ARC_DOC_ROOT=$(CURDIR)
# Root of the core directory
export ROOTDIR=$(CURDIR)/../.. 
export PYTHON_ROOT=/c/Python27


export MIKTEX_ROOT?=/c/devtools/miktex/texmfs/install/miktex/bin
export PATH:=$(PATH):$(PYTHON_ROOT):$(MIKTEX_ROOT):$(PYTHON_ROOT)/Scripts

SPHINXBUILD=python -msphinx
SPHINXQUICKSTART=sphinx-quickstart.exe


%.rst: %.rstp
	gcc -E -P -x c -traditional-cpp -DCFG_MPC5XXX $< > $@

# Create conf.py from template (conf_main.py)
conf.py: $(SPHINXDIR)/conf_main.py
	$(Q)echo Creating $<
	cp $(SPHINXDIR)/conf_main.py conf.py
	python $(SPHINXDIR)/create_conf.py -m $(doc_module) -t $(doc_type) >> conf.py	 


ifneq ($(arch-sym-y),)
qq:
	python $(SPHINXDIR)/create_symlink.py $(arch-sym-y)
else
qq:
	
endif

#$(foreach sym,$(arch-sym-y),$(eval $(call PROGRAM_template,$(sym))))
  
$(info $(rstp-y))
 
all: conf.py $(rstp-y) qq 
	$(SPHINXBUILD) -b html . obj_doc/html
	$(SPHINXBUILD) -b latex . obj_doc/latex
	#echo $doc_module_$doc_type
	pushd obj_doc/latex &&  pdfLatex $(doc_module)_$(doc_type).tex  
	

quick-start:
	$(SPHINXQUICKSTART)
