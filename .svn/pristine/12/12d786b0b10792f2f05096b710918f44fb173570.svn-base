BINARIES=my_dyninst my_dyninst_simple

LIBINSTRUMENT_SIMPLE=$(realpath ../libinstrument-dummy)
LIBINSTRUMENT_REAL=$(realpath ../libinstrument-real)

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
	g++ -o my_dyninst my_dyninst.cpp $(DYNINST_CFLAGS) $(DYNINST_LDFLAGS) -O0 -g -DLIB_EZTRACE_SO=\"$(LIBINSTRUMENT_REAL)/libinstrument.so\"

my_dyninst_simple: my_dyninst.cpp
	g++ -o my_dyninst_simple my_dyninst.cpp $(DYNINST_CFLAGS) $(DYNINST_LDFLAGS) -O0 -g -DLIB_EZTRACE_SO=\"$(LIBINSTRUMENT_SIMPLE)/libinstrument.so\"

run:
	@for i in $(TARGETS); do \
		make run-simple run-real SOURCE=$$i DYNINST_ROOT=$(DYNINST_ROOT); \
	done

run-simple:	instrument-simple
	@echo ================================================================================
	@echo Running DynInst on `basename $(SOURCE)` for simple instrumentation
	@echo ================================================================================
	@echo
	@DYNINSTAPI_RT_LIB=$(DYNINST_RT) LD_LIBRARY_PATH=$(LDLIBPATH)$(LIBINSTRUMENT_SIMPLE) ./$(TARGET) || true
	@rm -f instrumented || true

run-real:	instrument
	@echo ================================================================================
	@echo Running DynInst on `basename $(SOURCE)` for real instrumentation
	@echo ================================================================================
	@echo
	@DYNINSTAPI_RT_LIB=$(DYNINST_RT) LD_LIBRARY_PATH=$(LDLIBPATH)$(LIBINSTRUMENT_REAL) ./$(TARGET) || true
	@rm -f instrumented || true

instrument:
	LD_PRELOAD=$(LD_PRELOAD) DYNINSTAPI_RT_LIB=$(DYNINST_RT) LD_LIBRARY_PATH=$(LDLIBPATH)$(LIBINSTRUMENT_REAL) ./my_dyninst $(SOURCE) $(TARGET) 1>&2

instrument-simple:
	LD_PRELOAD=$(LD_PRELOAD) DYNINSTAPI_RT_LIB=$(DYNINST_RT) LD_LIBRARY_PATH=$(LDLIBPATH)$(LIBINSTRUMENT_SIMPLE) ./my_dyninst_simple $(SOURCE) $(TARGET) 1>&2

clean:
	rm -f *.o $(BINARIES)