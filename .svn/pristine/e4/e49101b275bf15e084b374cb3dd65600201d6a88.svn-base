DIRS=libinstrument-dummy libinstrument-real pin dyninst pptrace
EZTRACE_PREFIX=$(shell printenv EZTRACE_PREFIX)
DYNINST_ROOT=$(shell printenv DYNINST_ROOT)
FXT_ROOT=$(shell printenv FXT_ROOT)
PIN_ROOT=$(shell printenv PIN_ROOT)
LIBDWARF_DIR=$(shell printenv DWARF_LIBDIR)

all:	recursive-all

run:	all recursive-run

clean:	recursive-clean

recursive-%:
	@for i in $(DIRS); do make -C $$i $(subst recursive-,,$@) EZTRACE_PREFIX=$(EZTRACE_PREFIX) DYNINST_ROOT=$(DYNINST_ROOT) FXT_ROOT=$(FXT_ROOT) PIN_ROOT=$(PIN_ROOT) LIBDWARF_DIR=$(LIBDWARF_DIR); done
