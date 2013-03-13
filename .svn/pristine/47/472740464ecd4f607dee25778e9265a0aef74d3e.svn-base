# Copyright © CNRS, INRIA, Université Bordeaux 1
# See COPYING in top-level directory.

if AMEXTERNFXT
FXTCFLAGS=$(FXT_CPPFLAGS)
FXTLDFLAGS=$(FXT_LDFLAGS)
FXTLIBADD=$(FXT_LIB)
else
FXTCFLAGS = -I$(top_srcdir)/extlib/fxt/tools -I$(top_builddir)/extlib/fxt/tools
FXTLDFLAGS = $(top_builddir)/extlib/fxt/tools/libfxt.la
FXTLIBADD=
endif

if AMEXTERNGTG
GTGCFLAGS=$(GTG_CPPFLAGS)
GTGLDFLAGS=$(GTG_LDFLAGS)
GTGDEPENDENCIES =
GTGLIBADD=$(GTG_LIB)
else
GTGCFLAGS = -I$(top_srcdir)/extlib/gtg/inc -I$(top_builddir)/extlib/gtg
GTGLDFLAGS = $(top_builddir)/extlib/gtg/src/libgtg.la
GTGDEPENDENCIES = $(GTGLDFLAGS)
GTGLIBADD=
endif

if GTG_OUT_OF_ORDER
GTGCFLAGS+=-DGTG_OUT_OF_ORDER
endif

AM_CFLAGS = -W -Wall -Wextra -I$(srcdir)/../../core/ -I$(top_builddir)/src/core -I$(top_builddir)/src/pptrace $(FXTCFLAGS)

AM_CPPFLAGS =  $(FXTCFLAGS)
AM_LIBADD = $(FXT_LIB)
AM_LDFLAGS= $(FXTLDFLAGS)

CTESTS = teststdio \
	     testmemory \
	     teststatic
 
if OPENMP_FOUND
CTESTS += testopenmpsimple testopenmpnowait testopenmplock testopenmpnestlock
endif
if USE_MPI
CTESTS += testmpiring
endif
if USE_PTHREAD
CTESTS += testpthreaddummy_thread testpthreadtest testpthreadcond

#testpthreadcond_SOURCES = testpthreadcond.c
#testpthreadcond_DEPENDENCIES=../pthread/cond

#testpthreaddummy_thread_SOURCES = testpthreaddummy_thread.c
#testpthreaddummy_thread_DEPENDENCIES=../pthread/dummy_thread

#testpthreadtest_SOURCES = testpthreadtest.c
#testpthreadtest_DEPENDENCIES=../pthread/test_pthread
endif

check_PROGRAMS = $(CTESTS)
TESTS = $(CTESTS)

#teststdio_SOURCES = teststdio.c
#teststdio_DEPENDENCIES=../stdio/stdio

#teststatic_SOURCES = teststatic.c
#teststatic_DEPENDENCIES=../static/static
