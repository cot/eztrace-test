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

lib_LTLIBRARIES = libeztrace-convert-stdio.la \
		  libeztrace-stdio.la \
		  libeztrace-autostart-stdio.la

AM_CFLAGS = -W -Wall -Wextra -I$(srcdir)/../../core/ -I$(top_builddir)/src/core -I$(top_builddir)/src/pptrace


libeztrace_convert_stdio_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(GTGCFLAGS)
libeztrace_convert_stdio_la_LIBADD = $(FXT_LIB) $(GTG_LIB)
libeztrace_convert_stdio_la_LDFLAGS = $(GTGLDFLAGS) $(FXTLDFLAGS)
libeztrace_convert_stdio_la_DEPENDENCIES = $(GTGDEPENDENCIES)
libeztrace_convert_stdio_la_SOURCES = eztrace_convert_stdio.c

libeztrace_stdio_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS)
libeztrace_stdio_la_LIBADD = $(FXT_LIB)
libeztrace_stdio_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic
libeztrace_stdio_la_SOURCES = stdio.c

libeztrace_autostart_stdio_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS)
libeztrace_autostart_stdio_la_LIBADD = $(FXT_LIB)
libeztrace_autostart_stdio_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic
libeztrace_autostart_stdio_la_CFLAGS = $(AM_CFLAGS) -W -Wall -Wextra -DEZTRACE_AUTOSTART 
libeztrace_autostart_stdio_la_SOURCES = stdio.c

noinst_HEADERS =  stdio_ev_codes.h
