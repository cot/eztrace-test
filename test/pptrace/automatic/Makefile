BITS=64
CORE_DIRECTORY=/home/rue/EZTRACE/eztrace/unused-git/src/core
PPTRACE_DIRECTORY=/home/rue/EZTRACE/eztrace/unused-git/src/pptrace
TESTS=errors binary tracing opcodes wait_open hijack
DEBUG_LEVEL=0

ifeq "1" "1"
	LIBBIN=-lbfd
else
	LIBBIN=-lelf
endif

errors_FLAGS=$(LIBBIN)
errors_DEPS=errors.c binary.c

binary_FLAGS=$(LIBBIN)
binary_DEPS=errors.c binary.c

wait_open_FLAGS=$(LIBBIN)
wait_open_DEPS=errors.c binary.c tracing.c

tracing_FLAGS=$(LIBBIN)
tracing_DEPS=errors.c binary.c tracing.c

opcodes_FLAGS=$(LIBBIN)
opcodes_DEPS=errors.c binary.c opcodes.c

hijack_FLAGS=$(LIBBIN)
hijack_DEPS=errors.c binary.c opcodes.c tracing.c hijack.c memory.c isize.c

TESTCASES_=$(patsubst %.c,%,$(wildcard testcase/*.c))
TESTCASES=$(foreach testcase,$(TESTCASES_),$(foreach b,$(BITS),$(testcase).$(b) $(testcase).$(b).so))

TESTS_CHECK=$(foreach test,$(TESTS),$(test).check)

CFLAGS = -O0 -g -I$(PPTRACE_DIRECTORY) -I$(CORE_DIRECTORY) -D__PPTRACE_DEBUG_LEVEL=$(DEBUG_LEVEL)

.PHONY:	check testcase

all:

distdir:

check:	testcase $(TESTS)

testcase:
	@make -C testcase BITS='$(BITS)'

%.check:	%
	@echo -n " *** RUNNING TEST $<... "
	@./$<.test || (echo "FAILED!"; false)
	@echo "OK"

%::	%.c 
	@echo CC	$@
	@$(CC) $(CFLAGS) $^ $(foreach d,$($@_DEPS),$(PPTRACE_DIRECTORY)/$d) -o $@ $($@_FLAGS) 
	@echo "#!/bin/sh" >$@.test
	@echo "cd $$PWD" >>$@.test
	@echo './$@ ${TESTCASES} $$*' >>$@.test
	@echo 'RES=$$?' >>$@.test
	@echo 'cd - >/dev/null' >>$@.test
	@echo 'exit $$RES'>>$@.test
	@chmod +x $@.test

install:

installcheck:

uninstall:

distclean: clean

clean:
	@rm -f $(TESTS) $(foreach d,$(TESTS),$d.test)
	@if [ -d testcase ] ; then make -C testcase BITS='$(BITS)' clean ; fi
