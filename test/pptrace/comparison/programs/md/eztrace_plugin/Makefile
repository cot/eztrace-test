BIN = libeztrace-autostart-example_lib.so

LIBINSTRUMENT_ROOT=$(realpath ../../../instrument/libinstrument-real)

EZTRACE_PREFIX=$(shell printenv EZTRACE_PREFIX)
EZTRACE_CFLAGS=-I$(EZTRACE_PREFIX)/include/ -I$(EZTRACE_PREFIX)/include/fxt
EZTRACE_LDFLAGS=-Wl,--rpath,$(EZTRACE_PREFIX)/lib/ -L$(EZTRACE_PREFIX)/lib -leztrace

all: $(BIN)

instrument_pptrace.o: instrument_pptrace.c
	$(CC) -c instrument_pptrace.c -fPIC $(EZTRACE_CFLAGS) -I$(LIBINSTRUMENT_ROOT) -O3

libeztrace-autostart-example_lib.so: instrument_pptrace.o
	$(CC) -shared -o $@ $< $(EZTRACE_LDFLAGS) -Wl,--rpath,$(LIBINSTRUMENT_ROOT) -L$(LIBINSTRUMENT_ROOT) -linstrument

run:
	@for i in $(TARGETS); do \
		echo ================================================================================; \
		echo Running EZTrace on `basename $$i` for real-life instrumentation; \
		echo ================================================================================; \
		for j in {1..$(NBRUNS)}; do \
			EZTRACE_LIBRARY_PATH=$(CURDIR) EZTRACE_TRACE=example_lib $(EZTRACE_PREFIX)/bin/eztrace_pptrace $$i | grep seconds | xargs echo | cut -f 1 -d " " | xargs echo -n; \
			echo -n " "; \
		done; \
		echo; \
	done

clean:
	rm -f $(BIN)
