
lintdef_ext=-d
lintinc_ext=-i

ifeq (${COMPILER},cw)
lint_extra=+v -b -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt cw.lnt
else
lint_extra=+v -b -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt gcc.lnt 
endif

ifeq (${ALLOW_LINT_WARNINGS},y)
lint_extra += -zero
endif

inc-y += $(cc_inc_path)
#lint_files=$(TOPDIR)/scripts/lint/std.lnt
#lint_files=$(TOPDIR)/scripts/lint/lnt/au-misra2.lnt
#lint_files+=$(TOPDIR)/scripts/lint/lnt/au-misra2.lnt

#$(addprefix -i,$(abs-inc-y)) /C/lint/std.lnt  $(abspath $<))
