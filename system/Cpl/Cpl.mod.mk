#CPL
obj-$(USE_CPL) += Cpl_Mac.o
obj-$(USE_CPL) += hmac_sha2.o
obj-$(USE_CPL) += sha2.o


inc-$(USE_CPL) += $(ROOTDIR)/system/Cpl
inc-$(USE_CPL) += $(ROOTDIR)/system/Cpl/src
inc-$(USE_CPL) += $(ROOTDIR)/system/Cpl/crypto
vpath-$(USE_CPL) += $(ROOTDIR)/system/Cpl/src
vpath-$(USE_CPL) += $(ROOTDIR)/system/Cpl/crypto
