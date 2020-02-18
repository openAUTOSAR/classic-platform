# This file is included by the build_config.mk in the os tests
# It is currently only used for setting up the return value when
# lint produces warnings.
#

ALLOW_LINT_WARNINGS=y
CFG+=MINIMAL_OUTPUT


ifeq ($(board_name), mpc5604b_xpc560b)
ifeq ($(COMPILER), cw)
ALLOW_LINT_WARNINGS=n
endif
endif

ifeq ($(board_name), rh850f1l)
RH850_PARTNUM ?= RF7010243
endif

ifeq ($(BUILD_OS_SAFETY_PLATFORM),y)
  def-y += OS_SYSTICK_APP=APPLICATION_ID_OsApplication_Sys

  CFG+=TC2XX_OPTIMIZE_IRQ_VECTORTABLE_SIZE

  CFG+=ARC_CLIB
  # Currently only winidea support
  SELECT_CONSOLE = TTY_WINIDEA
  board_name_suffix=_sp
endif


