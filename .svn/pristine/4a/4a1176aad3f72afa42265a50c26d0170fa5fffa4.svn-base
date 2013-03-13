# Copyright © CNRS, INRIA, Université Bordeaux 1
# See COPYING in top-level directory.

if AMEXTERNFXT
FXTCFLAGS=$(FXT_CPPFLAGS)
FXTLDFLAGS=$(FXT_LDFLAGS)
else
FXTCFLAGS = -I$(top_srcdir)/extlib/fxt/tools -I$(top_builddir)/extlib/fxt/tools
FXTLDFLAGS = $(top_builddir)/extlib/fxt/tools/libfxt.la
endif

if AMEXTERNGTG
GTGCFLAGS=$(GTG_CPPFLAGS)
GTGLDFLAGS=$(GTG_LDFLAGS)
else
GTGCFLAGS = -I$(top_srcdir)/extlib/gtg/tools -I$(top_builddir)/extlib/gtg
GTGLDFLAGS = $(top_builddir)/extlib/gtg/libgtg.la
endif

SUBDIRS = extlib/ \
	  src/core/ \
	  src/modules/omp \
	  src/modules/mpi \
	  src/modules/pthread \
	  src/modules/stdio \
	  src/modules/memory \
	  src/modules/papi \
	  test/mpi \
	  test/openmp \
	  test/pptrace/automatic \
	  test/static \
	  test/stdio \
	  test/memory \
	  test/pthread \
	  test/automake


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eztrace.pc

EXTRA_DIST = \
bootstrap \
ChangeLog \
test/mpi/Makefile.am \
test/mpi/mpi_openmp.c \
test/mpi/mpi_ring.c \
test/mpi/mpi_ping.c \
test/mpi/mpi_spawn.c \
test/mpi/persistent.c \
test/mpi/test_mpi.c \
test/mpi/test_mpi_f.f \
test/mpi/mpi_multiring.c \
test/mpi/waitany.c \
test/mpi/allgather.c \
test/mpi/waitall.c \
test/openmp/Makefile.am \
test/openmp/simple.c \
test/openmp/openmp_nowait.f \
test/openmp/openmp_nowait_c.c \
test/openmp/test_task.c \
test/pthread/cond.c \
test/pthread/dummy_thread.c \
test/pthread/test_pthread.c \
test/pthread/test_sigsegv.c \
test/stdio/stdio.c \
test/memory/memory.c \
test/module_generator/example.tpl \
test/module_generator/example_application/example.c \
test/module_generator/example_application/example.h \
test/module_generator/example_application/main.c \
test/module_generator/example_application/Makefile \
example/README \
example/libexample_eztrace/example.c \
example/libexample_eztrace/example_ev_codes.h \
example/libexample_eztrace/eztrace_convert_example.c \
example/libexample_eztrace/Makefile \
example/libexample_orig/static_example.c \
example/libexample_orig/example.c \
example/libexample_orig/example.h \
example/libexample_orig/main.c \
example/libexample_orig/Makefile \
test/automake/testcommon.h \
test/static/Makefile.in \
test/static/static.c \
test/static/staticlib.c \
test/static/staticlib_ev_codes.h \
test/static/eztrace_convert_staticlib.c \
test/pptrace/automatic/binary.c \
test/pptrace/automatic/common.h \
test/pptrace/automatic/errors.c \
test/pptrace/automatic/hijack.c \
test/pptrace/automatic/Makefile.in \
test/pptrace/automatic/opcodes.c \
test/pptrace/automatic/tracing.c \
test/pptrace/automatic/wait_open.c \
test/pptrace/automatic/testcase/Makefile \
test/pptrace/automatic/testcase/titi.c

include_HEADERS = src/pptrace/pptrace.h \
                  src/pptrace/opcodes.h \
                  src/pptrace/memory.h \
                  src/pptrace/hijack.h \
                  src/pptrace/isize.h \
                  src/pptrace/binary.h \
                  src/pptrace/errors.h \
                  src/pptrace/tracing.h \
                  src/pptrace/tracing/linux.h \
                  src/pptrace/tracing/linux.c \
                  src/pptrace/binary/bfd.c \
                  src/pptrace/binary/elf.c \
                  src/pptrace/isize/trace.c \
                  src/pptrace/isize/opcode.c \
                  src/pptrace/opcodes/intel.c


#CTESTS = test/automake/teststdio \
#test/automake/testmemory \
#test/automake/teststatic
#if OPENMP_FOUND
#CTESTS += test/automake/testopenmpsimple test/automake/testopenmpnowait test/automake/testopenmplock test/automake/testopenmpnestlock 	
#endif
#if USE_MPI
#CTESTS += test/automake/testmpiring
#endif
#if USE_PTHREAD
#CTESTS += test/automake/testpthreaddummy_thread test/automake/testpthreadtest test/automake/testpthreadcond	
#endif

SHELLTESTS = test/pptrace/automatic/errors.test \
test/pptrace/automatic/binary.test \
test/pptrace/automatic/tracing.test \
test/pptrace/automatic/opcodes.test \
test/pptrace/automatic/wait_open.test \
test/pptrace/automatic/hijack.test

#TESTS=$(SHELLTESTS) $(CTESTS)

#check_PROGRAMS=$(CTESTS) test/stdio/stdio \
#test/memory/memory \
#test/static/static 

#if USE_PTHREAD
#check_PROGRAMS += test/pthread/cond test/pthread/dummy_thread test/pthread/test_pthread
#endif
	
#TESTS_ENVIRONMENT = prefix = "$(top_builddir)/src"
#TESTS_ENVIRONMENT += libdir = "$(top_builddir)/src/modules" 
