
lintdef_ext=-d
lintinc_ext=-i

ifeq (${COMPILER},cw)
lint_extra=+v -b -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt cw.lnt
else ifeq (${COMPILER},ghs)	
	ifeq (${ARCH},rh850_x)
	lint_extra=-i$(GHS_COMPILE)/include/v800 -d__v800__ -d__LITTLE_ENDIAN__ -d__V850__ -d__V850
	else
	lint_extra=-i$(GHS_COMPILE)/include/ppc -d__ppc
	endif
lint_extra+=+v -b -i$(TOPDIR)/scripts/pclint -i$(GHS_COMPILE)/ansi -i$(TOPDIR)/scripts/pclint/lnt ghs.lnt	
else ifeq (${COMPILER},armcc)
lint_extra=+v -b -i$(ARMCC_COMPILE)/include  -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt armcc.lnt
else 
# Use Gcc settings for all other compilers
lint_extra=+v -b -i$(TOPDIR)/scripts/pclint -i$(TOPDIR)/scripts/pclint/lnt $(addprefix $(lintinc_ext),$(cc_inc_path))  gcc.lnt
	ifeq (${ARCH},aurix)
	lint_extra+=-d__HIGHTEC__
	endif
endif

ifeq (${ALLOW_LINT_WARNINGS},y)
lint_extra += -zero
endif

lint_extra += ${LINT_EXTRA}