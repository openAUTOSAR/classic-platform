
lintdef_ext=-D
lintinc_ext=-I

#splint_extra=+v -b -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt std.lnt
splint_extra=+nolib
inc-y += $(cc_inc_path)
#lint_files=$(TOPDIR)/scripts/lint/std.lnt
#lint_files=$(TOPDIR)/scripts/lint/lnt/au-misra2.lnt
#lint_files+=$(TOPDIR)/scripts/lint/lnt/au-misra2.lnt

#$(addprefix -i,$(abs-inc-y)) /C/lint/std.lnt  $(abspath $<))
