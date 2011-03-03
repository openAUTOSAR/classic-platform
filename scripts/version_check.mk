

# Verification that the build settings are sound

CORE_BUILD_SYSTEM_VERSION_MAJOR = 1
CORE_BUILD_SYSTEM_VERSION_MINOR = 0
CORE_BUILD_SYSTEM_VERSION_PATCH = 0

# Only check if specified 
# (remove when we want to start checking always)
ifneq ($(REQUIRED_BUILD_SYSTEM_VERSION),)

REQUIRED_BUILD_SYSTEM_VERSION:=$(subst ., ,$(REQUIRED_BUILD_SYSTEM_VERSION))
REQUIRED_BUILD_SYSTEM_VERSION_MAJOR=$(word 1,$(REQUIRED_BUILD_SYSTEM_VERSION))
REQUIRED_BUILD_SYSTEM_VERSION_MINOR=$(word 2,$(REQUIRED_BUILD_SYSTEM_VERSION))
REQUIRED_BUILD_SYSTEM_VERSION_PATCH=$(word 3,$(REQUIRED_BUILD_SYSTEM_VERSION))

ifneq ($(CORE_BUILD_SYSTEM_VERSION_MAJOR),$(REQUIRED_BUILD_SYSTEM_VERSION_MAJOR))
ERROR_BUILD_SYSTEM_VERSION = y
endif
ifneq ($(CORE_BUILD_SYSTEM_VERSION_MINOR),$(REQUIRED_BUILD_SYSTEM_VERSION_MINOR))
ERROR_BUILD_SYSTEM_VERSION = y
endif
ifeq (y,$(ERROR_BUILD_SYSTEM_VERSION))
$(error Build system version mismatch. Core has $(CORE_BUILD_SYSTEM_VERSION_MAJOR).$(CORE_BUILD_SYSTEM_VERSION_MINOR), Project requires $(REQUIRED_BUILD_SYSTEM_VERSION_MAJOR).$(REQUIRED_BUILD_SYSTEM_VERSION_MINOR))
endif

else # Only check if specified
$(info >>>> Build system version check not performed.)

endif # Only check if specified

define assert_exist
$(if $(value $(1)),,
$(error [Build system] Missing variable: $(1)))
endef

#$(call assert_exist,BOARDDIR)
#$(call assert_exist,CROSS_COMPILE)
