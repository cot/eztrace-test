
ldflags =

if FXT_HAVE_WINDOWS
ldflags += -Xlinker --output-def -Xlinker .libs/libfxt.def

if FXT_HAVE_MS_LIB
.libs/libfxt-0.lib: libfxt.la dolib
	./dolib "$(FXT_MS_LIB)" $(FXT_MS_LIB_ARCH) .libs/libfxt.def libfxt-0 .libs/libfxt-0.lib
all-local: .libs/libfxt-0.lib
endif FXT_HAVE_MS_LIB

install-exec-hook:
	$(INSTALL) .libs/libfxt.def $(DESTDIR)$(libdir)
if FXT_HAVE_MS_LIB
	$(INSTALL) .libs/libfxt-0.lib $(DESTDIR)$(libdir)
	$(INSTALL) .libs/libfxt-0.exp $(DESTDIR)$(libdir)
endif FXT_HAVE_MS_LIB

endif

# Flags globaux
AM_CFLAGS= $(shell getconf LFS_CFLAGS)
AM_LDFLAGS= $(shell getconf LFS_LDFLAGS) 
AM_CPPFLAGS= -I$(srcdir)/../kernel/linux-fkt/include
libfxt_la_LDFLAGS = $(ldflags) -no-undefined

noinst_HEADERS = fxt_internal.h fut-template.h timer.h
dist_noinst_SCRIPTS = template2h.pl makediff
include_HEADERS = fxt-tools.h fxt.h fut.h
pkginclude_HEADERS = fxt-tools.h fxt.h fut.h
dist_bin_SCRIPTS = sigmund
#noinst_PROGRAMS = fut_bench

EXTRA_DIST = fkt_bufstats fkt_timestats fkt_timestats-log time.c timer.c tbx_timing.h dolib.c

# Les programmes
bin_PROGRAMS = fxt_print
if FKT
bin_PROGRAMS += fkt_record fkt_setmask fkt_extract fkt_print
if X11
bin_PROGRAMS += fkt_select
endif
SYMLINKS_PROGS = fkt_disable fkt_enable fkt_probe0
endif
install-exec-local:
	set -e; \
	cd $(DESTDIR)$(bindir) ; \
	$(foreach prog, $(SYMLINKS_PROGS), \
	  $(RM) $(prog)$(EXEEXT) ; \
	  $(LN_S) fkt_setmask$(EXEEXT) $(prog)$(EXEEXT) ;)

fxt_print_LDADD = -lfxt $(FXT_LIBS)
fxt_print.$(OBJEXT): fut.h fut_print.h
fxt_print_DEPENDENCIES = libfxt.la

fkt_print_LDADD = -lfxt $(FXT_LIBS)
fkt_print.$(OBJEXT): fut.h fut_print.h
fkt_print_DEPENDENCIES = libfxt.la

fkt_record_LDADD = -lfxt $(FXT_LIBS)
fkt_select_LDADD = -L/usr/X11R6/lib -lX11

#fut_bench_LDADD = -lfxt $(FXT_LIBS) -lpthread

ev.lo: fut.h

# La bibliothèque
lib_LTLIBRARIES = libfxt.la

libfxt_la_SOURCES = fxt.c names.c symbols.c pids.c get_cpu_info.c ev.c fut_setup.c fut_record.c
libfxt_la_SOURCES += fxt/trap_names.h fxt/sysirq_names.h fxt/syscall_names.h fkt_code_name.h 

# Les trucs spéciaux
CLEANFILES = fut.h fut_print.h
BUILT_SOURCES = fut.h fut_print.h
fut.h: fut-template.h template2h.pl
	./template2h.pl < $< > $@
fut_print.h: fut.h
	$(AWK) 'BEGIN \
			{ print "/* this is a generated file,"\
				"do not edit it */" \
			}\
		/^#define.*FUT_.*_CODE/ \
			{ print "{ "$$2", \"" \
				tolower(gensub("_CODE$$","","g",$$2)) \
				"\"}," \
			}' \
		$< > $@ || ( $(RM) -f $@ ; exit 1 )
