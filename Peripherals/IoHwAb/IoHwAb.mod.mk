# IO Hardware Abstraction
obj-$(USE_IOHWAB) += IoHwAb_Digital.o
obj-$(USE_IOHWAB) += IoHwAb_Analog.o
obj-$(USE_IOHWAB) += IoHwAb_Pwm.o

vpath-$(USE_IOHWAB) += $(ROOTDIR)/Peripherals/IoHwAb/src
inc-$(USE_IOHWAB) += $(ROOTDIR)/Peripherals/IoHwAb/src
inc-$(USE_IOHWAB) += $(ROOTDIR)/Peripherals/IoHwAb/inc

