# Safe IO Hardware Abstraction
vpath-$(USE_SAFEIOHWAB) += $(ROOTDIR)/Peripherals/SafeIoHwAb/src
inc-$(USE_SAFEIOHWAB) += $(ROOTDIR)/Peripherals/SafeIoHwAb/inc
obj-$(USE_SAFEIOHWAB) += SafeIoHwAb_Digital.o
obj-$(USE_SAFEIOHWAB) += SafeIoHwAb_Analog.o
obj-$(USE_SAFEIOHWAB) += SafeIoHwAb_Pwm.o
obj-$(USE_SAFEIOHWAB) += SafeIoHwAb_Cfg.o
obj-$(USE_SAFEIOHWAB) += SafeIoHwAb_Generic.o
