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

bin_PROGRAMS = eztrace_convert eztrace_avail eztrace_loaded eztrace_stats
if USE_PPTRACE
bin_PROGRAMS += eztrace
endif

generated_scripts = eztrace_create_plugin eztrace.old
bin_SCRIPTS = $(generated_scripts) function.pm input/Makefile.template  
check_PROGRAMS = eztrace



lib_LTLIBRARIES =  libeztrace.la \
		   libeztrace-autostart.la \
                   libeztrace-convert-core.la

eztrace_SOURCES = eztrace.c \
		  $(top_srcdir)/src/pptrace/pptrace.c \
		  $(top_srcdir)/src/pptrace/binary.c \
		  $(top_srcdir)/src/pptrace/errors.c \
		  $(top_srcdir)/src/pptrace/hijack.c \
		  $(top_srcdir)/src/pptrace/isize.c \
		  $(top_srcdir)/src/pptrace/memory.c \
		  $(top_srcdir)/src/pptrace/opcodes.c \
		  $(top_srcdir)/src/pptrace/tracing.c 
eztrace_CPPFLAGS = -I$(top_srcdir)/src/pptrace/ $(FXTCFLAGS) -g -O0
eztrace_LDFLAGS = -g -O0 $(FXTLDFLAGS)
eztrace_LDADD = -L${prefix}/lib -leztrace -ldl -lbfd $(FXTLIBADD)
if USE_LIBELF
eztrace_LDADD += -lelf
endif

eztrace_avail_SOURCES = eztrace_avail.c
eztrace_avail_CPPFLAGS = $(FXTCFLAGS)
eztrace_avail_LDFLAGS = $(FXTLDFLAGS)
eztrace_avail_LDADD = -ldl libeztrace-convert-core.la

eztrace_loaded_SOURCES = eztrace_loaded.c
eztrace_loaded_CPPFLAGS = $(FXTCFLAGS)
eztrace_loaded_LDFLAGS = $(FXTLDFLAGS)
eztrace_loaded_LDADD = -ldl libeztrace-convert-core.la

eztrace_stats_SOURCES = eztrace_stats.c
eztrace_stats_CPPFLAGS = $(FXTCFLAGS)
eztrace_stats_LDFLAGS = $(FXTLDFLAGS)
eztrace_stats_LDADD = -ldl libeztrace-convert-core.la

eztrace_convert_SOURCES = eztrace_convert.c \
		          eztrace_convert.h \
		          eztrace_convert_types.h \
		          eztrace_convert_macros.h \
		          ev_codes.h\
                          eztrace_hook.h

eztrace_convert_CPPFLAGS = $(GTGCFLAGS) $(FXTCFLAGS)
eztrace_convert_LDFLAGS = $(GTGLDFLAGS) $(FXTLDFLAGS)
eztrace_convert_LDADD = $(GTGLIBADD) -ldl libeztrace-convert-core.la
eztrace_convert_DEPENDENCIES = $(GTGDEPENDENCIES)




libeztrace_convert_core_la_CPPFLAGS = $(FXTCFLAGS) $(GTGCFLAGS) -DEZTRACE_LIB_DIR=\"${libdir}\" -DDYNLIB_EXT=\"${DYNLIB_EXT}\"

libeztrace_convert_core_la_LIBADD = -lm -lpthread $(FXTLIBADD) $(GTGLIBADD)
libeztrace_convert_core_la_LDFLAGS = --no-undefined $(GTGLDFLAGS) $(FXTLDFLAGS)
libeztrace_convert_core_la_DEPENDENCIES = $(GTGDEPENDENCIES)
libeztrace_convert_core_la_SOURCES = eztrace_convert_core.c \
                                     eztrace_stats_core.c \
                                     eztrace_stats_core.h \
                                     eztrace_hierarchical_array.c \
                                     eztrace_hierarchical_array.h \
                                     eztrace_array.c \
                                     eztrace_array.h \
		          	     eztrace_convert_types.h \
		          	     eztrace_convert_macros.h \
				     eztrace_convert.h


libeztrace_la_CPPFLAGS = $(FXTCFLAGS)
libeztrace_la_LIBADD = -lm -lpthread $(FXTLIBADD)
libeztrace_la_LDFLAGS = --no-undefined $(FXTLDFLAGS)


libeztrace_la_SOURCES = eztrace_core.c \
		        pthread_core.c 

libeztrace_autostart_la_CPPFLAGS = $(FXTCFLAGS)
libeztrace_autostart_la_CFLAGS = -DEZTRACE_AUTOSTART
libeztrace_autostart_la_LIBADD = -lm -lpthread $(FXTLIBADD)
libeztrace_autostart_la_LDFLAGS = --no-undefined $(FXTLDFLAGS)
libeztrace_autostart_la_SOURCES = eztrace_core.c \
                                  pthread_core.c

inputdir   = $(datadir)/eztrace

input_DATA =      input/example.c.template\
                  input/example_ev_codes.h.template\
                  input/eztrace_convert_example.c.template \
                  input/Makefile.template

EXTRA_DIST = $(input_DATA) eztrace_create_plugin.in function.pm eztrace.c.in  eztrace.old.in
CLEANFILES = $(generated_scripts) eztrace config.h

include_HEADERS = eztrace.h\
		  pptrace.h \
		  config.h \
		  ev_codes.h \
		  eztrace_list.h \
		  eztrace_stack.h \
		  eztrace_hook.h \
                  eztrace_convert.h\
                  eztrace_convert_core.h\
                  eztrace_convert_macros.h\
                  eztrace_convert_types.h
