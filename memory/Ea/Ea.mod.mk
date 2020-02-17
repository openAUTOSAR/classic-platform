#Ea

obj-$(USE_EA)-$(CFG_GNULINUX) += Ea_gnulinux.o
obj-$(USE_EA)-$(if $(CFG_GNULINUX),n,y) += Ea.o
obj-$(USE_EA) += Ea_Lcfg.o

vpath-$(USE_EA) += $(ROOTDIR)/memory/Ea/src
inc-$(USE_EA) += $(ROOTDIR)/memory/Ea/inc