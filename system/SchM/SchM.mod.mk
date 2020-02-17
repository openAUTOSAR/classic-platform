#SchM

# Add the regular SchM as a target if both SCHM is used as module
# and if SAFETY_PLATFORM has not been added to CFG +=
obj-$(USE_SCHM)-$(if $(CFG_SAFETY_PLATFORM),,y) += SchM.o

# Add safety platform SCHM solution if both SCHM and 
# CFG_SAFETYPLATFORM have been defined
obj-$(USE_SCHM)-$(CFG_SAFETY_PLATFORM) += SchM_partition_QM.o
obj-$(USE_SCHM)-$(CFG_SAFETY_PLATFORM) += SchM_partition_A0.o

# The SchM/inc should always be included even if SchM module is not used
inc-y += $(ROOTDIR)/system/SchM/inc
vpath-$(USE_SCHM) += $(ROOTDIR)/system/SchM/src

# MemMap is located in src directory, used for safety platform
inc-$(USE_SCHM)-$(CFG_SAFETY_PLATFORM) += $(ROOTDIR)/system/SchM/src


