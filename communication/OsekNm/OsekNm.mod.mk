#OsekNm
obj-$(USE_OSEKNM) += OsekDirectNm.o
obj-$(USE_OSEKNM) += OsekIndirectNm.o
obj-$(USE_OSEKNM) += OsekNm.o
obj-$(USE_OSEKNM) += OsekNm_Cfg.o
obj-$(USE_OSEKNM) += OsekNmDllStubs.o

vpath-$(USE_OSEKNM) += $(ROOTDIR)/communication/OsekNm/src
inc-$(USE_OSEKNM) += $(ROOTDIR)/communication/OsekNm/inc
