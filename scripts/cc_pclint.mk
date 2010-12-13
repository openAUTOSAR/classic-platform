
lintdef_ext=-d
lintinc_ext=-i

lint_extra=+v -b -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt std.lnt
inc-y += $(cc_inc_path)
#lint_files=$(TOPDIR)/scripts/lint/std.lnt
#lint_files=$(TOPDIR)/scripts/lint/lnt/au-misra2.lnt
#lint_files+=$(TOPDIR)/scripts/lint/lnt/au-misra2.lnt

#$(addprefix -i,$(abs-inc-y)) /C/lint/std.lnt  $(abspath $<))
