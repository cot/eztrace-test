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

AM_CFLAGS = -I$(srcdir)/../../core/ -I$(top_builddir)/src/core -I$(top_builddir)/src/pptrace

lib_LTLIBRARIES =

if USE_MPI
lib_LTLIBRARIES += libeztrace-convert-mpi.la \
		   libeztrace-mpi.la \
		   libeztrace-autostart-mpi.la
AM_CFLAGS += -W -Wall -Wextra -DUSE_MPI
endif


libeztrace_convert_mpi_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) $(GTGCFLAGS)
libeztrace_convert_mpi_la_LIBADD = $(FXT_LIB) $(GTG_LIB)
libeztrace_convert_mpi_la_LDFLAGS = --no-undefined $(GTGLDFLAGS) $(FXTLDFLAGS)
libeztrace_convert_mpi_la_SOURCES = eztrace_convert_mpi.c \
			  	    eztrace_convert_mpi.h\
	 			    mpi_ev_codes.h\
	 			    eztrace_convert_mpi_p2p.c\
	 			    eztrace_convert_mpi_p2p.h\
	 			    eztrace_convert_mpi_coll.c\
	 			    eztrace_convert_mpi_coll.h\
	 			    eztrace_convert_mpi_pers.c\
	 			    eztrace_convert_mpi_pers.h\
	 			    eztrace_stats_mpi.h\
	 			    eztrace_stats_mpi.c

libeztrace_convert_mpi_la_DEPENDENCIES = $(GTGDEPENDENCIES)


mpi_function_dir = mpi_funcs
mpi_function_files = $(mpi_function_dir)/mpi_allgather.c       \
		     $(mpi_function_dir)/mpi_allgatherv.c      \
		     $(mpi_function_dir)/mpi_allreduce.c       \
		     $(mpi_function_dir)/mpi_alltoall.c        \
		     $(mpi_function_dir)/mpi_alltoallv.c       \
		     $(mpi_function_dir)/mpi_barrier.c         \
		     $(mpi_function_dir)/mpi_bcast.c           \
		     $(mpi_function_dir)/mpi_bsend.c           \
		     $(mpi_function_dir)/mpi_bsend_init.c      \
		     $(mpi_function_dir)/mpi_gather.c          \
		     $(mpi_function_dir)/mpi_gatherv.c         \
		     $(mpi_function_dir)/mpi_get.c             \
		     $(mpi_function_dir)/mpi_ibsend.c          \
		     $(mpi_function_dir)/mpi_iprobe.c          \
		     $(mpi_function_dir)/mpi_irecv.c           \
		     $(mpi_function_dir)/mpi_irsend.c          \
		     $(mpi_function_dir)/mpi_isend.c           \
		     $(mpi_function_dir)/mpi_issend.c          \
		     $(mpi_function_dir)/mpi_probe.c           \
		     $(mpi_function_dir)/mpi_put.c             \
		     $(mpi_function_dir)/mpi_recv.c	       \
		     $(mpi_function_dir)/mpi_recv_init.c       \
		     $(mpi_function_dir)/mpi_reduce.c          \
		     $(mpi_function_dir)/mpi_reduce_scatter.c  \
		     $(mpi_function_dir)/mpi_rsend.c           \
		     $(mpi_function_dir)/mpi_rsend_init.c      \
		     $(mpi_function_dir)/mpi_scan.c            \
		     $(mpi_function_dir)/mpi_scatter.c         \
		     $(mpi_function_dir)/mpi_scatterv.c        \
		     $(mpi_function_dir)/mpi_send.c            \
		     $(mpi_function_dir)/mpi_send_init.c       \
		     $(mpi_function_dir)/mpi_sendrecv.c        \
		     $(mpi_function_dir)/mpi_sendrecv_replace.c\
		     $(mpi_function_dir)/mpi_ssend.c           \
		     $(mpi_function_dir)/mpi_ssend_init.c      \
		     $(mpi_function_dir)/mpi_start.c           \
		     $(mpi_function_dir)/mpi_startall.c        \
		     $(mpi_function_dir)/mpi_test.c            \
		     $(mpi_function_dir)/mpi_testall.c         \
		     $(mpi_function_dir)/mpi_testany.c         \
		     $(mpi_function_dir)/mpi_testsome.c        \
		     $(mpi_function_dir)/mpi_wait.c            \
		     $(mpi_function_dir)/mpi_waitall.c         \
		     $(mpi_function_dir)/mpi_waitany.c         \
		     $(mpi_function_dir)/mpi_waitsome.c

libeztrace_mpi_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) -DLD_PRELOAD_NAME=\"@LD_PRELOAD_NAME@\"
libeztrace_mpi_la_LIBADD = -lm -lpthread $(FXT_LIB)
libeztrace_mpi_la_LDFLAGS = --no-undefined $(FXTLDFLAGS) -Wl,-Bsymbolic
libeztrace_mpi_la_SOURCES = mpi.c mpi_fortran.c mpi_f.f90 mpi_ev_codes.h mpi_eztrace.h $(mpi_function_files)

libeztrace_autostart_mpi_la_CPPFLAGS = $(AM_CPPFLAGS) $(FXTCFLAGS) -DLD_PRELOAD_NAME=\"@LD_PRELOAD_NAME@\"
libeztrace_autostart_mpi_la_LIBADD = -lm -lpthread $(FXT_LIB)
libeztrace_autostart_mpi_la_LDFLAGS = $(FXTLDFLAGS) -rdynamic -Wl,-Bsymbolic
libeztrace_autostart_mpi_la_CFLAGS = $(AM_CFLAGS) -W -Wall -Wextra -DEZTRACE_AUTOSTART
libeztrace_autostart_mpi_la_SOURCES = mpi.c mpi_fortran.c mpi_f.f90 mpi_ev_codes.h mpi_eztrace.h $(mpi_function_files)

#noinst_HEADERS =  eztrace_convert_mpi.h \
#		  mpi_ev_codes.h
