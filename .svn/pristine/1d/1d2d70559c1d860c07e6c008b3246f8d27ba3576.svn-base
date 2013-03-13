AUTOMAKE_OPTIONS = -Wno-gnu

bin_PROGRAMS = opari
lib_LTLIBRARIES =  libpomp.la

# CC = $(CC_FOR_BUILD)
# CFLAGS = $(CFLAGS_FOR_BUILD)
# CPP = $(CPP_FOR_BUILD)
# CXX = $(CXX_FOR_BUILD)
# CXXFLAGS = $(CXXFLAGS_FOR_BUILD)
# CXXCPP = $(CXXCPP_FOR_BUILD)
# CPPFLAGS = $(CPPFLAGS_FOR_BUILD)
# LDFLAGS = $(LDFLAGS_FOR_BUILD)
# LIBS = $(LIBS_FOR_BUILD)


POMP_DIR=../eztrace_pomp/

opari_SOURCES = \
	handler.h \
	ompragma.h \
	ompregion.h \
	opari.h \
	handler.cc \
	ompragma.cc \
	ompragma_c.cc \
	ompragma_f.cc \
	opari.cc \
	process_c.cc \
	process_f.cc \
	process_omp.cc

opari_LDADD = libpomp.la

libpomp_la_SOURCES = pomp_lib.c \
		     pomp_lib.h \
		     pomp_flib.f90

include_HEADERS = pomp_lib.h

EXTRA_DIST = \
	fixinc/cctype \
	fixinc/cstdio \
	fixinc/cstdlib \
	fixinc/cstring


