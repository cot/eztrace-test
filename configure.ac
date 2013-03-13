# Copyright � CNRS, INRIA, Universit� Bordeaux 1
# See COPYING in top-level directory.

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([eztrace], [0.8.1], [eztrace-devel@lists.gforge.inria.fr])
AM_INIT_AUTOMAKE([1.10 tar-pax color-tests])

# Checks for programs.
AC_PROG_CC
AC_PROG_FC
AC_PROG_CXX
AC_LANG_CPLUSPLUS
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_CC_C_O


F77=$(FC)
FFLAGS=$(FCFLAGS)

AC_PATH_PROG([MPICC], [mpicc])
AC_PATH_PROG(PERL,perl)
#AC_CHECK_PROG(PERL,perl,yes,missing -- the perl scripting language)

# crappy hack: of the macros AC_PROG_{CC,CXX,F77,FC}, only the
# first expanded one will check for object file extension etc,
# and fail if that doesn't work.
if test x$FC = x; then
AC_MSG_ERROR(A Fortran compiler is required)
fi

cxx_enable=yes
if test x$CXX = x; then
cxx_enable=no
fi


# Correct install paths
AS_IF([test x"$prefix" = x -o x"$prefix" = "xNONE"], [prefix="/usr/local"])
AS_IF([test x"$exec_prefix" = x -o x"$exec_prefix" = "xNONE"], [exec_prefix="\${prefix}"])
AS_IF([test "$docdir" = "\${datarootdir}/doc/\${PACKAGE_TARNAME}"],
[docdir="\${datarootdir}/doc"])



# Checks for libraries.
AC_CHECK_LIB([dl], [dlsym])
AC_CHECK_LIB([m], [cos])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([pthread], [pthread_spin_lock], have_full_pthread=yes, have_full_pthread=no)

# Check the OS type
os_type=`uname -s`

if [ test "x$os_type" = "xLinux" ]; then
# Linux
  AC_MSG_WARN([OS: Linux])
  CFLAGS="$CFLAGS -DHAVE_LINUX"
  DYNLIB_EXT=".so"
  LD_PRELOAD_NAME="LD_PRELOAD"
  LD_LIBRARY_PATH_NAME="LD_LIBRARY_PATH"
elif [ test "x$os_type" = "xDarwin" ]; then
# Mac OS
  AC_MSG_WARN([OS: Mac OS])
  CFLAGS="$CFLAGS -DHAVE_DARWIN"
  DYNLIB_EXT=".dylib"
  LD_PRELOAD_NAME="DYLD_INSERT_LIBRARIES"
  LD_LIBRARY_PATH_NAME="DYLD_LIBRARY_PATH"
else
# unknown OS, let's try Linux
  AC_MSG_WARN([unknown OS: falling back to Linux])
  CFLAGS="$CFLAGS -DHAVE_LINUX"
  DYNLIB_EXT=".so"
  LD_PRELOAD_NAME="LD_PRELOAD"
  LD_LIBRARY_PATH_NAME="LD_LIBRARY_PATH"
fi


# Check for a library with a particular LDFLAGS
# this LDFLAGS is not set in the 'final' LDFLAGS
# This is usefull if only a small part of your program
# needs this library
#
# example:
#CHECK_LIB_LDFLAGS([gtg],
#		   [setTraceType],
#                  "$GTG_LDFLAGS",
#                   ,
#                   AC_MSG_ERROR([cannot find libgtg]))
AC_DEFUN([CHECK_LIB_LDFLAGS],
[save_LDFLAGS="$LDFLAGS"
save_LIBS="$LIBS"
LDFLAGS="$LDFLAGS $3"
AC_CHECK_LIB([$1], [$2], [$4], [$5])
LDFLAGS="$save_LDFLAGS"
LIBS="$save_LIBS"
])


# Check for a library with a particular LDFLAGS
# this LDFLAGS is not set in the 'final' LDFLAGS
# This is usefull if only a small part of your program
# needs this library
#
# example:
#CHECK_LIB_TRYLINK("$GTG_LDFLAGS",
#                  [#include "GTG.h"],
#		   [gtg_container_t c;],
#                  [enable_gtg=yes],
#                   AC_MSG_ERROR([cannot find libgtg]))
AC_DEFUN([CHECK_LIB_TRYLINK],
[save_LDFLAGS="$LDFLAGS"
save_LIBS="$LIBS"
LDFLAGS="$LDFLAGS $1"
AC_TRY_LINK([$2], [$3], [$4], [$5])
LDFLAGS="$save_LDFLAGS"
LIBS="$save_LIBS"
])


# Check for a header with a particular CPPFLAGS
# this CPPFLAGS is not set in the 'final' CPPFLAGS
# This is usefull if only a small part of your program
# needs this header
#
# example:
# CHECK_HEADER_CPPFLAGS([GTGBasic1.h],
#                       ,
#                       "$GTG_CPPFLAGS",
#                        AC_MSG_ERROR([cannot find headers for gtg]))
AC_DEFUN([CHECK_HEADER_CPPFLAGS],
[save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $3"
AC_CHECK_HEADERS([$1], [$2], [$4], [$5])
CPPFLAGS="$save_CPPFLAGS"
])


# Checks for programs
AC_CHECK_PROG(HAVE_ENV, [env], yes, no)

if test x$HAVE_ENV = xno ; then
   AC_MSG_ERROR([Cannot find env])
fi

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h stdint.h stdlib.h string.h sys/time.h unistd.h sys/syscall.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit])


# pptrace stuff
pptrace_lib_found=no
pptrace_binary_type=0
AC_ARG_WITH([pptrace-binary-type],
            [AS_HELP_STRING([--with-pptrace-binary-type],
               [use pptrace-binary-type (bfd or elf)])],
            [
               if test x$withval = xelf; then
                     AC_CHECK_LIB(elf, elf_begin, [pptrace_binary_type=2], AC_MSG_ERROR([Error! You need to have libelf]) )
                     AC_MSG_RESULT(using elf binary type)
               else if test x$withval = xbfd; then
                        AC_CHECK_LIB(bfd, bfd_release, [pptrace_binary_type=1], AC_MSG_ERROR([Error! You need to have libbfd]) )
                        AC_MSG_RESULT(using bfd binary type)
                     else
				            AC_MSG_ERROR([Unknown binary type])
                     fi
               fi
            ],
            [
               AC_CHECK_LIB(bfd, bfd_release, 
               [
	          pptrace_lib_found=yes
                  AC_MSG_RESULT(bfd is present: use bfd binary type)
                  pptrace_binary_type=1], 
               [
                  AC_CHECK_LIB(elf, elf_begin, 
                  [                  
	             pptrace_lib_found=yes
                     pptrace_binary_type=2
                     AC_MSG_RESULT(bfd not present: use elf binary type)
                  ],
                  [
		       PPTRACE_LIB_FOUND=no
                  ])
               ])
            ])
AM_CONDITIONAL([USE_PPTRACE], [test "$pptrace_lib_found" = yes])
AM_CONDITIONAL([USE_LIBELF], [test "$pptrace_binary_type" = 2])
AC_SUBST(PPTRACE_BINARY_TYPE, $pptrace_binary_type)

AC_CHECK_HEADER([sys/prctl.h], 
                [
                AC_CHECK_FUNC(prctl, 
                              [have_prctl_h=1], 
                              [have_prctl_h=0])
                ],
                [have_prctl_h=0])
AC_SUBST(HAVE_PRCTL_H, [$have_prctl_h])

# -m32 & -m64 flags for tests
AC_MSG_CHECKING([whether $CC supports -m32 and -m64])
_save_cflags="$CFLAGS"
_save_ldflags="$LDFLAGS"
CFLAGS="$CFLAGS -m32"
LDFLAGS="$LDFLAGS -m32"
AC_LINK_IFELSE([#include <stdio.h>
	int main(void) { printf("Hello, World!\n"); return 0;}],
	[m32_flag=yes], [m32_flag=no]);
CFLAGS="$_save_cflags -m64"
LDFLAGS="$_save_ldflags -m64"
AC_LINK_IFELSE([#include <stdio.h>
	int main(void) { printf("Hello, World!\n"); return 0;}],
	[m64_flag=yes], [m64_flag=no]);
CFLAGS="$_save_cflags"
LDFLAGS="$_save_ldflags"
if test "x$m64_flag" = "xyes"; then
	if test "x$m32_flag" = "xyes"; then
		available_target_bits="32 64"
		supported_mXX_flags="-m32 -m64"
	else
		available_target_bits="64"
		supported_mXX_flags="-m64"
	fi
else if test "x$m32_flag" = "xyes"; then
		available_target_bits="32"
		supported_mXX_flags="-m32"
	else
		available_target_bits=""
		supported_mXX_flags=""
	fi
fi
AC_MSG_RESULT([supports $supported_mXX_flags])
AC_SUBST(TARGET_BITS, $available_target_bits)


# FxT stuff
use_extern_fxt=no
AC_ARG_WITH(fxt, [AS_HELP_STRING([--with-fxt[=<dir>]], [generate fxt traces])],
        [
                if test x$withval != xno; then
                        if test x$withval = xyes; then
                                AC_MSG_RESULT(yes)
                        else
                                # use specified path
				use_extern_fxt=yes
                                fxtdir=$withval
                                AC_MSG_RESULT(yes using $fxtdir)
                                AC_SUBST(FXTDIR, $fxtdir)
                        fi
                else
                        AC_MSG_RESULT(no)
			AC_MSG_ERROR([Cannot find FxT])
                fi
        ])


if test x$use_extern_fxt = xyes; then
        fxt_found=yes
        FXT_CPPFLAGS="-I$fxtdir/include/ -DEXTERNAL_FXT"
	FXT_LDFLAGS="-L$fxtdir/lib/ -lfxt"

else
    fxt_found=no
  dnl configure fxt from extlib
    fxt_parent_dir=`pwd`
    fxt_dir="extlib/fxt"
    AC_MSG_NOTICE([configuring in fxt_dir ($fxt_parent_dir/$fxt_dir)])
    AS_IF([test "$srcdir" != "."],
                [
                        test -d "$fxt_dir" ||
                        mkdir -p "$fxt_dir" ||
                        AC_MSG_ERROR([cannot create $fxt_dir])
                ])

                cd $fxt_dir

                case $srcdir in
                        .)
                                fxt_srcdir="$srcdir"
                                ;;
                        /*)
                                fxt_srcdir="$srcdir/$fxt_dir"
                                ;;
                        *)
                                fxt_srcdir="../../$srcdir/$fxt_dir"
                                ;;
                esac

                fxt_conf_cmd="$fxt_srcdir/configure"
		fxt_conf_args="$fxt_conf_args --prefix=\"$prefix\" --exec-prefix=\"$exec_prefix\" --bindir=\"$bindir\" --libdir=\"$libdir\" --includedir=\"$includedir\" --docdir=\"$docdir/fxt\" $FXTFLAGS --cache-file=\"/dev/null\" --srcdir=\"$fxt_srcdir\""
                AC_MSG_NOTICE([running $SHELL $fxt_conf_cmd $fxt_conf_args])
                eval "$SHELL '$fxt_conf_cmd' $fxt_conf_args"
                AS_IF([test $? != "0"], [AC_MSG_ERROR([$fxt_conf_cmd failed for $fxt_dir])])

                cd $fxt_parent_dir
		TID_RECORDING_ENABLED=yes
                AS_IF([test x"$FXTLIB" = x], [FXT_LDFLAGS="-lfxt"])

		FXT_CPPFLAGS="$FXT_CPPFLAGS -I${prefix}/include/fxt"
fi

# FxT
if test x$fxt_found = xyes ; then
  CPPFLAGS="${CPPFLAGS} ${FXT_CPPFLAGS}"
  LDFLAGS="${LDFLAGS} ${FXT_LDFLAGS}"
  AC_CHECK_LIB(fxt, fut_setup,,AC_MSG_ERROR([cannot find fxt lib]))
  AC_CHECK_HEADER([fxt/fxt.h],,AC_MSG_ERROR([cannot find headers for fxt]))
  AC_CHECK_HEADER([fxt/fut.h],,AC_MSG_ERROR([cannot find headers for fxt]))
  AC_CHECK_FUNC(fut_enable_tid_logging,[TID_RECORDING_ENABLED=yes],[TID_RECORDING_ENABLED=no])
fi

if test "x$TID_RECORDING_ENABLED" = xyes ; then
   CFLAGS="$CFLAGS -DTID_RECORDING_ENABLED"
   FXT_CPPFLAGS="${FXT_CPPFLAGS} -DTID_RECORDING_ENABLED"
fi




# check for openmp
CHECK_HEADER_CPPFLAGS([omp.h], [openmp_found=yes], "-fopenmp", AC_MSG_WARN([cannot find omp.h]))


use_papiB=yes
AC_ARG_WITH(papi, [AS_HELP_STRING([--with-papi[=<dir>]], [PAPI])],
        [
                if test x$withval != xno; then
                        use_papi=yes
                        if test x$withval = xyes; then
                                AC_MSG_RESULT(yes)
                                use_papi_from_system=yes
                        else
                                # use specified path
                                use_papi_from_system=no
                                papidir=$withval
                                AC_MSG_RESULT(yes using $papidir)
                                AC_SUBST(PAPIDIR, $papidir)
                        fi
                else
                        use_papi=no
                        AC_MSG_RESULT(no)
                fi
        ])

AC_SUBST(USE_PAPIDIR_FROM_USER,$use_papi_from_system)
if test x$use_papi_from_system = xno; then
   PAPI_LIBDIR=$papidir/lib
   PAPI_CPPFLAGS="-I$papidir/include/"
   PAPI_LDFLAGS="-L${PAPI_LIBDIR} -Xlinker -rpath=${PAPI_LIBDIR}"
elif test x$use_papi = xyes ; then
     if test -d "/usr/include/papi" ; then
        PAPI_CPPFLAGS="-I/usr/include/papi/"
     fi

     if test -d "/usr/lib/PAPI" ; then
        PAPI_LIBDIR=/usr/lib/papi
	PAPI_LDFLAGS="-L/usr/lib/papi -Xlinker -rpath=/usr/lib/papi"
     fi
fi

enable_papi=no

if test x$use_papi = xyes ; then
   save_CPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS $PAPI_CPPFLAGS"
   CHECK_LIB_TRYLINK([$PAPI_LDFLAGS -lpapi],
               [#include <papi.h>],
   	       [PAPI_event_info_t plop;],
	       [enable_papi=yes],
	       AC_MSG_WARN([cannot find PAPI lib]))
   CPPFLAGS="$save_CPPFLAGS"

   if test x$enable_papi = xyes; then
      enable_papi=no
      CHECK_HEADER_CPPFLAGS([papi.h], [enable_papi=yes], "$PAPI_CPPFLAGS", AC_MSG_WARN([cannot find headers for PAPI]))
   fi

   if test x$enable_papi = xyes; then
      CFLAGS="$CFLAGS -DUSE_PAPI"
   fi

fi


# MPI stuff
MPI_ROOT=
MPI_INC=
use_mpi=yes
use_mpi_from_system=yes
AC_ARG_WITH(mpi, [AS_HELP_STRING([--with-mpi[=<dir>]], [Use a specific MPI installation])],
        [
                if test x$withval != xno; then
                        if test x$withval = xyes; then
                                AC_MSG_RESULT(yes)
                                use_mpi_from_system=yes
                        else
                                # use specified path
                                use_mpi_from_system=no
                                mpidir=$withval
                                AC_MSG_RESULT(yes using $mpidir)
				MPI_ROOT=$mpidir
                                AC_SUBST(MPI_ROOT, $mpidir)
                        fi
                else
			# either --without-mpi or --with-mpi=no is set
                        use_mpi=no
                fi
        ])

AC_ARG_WITH(mpi-include, [AS_HELP_STRING([--with-mpi-include[=<dir>]], [Use a specific MPI header])],
        [
                if test x$withval != xno; then
                        if test x$withval = xyes; then
                                AC_MSG_RESULT(yes)
                                use_mpi_from_system=yes
                        else
                                # use specified path
                                use_mpi_from_system=no
                                mpidir=$withval
                                AC_MSG_RESULT(yes using $mpidir)
				MPI_INC=$mpidir
                                AC_SUBST(MPI_INC, $mpidir)
                        fi
                else
			# either --without-mpi or --with-mpi=no is set
                        use_mpi=no
                fi
        ])

if test x$MPI_INC != x; then
# use the value supplied by --with-mpi-include
        CPPFLAGS="${CPPFLAGS} -I$MPI_INC/"
elif test x$MPI_ROOT != x; then
# use the value supplied by --with-mpi
        CPPFLAGS="${CPPFLAGS} -I$MPI_ROOT/include/ "
fi

if test $use_mpi = yes ; then
   AC_SUBST(USE_MPIDIR_FROM_USER,$use_mpi_from_system)
   # MPI header
   AC_CHECK_HEADER([mpi.h],[enable_mpi=yes],AC_MSG_WARN([cannot find headers for MPI]))
else
  enable_mpi=no
fi


# GTG stuff
use_extern_gtg=no
AC_ARG_WITH(gtg, [AS_HELP_STRING([--with-gtg[=<dir>]], [path to libgtg])],
        [
                if test x$withval != xno; then
                        if test x$withval = xyes; then
                                AC_MSG_RESULT(yes)
                        else
                                # use specified path
				use_extern_gtg=yes
                                gtgdir=$withval
                                AC_MSG_RESULT(yes using gtgdir)
                                AC_SUBST(GTGDIR, $gtgdir)
                        fi
                else
                        AC_MSG_RESULT(no)
			AC_MSG_ERROR([Cannot find GTG])
                fi
        ])


if test x$use_extern_gtg = xyes; then
        GTG_CPPFLAGS="${CPPFLAGS} -I$gtgdir/include/"
        GTG_LDFLAGS="${LDFLAGS} -L$gtgdir/lib/ -lgtg"
	CHECK_LIB_LDFLAGS([gtg], [setTraceType],"$GTG_LDFLAGS", , AC_MSG_ERROR([cannot find libgtg]))
	CHECK_LIB_LDFLAGS([gtg], [gtg_record],"$GTG_LDFLAGS", GTG_OUT_OF_ORDER=1, GTG_OUT_OF_ORDER=0)
	CHECK_HEADER_CPPFLAGS([GTG.h],, "$GTG_CPPFLAGS", AC_MSG_ERROR([cannot find headers for gtg]))
else

    GTG_OUT_OF_ORDER=1
  dnl configure gtg from extlib
    gtg_parent_dir=`pwd`
    gtg_dir="extlib/gtg"
    AC_MSG_NOTICE([configuring in gtg_dir ($gtg_parent_dir/$gtg_dir)])
    AS_IF([test "$srcdir" != "."],
                [
                        test -d "$gtg_dir" ||
                        mkdir -p "$gtg_dir" ||
                        AC_MSG_ERROR([cannot create $gtg_dir])
                ])

                cd $gtg_dir

                case $srcdir in
                        .)
                                gtg_srcdir="$srcdir"
                                ;;
                        /*)
                                gtg_srcdir="$srcdir/$gtg_dir"
                                ;;
                        *)
                                gtg_srcdir="../../$srcdir/$gtg_dir"
                                ;;
                esac

                gtg_conf_cmd="$gtg_srcdir/configure"
   		gtg_conf_args="$gtg_conf_args --prefix=\"$prefix\" --exec-prefix=\"$exec_prefix\" --bindir=\"$bindir\" --libdir=\"$libdir\" --includedir=\"$includedir\" --docdir=\"$docdir/gtg\" $GTGFLAGS --cache-file=\"/dev/null\" --srcdir=\"$gtg_srcdir\""
                AC_MSG_NOTICE([running $SHELL $gtg_conf_cmd $gtg_conf_args])
                eval "$SHELL '$gtg_conf_cmd' $gtg_conf_args"
                AS_IF([test $? != "0"], [AC_MSG_ERROR([$gtg_conf_cmd failed for $gtg_dir])])

                cd $gtg_parent_dir
                AS_IF([test x"$GTGLIB" = x], [GTG_LDFLAGS="-lgtg"])

		GTG_CPPFLAGS="$GTG_CPPFLAGS -I${prefix}/include/gtg"
fi


AC_ARG_ENABLE([gettid],
	AC_HELP_STRING([--enable-gettid],
		[Use syscall(SYS_gettid) to get the thread ID instead of pthread_self(). This however costs a system call for each trace entry]))
if test "x$enable_gettid" = xyes; then
  AC_DEFINE([USE_GETTID], [1], [Use gettid])
fi

AC_SUBST(libdir)
AC_SUBST(exec_prefix)
AC_SUBST(prefix)
AC_SUBST(includedir)
AC_SUBST(bindir)
AC_SUBST(datadir)


# if MPI is disabled/not found, do not compile EZTrace MPI module
AC_SUBST(GTG_CPPFLAGS)
AC_SUBST(GTG_LDFLAGS)
AC_SUBST(GTG_OUT_OF_ORDER)
AM_CONDITIONAL([GTG_OUT_OF_ORDER], [test "$GTG_OUT_OF_ORDER" = "1"] )

AC_SUBST(FXT_CPPFLAGS)
AC_SUBST(FXT_LDFLAGS)




AM_CONDITIONAL([OPENMP_FOUND], [test "x$openmp_found" = xyes])


AC_SUBST(PAPI_CPPFLAGS)
AC_SUBST(PAPI_LDFLAGS)
AC_SUBST(PAPI_LIBDIR)

PAPI_ENABLED="$enable_api"
AC_SUBST(PAPI_ENABLED)
AM_CONDITIONAL([USE_PAPI], [test "$enable_papi" = yes])


AC_SUBST(DYNLIB_EXT)
AC_SUBST(LD_PRELOAD_NAME)
AC_SUBST(LD_LIBRARY_PATH_NAME)
AM_CONDITIONAL([USE_MPI], [test "$enable_mpi" = yes])
AM_CONDITIONAL([USE_PTHREAD], [test "$have_full_pthread" = yes])

AM_CONDITIONAL(AMEXTERNFXT, test x"$use_extern_fxt" = "xyes")
AM_CONDITIONAL(AMEXTERNGTG, test x"$use_extern_gtg" = "xyes")

AC_CONFIG_SUBDIRS([extlib/gtg extlib/fxt extlib/opari2])


AC_OUTPUT([Makefile eztrace.pc \
		    extlib/Makefile \
		    src/core/Makefile \
		    src/modules/omp/Makefile \
		    src/modules/mpi/Makefile \
		    src/modules/pthread/Makefile \
		    src/modules/stdio/Makefile \
		    src/modules/memory/Makefile \
		    src/modules/papi/Makefile \
		    src/core/eztrace_create_plugin \
		    src/core/input/Makefile.template \
		    src/modules/omp/eztrace_cc \
		    test/mpi/Makefile \
		    test/openmp/Makefile \
		    src/core/eztrace.c \
		    src/core/eztrace.old \
		    src/core/config.h \
		    test/static/Makefile \
		    test/stdio/Makefile \
                    test/memory/Makefile \
                    test/pthread/Makefile \
		    test/automake/Makefile \
                    test/automake/testcommon.h \
		    test/pptrace/automatic/Makefile])
		    
