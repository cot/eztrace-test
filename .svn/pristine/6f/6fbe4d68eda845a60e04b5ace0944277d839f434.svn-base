#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT(Fast Traces Toolkit, 0.2.11, samuel.thibault@labri.fr,fxt)
AC_CONFIG_SRCDIR([tools/fxt.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign -Wall])
# AC_INIT_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_LN_S
AC_CHECK_PROGS([AWK], [awk])
AC_CHECK_PROGS([LATEX], [latex], [latex])
AC_CHECK_PROGS([EPSTOPDF], [epstopdf], [epstopdf])

LT_PREREQ([2])
LT_INIT([win32-dll])

AC_PATH_PROG(PERL, perl, no)

AC_PATH_PROGS([FXT_MS_LIB], [lib])
AC_ARG_VAR([FXT_MS_LIB], [Path to Microsoft's Visual Studio `lib' tool])
AM_CONDITIONAL([FXT_HAVE_MS_LIB], [test "x$FXT_MS_LIB" != "x"])

AC_CHECK_SIZEOF([void *])
SIZEOF_VOID_P=$ac_cv_sizeof_void_p
case $SIZEOF_VOID_P in
        4)
                FXT_MS_LIB_ARCH=X86
                ;;
        8)
                FXT_MS_LIB_ARCH=X64
                ;;
esac
AC_SUBST(FXT_MS_LIB_ARCH)

echo a | $AWK '{ print gensub(/a/, "AA", "g"); }' 2>/dev/null
if test x$? != x0 ; then
  AC_MSG_ERROR(Your awk program does not define gensub.  Use gawk or another awk with gensub)
fi

AC_PATH_XTRA

# Checks for libraries.

AC_CHECK_LIB([bfd], bfd_release, , echo "FxT needs the binutils-dev package for function names")

# add some libs that bfd is "known" to depend on when statically linking
libs_save="${LIBS}"
LIBS=
AC_HAVE_LIBRARY(iberty)
AC_HAVE_LIBRARY(z)
FXT_LIBS="${LIBS}"
LIBS="${libs_save}"
ldflags_save="${LDFLAGS}"
AC_SUBST(FXT_LIBS)

LDFLAGS="${LDFLAGS} ${X_LIBS}"
AC_CHECK_HEADERS([X11/Xlib.h X11/Xutil.h], [dnl
  AC_CHECK_LIB(X11,XDrawLine, X11=yes, echo "fxt_select needs the xlibs-dev package" ; X11=no)
])
LDFLAGS="${ldflags_save}"


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h sys/ioctl.h sys/mount.h sys/vfs.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([dup2 getpagesize localtime_r memmove memset strchr strdup strerror strspn strstr strtol strtoul uname])

case "${host_os}"
in
  linux*)
    FKT=yes
    ;;
  mingw*)
    AC_HAVE_LIBRARY(pthread)
    fxt_windows=yes
    ;;
  cygwin*)
    fxt_windows=yes
    ;;
esac
AM_CONDITIONAL([FXT_HAVE_WINDOWS], [test "x$fxt_windows" = "xyes"])

AC_C_BIGENDIAN(AC_DEFINE([IS_BIG_ENDIAN],[1],[Define this if architecture is big endian]), , [echo "Warning: unknown endian order, don't exchange traces with other endiannesses"])

AM_CONDITIONAL(FKT, [test "x$FKT" = xyes])
AM_CONDITIONAL(X11, [test "x$X11" = xyes])
AM_CONDITIONAL(LATEX, [test "x$ac_cv_prog_LATEX" != x])
AM_CONDITIONAL(EPSTOPDF, [test "x$ac_cv_prog_EPSTOPDF" != x])

AC_ARG_ENABLE([gettid],
	AC_HELP_STRING([--enable-gettid],
		[Use syscall(SYS_gettid) to get the thread ID instead of pthread_self(). This however costs a system call for each trace entry]))
if test "x$enable_gettid" = xyes; then
  AC_DEFINE([USE_GETTID], [1], [Use gettid])
fi

# On many multicore CPUs, clock cycles are not synchronized, we'd better use
# clock_gettime() instead of TSC.
AC_CHECK_LIB([rt], [clock_gettime], [LIBS="$LIBS -lrt"] )
AC_MSG_CHECKING([for clock_gettime(CLOCK_MONOTONIC)])
AC_COMPILE_IFELSE(
  AC_LANG_PROGRAM([[
    #include <time.h>
    struct timespec tp;
    ]], [[ clock_gettime(CLOCK_MONOTONIC, &tp);
    ]]),
  AC_DEFINE(HAVE_CLOCK_GETTIME, [1], [Use clock_gettime])
  AC_MSG_RESULT([yes]),
  AC_MSG_RESULT([no])
)

AC_CONFIG_FILES([
  tools/template2h.pl
], [chmod +x tools/template2h.pl])

# WARNING: Makefiles on different lines to be grepped by bootstrap
AC_OUTPUT([
  Makefile
  tools/Makefile
  doc/Makefile
  fxt.pc
])

