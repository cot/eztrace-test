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

if USE_PAPI

lib_LTLIBRARIES = libeztrace-convert-papi.la \
		  libeztrace-papi.la \
		  libeztrace-autostart-papi.la

AM_CFLAGS = -W -Wall -Wextra -I$(srcdir)/../../core/ -I$(top_builddir)/src/core -I$(top_builddir)/src/pptrace


libeztrace_convert_papi_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(GTGCFLAGS) $(PAPI_CPPFLAGS)
libeztrace_convert_papi_la_LIBADD = $(FXT_LIB) $(GTG_LIB) -lpapi
libeztrace_convert_papi_la_LDFLAGS = $(GTGLDFLAGS) $(FXTLDFLAGS) $(PAPI_LDFLAGS)
libeztrace_convert_papi_la_DEPENDENCIES = $(GTGDEPENDENCIES)
libeztrace_convert_papi_la_SOURCES = eztrace_convert_papi.c

libeztrace_papi_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(PAPI_CPPFLAGS)
libeztrace_papi_la_LIBADD = $(FXT_LIB) -lpapi
libeztrace_papi_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) $(PAPI_LDFLAGS) -Wl,-Bsymbolic
libeztrace_papi_la_SOURCES = papi.c

libeztrace_autostart_papi_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(PAPI_CPPFLAGS)
libeztrace_autostart_papi_la_LIBADD = $(FXT_LIB) -lpapi
libeztrace_autostart_papi_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) $(PAPI_LDFLAGS) -Wl,-Bsymbolic
libeztrace_autostart_papi_la_CFLAGS = $(AM_CFLAGS) -W -Wall -Wextra -DEZTRACE_AUTOSTART
libeztrace_autostart_papi_la_SOURCES = papi.c

noinst_HEADERS =  papi_ev_codes.h
endif
