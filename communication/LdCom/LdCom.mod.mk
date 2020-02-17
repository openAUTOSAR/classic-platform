#LdCom
obj-$(USE_LDCOM) += LdCom.o
obj-$(USE_LDCOM) += LdCom_Lcfg.o
obj-$(USE_LDCOM) += LdCom_PBcfg.o

vpath-$(USE_LDCOM) += $(ROOTDIR)/communication/LdCom/src
inc-$(USE_LDCOM) += $(ROOTDIR)/communication/LdCom/inc
