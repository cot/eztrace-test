BINARIES=libeztrace-autostart-foodummy.so libeztrace-autostart-foo.so

LIBINSTRUMENT_ROOT_DUMMY=$(realpath ../libinstrument-dummy)
LIBINSTRUMENT_ROOT=$(realpath ../libinstrument-real)

EZTRACE_PREFIX=$(shell printenv EZTRACE_PREFIX)
EZTRACE_CFLAGS=-I$(EZTRACE_PREFIX)/include/ -I$(EZTRACE_PREFIX)/include/fxt
EZTRACE_LDFLAGS=-Wl,--rpath,$(EZTRACE_PREFIX)/lib/ -L$(EZTRACE_PREFIX)/lib -leztrace

all: $(BINARIES)

instrument_pptrace.o: instrument_pptrace.c
	$(CC) -c instrument_pptrace.c -fPIC $(EZTRACE_CFLAGS) -I$(LIBINSTRUMENT_ROOT) -O3

libeztrace-autostart-foodummy.so: instrument_pptrace.o
	$(CC) -shared -o $@ $< $(EZTRACE_LDFLAGS) -Wl,--rpath,$(LIBINSTRUMENT_ROOT_DUMMY) -L$(LIBINSTRUMENT_ROOT_DUMMY) -linstrument

libeztrace-autostart-foo.so: instrument_pptrace.o
	$(CC) -shared -o $@ $< $(EZTRACE_LDFLAGS) -Wl,--rpath,$(LIBINSTRUMENT_ROOT) -L$(LIBINSTRUMENT_ROOT) -linstrument

run:
	@for i in $(TARGETS); do \
		echo ================================================================================; \
		echo Running EZTrace on `basename $$i` for simple instrumentation; \
		echo ================================================================================; \
		echo; \
		EZTRACE_LIBRARY_PATH=$(CURDIR) EZTRACE_TRACE=foodummy $(EZTRACE_PREFIX)/bin/eztrace_pptrace $$i; \
		echo ================================================================================; \
		echo Running EZTrace on `basename $$i` for real instrumentation; \
		echo ================================================================================; \
		echo; \
		EZTRACE_LIBRARY_PATH=$(CURDIR) EZTRACE_TRACE=foo $(EZTRACE_PREFIX)/bin/eztrace_pptrace $$i; \
	done

clean:
	rm -f *.o $(BINARIES)