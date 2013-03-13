BINARIES=my_dyninst

LIBINSTRUMENT=$(realpath ../../../instrument/libinstrument-real)

DYNINST_ROOT=$(shell printenv DYNINST_ROOT)
DYNINST_LIBDIR=$(DYNINST_ROOT)/x86_64-unknown-linux2.4/lib/
DYNINST_CFLAGS=-I$(DYNINST_ROOT)/include/
DYNINST_LDFLAGS=-Wl,--rpath,$(DYNINST_LIBDIR) -L$(DYNINST_LIBDIR) -ldyninstAPI -liberty

LDLIBPATH=$(shell printenv LD_LIBRARY_PATH):$(DYNINST_LIBDIR):
DYNINST_RT=$(DYNINST_LIBDIR)/libdyninstAPI_RT.so
LDPRELOAD=$(DYNINST_LIBDIR)/libdyninstAPI.so

TARGET=instrumented

all: $(BINARIES)

my_dyninst: my_dyninst.cpp
	g++ -o my_dyninst my_dyninst.cpp $(DYNINST_CFLAGS) $(DYNINST_LDFLAGS) -O0 -g -DLIB_EZTRACE_SO=\"$(LIBINSTRUMENT)/libinstrument.so\"

run:
	@for i in $(TARGETS); do \
		make run-real SOURCE=$$i DYNINST_ROOT=$(DYNINST_ROOT); \
	done

run-real:	instrument
	@echo ================================================================================
	@echo Running DynInst on `basename $(SOURCE)` for real-life instrumentation
	@echo ================================================================================
	@for i in {1..$(NBRUNS)}; do \
		DYNINSTAPI_RT_LIB=$(DYNINST_RT) LD_LIBRARY_PATH=$(LDLIBPATH)$(LIBINSTRUMENT) ./$(TARGET) | grep seconds | xargs echo | cut -f 1 -d " " | xargs echo -n; \
		echo -n " "; \
	done
	@echo
	@rm -f $(TARGET) || true

instrument:
	LD_PRELOAD=$(LDPRELOAD) DYNINSTAPI_RT_LIB=$(DYNINST_RT) LD_LIBRARY_PATH=$(LDLIBPATH)$(LIBINSTRUMENT) ./my_dyninst $(SOURCE) $(TARGET) 1>&2

clean:
	rm -f *.o $(BINARIES)