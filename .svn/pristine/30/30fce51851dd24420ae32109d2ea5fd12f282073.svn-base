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

check_PROGRAMS = static

check_LTLIBRARIES =  libeztrace-convert-staticlib.la\
                     libeztrace-staticlib.la\
                     libeztrace-autostart-staticlib.la

static_SOURCES = static.c

AM_CFLAGS = -W -Wall -Wextra -I$(srcdir)/../../core/ -I$(top_builddir)/src/core -I$(top_builddir)/src/pptrace

libeztrace_convert_staticlib_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(GTGCFLAGS)
libeztrace_convert_staticlib_la_LIBADD = $(FXT_LIB) $(GTG_LIB)
libeztrace_convert_staticlib_la_LDFLAGS = $(GTGLDFLAGS) $(FXTLDFLAGS) -module -avoid-version -rpath /dev/null
libeztrace_convert_staticlib_la_DEPENDENCIES = $(GTGDEPENDENCIES)
libeztrace_convert_staticlib_la_SOURCES = eztrace_convert_staticlib.c

libeztrace_staticlib_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS)
libeztrace_staticlib_la_LIBADD = $(FXT_LIB)
libeztrace_staticlib_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic -module -avoid-version -rpath /dev/null
libeztrace_staticlib_la_SOURCES = staticlib.c

libeztrace_autostart_staticlib_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS)
libeztrace_autostart_staticlib_la_LIBADD = $(FXT_LIB)
libeztrace_autostart_staticlib_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic -module -avoid-version -rpath /dev/null
libeztrace_autostart_staticlib_la_CFLAGS = $(AM_CFLAGS) -W -Wall -Wextra -DEZTRACE_AUTOSTART 
libeztrace_autostart_staticlib_la_SOURCES = staticlib.c


noinst_HEADERS =  staticlib_ev_codes.h

