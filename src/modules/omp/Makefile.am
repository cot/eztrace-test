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
GTGDEPENDENCIES =
else
GTGCFLAGS = -I$(top_srcdir)/extlib/gtg/inc -I$(top_builddir)/extlib/gtg
GTGLDFLAGS = $(top_builddir)/extlib/gtg/src/libgtg.la
GTGDEPENDENCIES = $(GTGLDFLAGS)
endif

if GTG_OUT_OF_ORDER
GTGCFLAGS+=-DGTG_OUT_OF_ORDER
endif



OPARICFLAGS = -I$(top_srcdir)/extlib/opari/tool -I$(top_builddir)/extlib/opari/tool

bin_SCRIPTS = eztrace_cc

lib_LTLIBRARIES = libeztrace-convert-omp.la \
		  libeztrace-omp.la \
		  libeztrace-autostart-omp.la \
		  libpomp2.la

AM_CFLAGS = -W -Wall -Wextra

if OPENMP_FOUND
AM_CFLAGS+=-DOPENMP_FOUND
endif

AM_CFLAGS += -I$(srcdir)/../../core/ -I$(top_builddir)/src/core -I$(top_builddir)/src/pptrace

libpomp2_la_SOURCES = pomp2_lib.c
libpomp2_la_CFLAGS = $(AM_CFLAGS)


libeztrace_convert_omp_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(GTGCFLAGS)
libeztrace_convert_omp_la_CFLAGS = $(AM_CFLAGS)
libeztrace_convert_omp_la_LIBADD = $(FXT_LIB) $(GTG_LIB)
libeztrace_convert_omp_la_LDFLAGS = --no-undefined $(GTGLDFLAGS) $(FXTLDFLAGS) $(OPARICFLAGS)
libeztrace_convert_omp_la_DEPENDENCIES = $(GTGDEPENDENCIES)
libeztrace_convert_omp_la_SOURCES = eztrace_convert_gomp.c

libeztrace_omp_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(OPARICFLAGS)
libeztrace_omp_la_CFLAGS = $(AM_CFLAGS)
libeztrace_omp_la_LIBADD = -lm -lpthread $(FXT_LIB)
libeztrace_omp_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic
libeztrace_omp_la_SOURCES = gomp.c

libeztrace_autostart_omp_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(OPARICFLAGS)
libeztrace_autostart_omp_la_CFLAGS = -W -Wall -Wextra -DEZTRACE_AUTOSTART $(AM_CFLAGS)
libeztrace_autostart_omp_la_LIBADD =  -lm -lpthread $(FXT_LIB)
libeztrace_autostart_omp_la_LDFLAGS =  -rdynamic --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic
libeztrace_autostart_omp_la_SOURCES = gomp.c


noinst_HEADERS = gomp_ev_codes.h pomp2_lib.h
EXTRA_DIST = eztrace_cc.in
