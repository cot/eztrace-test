#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB(bfd, bfd_get_error)
AC_CHECK_LIB(dl, dlsym)
AC_CHECK_LIB(pthread, pthread_create)

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([setenv])

# arguments for dyninst
AC_ARG_WITH(dyninst, AS_HELP_STRING([--with-dyninst], [Activate DynInst and set its path to ARG (default is /usr/local/lib)]), [have_dyninst=yes])
AC_ARG_WITH(dyninst_inc, AS_HELP_STRING([--with-dyninst-inc], [Path to dyninst include (default is /usr/local/include)]))

# arguments for pin
AC_ARG_WITH(pin, AS_HELP_STRING([--with-pin], [Activate PIN and set its path to ARG (default is /usr/shared/pin)]), [have_pin=yes])

# Config files
AC_CONFIG_FILES([Makefile instrumentation/Makefile tests/Makefile])
AS_IF([test "x$have_dyninst" = "xyes"],
	[AC_CONFIG_FILES([dyninst.conf])
	AC_SUBST(dyninst, "ddyninst dyninst")
	AC_SUBST(dyninst_mk, "include ../dyninst.conf")
	AS_IF(test "x$with_dyninst" = "xyes", [AC_SUBST(dyninst_lib, /usr/local/lib)], [AC_SUBST(dyninst_lib, "$with_dyninst")])
	AS_IF(test "x$with_dyninst_inc" = "x", [AC_SUBST(dyninst_inc, /usr/local/include)], [AC_SUBST(dyninst_inc, "$with_dyninst_inc")])
	])

AS_IF([test "x$have_dyninst" = "xyes"],
	[AC_CONFIG_FILES([pin.conf])
	AC_SUBST(pin, "pin_fast.so")
	AC_SUBST(pin_mk, "include ../pin.conf")
	AS_IF(test "x$with_pin" = "xyes", [AC_SUBST(pin_path, /usr/shared/pin)], [AC_SUBST(pin_path, "$with_pin")])
	AS_IF(test "x$with_pin" = "xyes", [AC_SUBST(pin_mk2, "include /usr/shared/pin/source/tools/makefile.gnu.config")], [AC_SUBST(pin_mk2, "include $with_pin/source/tools/makefile.gnu.config")])
	])

AC_OUTPUT
