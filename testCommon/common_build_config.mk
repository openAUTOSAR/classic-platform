# This file is included by the build_config.mk in the os tests
# It is currently only used for setting up the return value when
# lint produces warnings.
#

ALLOW_LINT_WARNINGS=y

ifeq ($(board_name), mpc5604b_xpc560b)
ifeq ($(COMPILER), cw)
ALLOW_LINT_WARNINGS=n
endif
endif

ifeq ($(board_name), rh850f1l)
RH850_PARTNUM ?= RF7010243
endif

