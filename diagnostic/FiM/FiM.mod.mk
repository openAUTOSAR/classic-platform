# FiM
FIM_PB ?= y

obj-$(USE_FIM) += FiM.o
ifeq (${FIM_PB},y)
pb-obj-$(USE_FIM) += FiM_PBCfg.o
pb-pc-file-$(USE_FIM) += FiM_Cfg.h
else
obj-$(USE_FIM) += FiM_PBCfg.o
endif

vpath-$(USE_FIM) += $(ROOTDIR)/diagnostic/FiM/src
inc-$(USE_FIM) += $(ROOTDIR)/diagnostic/FiM/inc