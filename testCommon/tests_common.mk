
###
# This file is supposed to be included from the test-specific makefiles.
# For this reason all paths in this file is relative to the current BDIR.
###

CFG_STANDARD_NEWLIB:=y

# === EmbUnit ===
VPATH += $(PROJECT_DIR)/embUnit/embUnit
inc-y += $(PROJECT_DIR)/embUnit/embUnit
LINT_EXCLUDE_PATHS += $(PROJECT_DIR)/embUnit/embUnit
VPATH += $(PROJECT_DIR)/embUnit/textui
inc-y += $(PROJECT_DIR)/embUnit/textui
LINT_EXCLUDE_PATHS += $(PROJECT_DIR)/embUnit/textui
inc-y += $(PROJECT_DIR)/embUnit
include $(PROJECT_DIR)/embUnit/embUnit/embUnit.mk
include $(PROJECT_DIR)/embUnit/textui/textui.mk

# === Common includes ===
inc-y += $(ROOTDIR)/testCommon/inc


# === Test specific configuration files ===
# The more precise configuration, the higher preceedance.
VPATH := ../config/$(board_name) ../config ../stubs ../utils $(VPATH)
inc-y := $(inc-pre-y) ../config/$(board_name) ../config ../stubs ../utils $(inc-y)


# === Test specific object files ===
# Our project files (adds all .c files in project root)
PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
obj-y += $(PROJECT_C_FILES:%.c=%.o)
VPATH += ..

def-y += $(XML_HEADER_SWITCH)
def-y += $(XML_ESCAPE_SWITCH)
def-y += $(XML_OUTPUT_SWITCH)


ifeq (USE_LINSM, $(findstring USE_LINSM,$(def-y)))
inc-y += $(ROOTDIR)/communication/LinSM/inc 
endif

ifeq (USE_LINIF, $(findstring USE_LINIF,$(def-y)))
inc-y += $(ROOTDIR)/communication/LinIf/inc 
endif

ifeq (USE_CANSM, $(findstring USE_CANSM,$(def-y)))
inc-y += $(ROOTDIR)/communication/CanSM/inc 
endif

ifeq (USE_CANIF, $(findstring USE_CANIF,$(def-y)))
inc-y += $(ROOTDIR)/communication/CanIf/inc 
endif

ifeq (USE_LIN, $(findstring USE_LIN,$(def-y)))
inc-y += $(ROOTDIR)/mcal/Lin/inc 
endif

ifeq (USE_SPI, $(findstring USE_SPI,$(def-y)))
inc-y += $(ROOTDIR)/mcal/Spi/inc 
endif

ifeq (USE_FRSM, $(findstring USE_FRSM,$(def-y)))
inc-y += $(ROOTDIR)/communication/FrSM/inc 
endif

ifeq (USE_ETHSM, $(findstring USE_ETHSM,$(def-y)))
inc-y += $(ROOTDIR)/communication/EthSM/inc 
endif

ifeq (USE_ETH, $(findstring USE_ETH,$(def-y)))
inc-y += $(ROOTDIR)/mcal/Eth/inc 
endif

ifeq (USE_NM, $(findstring USE_NM,$(def-y)))
inc-y += $(ROOTDIR)/communication/Nm/inc 
endif

ifeq (USE_ECUM, $(findstring USE_ECUM,$(def-y)))
inc-y += $(ROOTDIR)/system/EcuM/inc 
endif

ifeq (USE_DEM, $(findstring USE_DEM,$(def-y)))
inc-y += $(ROOTDIR)/diagnostic/Dem/inc 
endif

ifeq (USE_COM, $(findstring USE_COM,$(def-y)))
inc-y += $(ROOTDIR)/communication/Com/inc 
endif

ifeq (USE_COMM, $(findstring USE_COMM,$(def-y)))
inc-y += $(ROOTDIR)/communication/ComM/inc 
endif

ifeq (USE_PDUR, $(findstring USE_PDUR,$(def-y)))
inc-y += $(ROOTDIR)/communication/PduR/inc
inc-y += $(ROOTDIR)/communication/Com/inc 
endif

ifeq (USE_SD, $(findstring USE_SD,$(def-y)))
inc-y += $(ROOTDIR)/communication/SD/inc 
endif

ifeq (USE_TCPIP, $(findstring USE_TCPIP,$(def-y)))
inc-y += $(ROOTDIR)/communication/TcpIp/inc 
endif

ifeq (USE_SOAD, $(findstring USE_SOAD,$(def-y)))
inc-y += $(ROOTDIR)/communication/SoAd/inc 
endif


ifeq (USE_DOIP, $(findstring USE_DOIP,$(def-y)))
inc-y += $(ROOTDIR)/communication/DoIP/inc 
endif

ifeq (USE_DCM, $(findstring USE_DCM,$(def-y)))
inc-y += $(ROOTDIR)/diagnostic/Dcm/inc 
endif

ifeq (USE_DET, $(findstring USE_DET,$(def-y)))
inc-y += $(ROOTDIR)/diagnostic/Det/inc 
endif

ifeq (USE_NM, $(findstring USE_NM,$(def-y)))
inc-y += $(ROOTDIR)/communication/Nm/inc 
endif

ifeq (USE_CANNM, $(findstring USE_CANNM,$(def-y)))
inc-y += $(ROOTDIR)/communication/CanNm/inc 
endif

ifeq (USE_UDPNM, $(findstring USE_UDPNM,$(def-y)))
inc-y += $(ROOTDIR)/communication/UdpNm/inc 
endif

ifeq (USE_CANTP, $(findstring USE_CANTP,$(def-y)))
inc-y += $(ROOTDIR)/communication/CanTp/inc 
endif


ifeq (USE_MEMIF, $(findstring USE_MEMIF,$(def-y)))
inc-y += $(ROOTDIR)/memory/Memif/inc 
endif

ifeq (USE_NVM, $(findstring USE_NVM,$(def-y)))
inc-y += $(ROOTDIR)/memory/NvM/inc 
endif

ifeq (USE_Fee, $(findstring USE_FEE,$(def-y)))
inc-y += $(ROOTDIR)/memory/Fee/inc 
endif

ifeq (USE_EA, $(findstring USE_EA,$(def-y)))
inc-y += $(ROOTDIR)/memory/Ea/inc 
endif


ifeq (USE_BSWM, $(findstring USE_BSWM,$(def-y)))
inc-y += $(ROOTDIR)/system/BswM/inc 
endif

