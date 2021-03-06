BINARIES=my_pin.so

LIBINSTRUMENT_ROOT=$(realpath ../../../instrument/libinstrument-real)

PIN_ROOT=$(shell printenv PIN_ROOT)
LIBDWARF_DIR=$(shell printenv DWARF_LIBDIR)

PIN_CFLAGS=-DTARGET_IA32E -DFUND_TC_HOSTCPU=FUND_CPU_INTEL64 -DFUND_TC_TARGETCPU=FUND_CPU_INTEL64 -DTARGET_LINUX -DFUND_TC_HOSTOS=FUND_OS_LINUX -DFUND_TC_TARGETOS=FUND_OS_LINUX \
	-fpermissive \
	-I$(PIN_ROOT)/source/InstLib -I$(PIN_ROOT)/extras/xed2-intel64/include -I$(PIN_ROOT)/extras/components/include -I$(PIN_ROOT)/source/include -I$(PIN_ROOT)/source/include/gen
LIBINSTRUMENT_CFLAGS=-I$(LIBINSTRUMENT_ROOT)
LIBINSTRUMENT_DUMMY_CFLAGS=-I$(LIBINSTRUMENT_SIMPLE_ROOT)

PIN_LDFLAGS=-Wl,--version-script=$(PIN_ROOT)/source/include/pintool.ver -L../Lib/ -L../ExtLib/ -L$(PIN_ROOT)/extras/xed2-intel64/lib -L$(PIN_ROOT)/intel64/lib -L$(PIN_ROOT)/intel64/lib-ext \
-Wl,--rpath,$(PIN_ROOT)/extras/xed2-intel64/lib/ -L$(PIN_ROOT)/extras/xed2-intel64/lib/\
 -lpin -lxed -lelf -ldl -Wl,--rpath,$(LIBDWARF_DIR) -L$(LIBDWARF_DIR) -ldwarf

LIBINSTRUMENT_LDFLAGS=-Wl,--rpath,$(LIBINSTRUMENT_ROOT) -L$(LIBINSTRUMENT_ROOT) -linstrument

all: $(BINARIES)

my_pin.so: my_pin.cpp
	g++ -shared -o my_pin.so -fPIC my_pin.cpp $(PIN_CFLAGS) $(PIN_LDFLAGS) $(LIBINSTRUMENT_CFLAGS) $(LIBINSTRUMENT_LDFLAGS)

run:
	@for i in $(TARGETS); do \
		echo ================================================================================; \
		echo Running PIN on `basename $$i` for real-life instrumentation; \
		echo ================================================================================; \
		echo; \
		for j in {1..$(NBRUNS)}; do \
			$(PIN_ROOT)/pin -t my_pin.so -- $$i | grep seconds | xargs echo | cut -f 1 -d " " | xargs echo -n; \
			echo -n " "; \
		done; \
		echo; \
	done

clean:
	rm -f *.o $(BINARIES)
	