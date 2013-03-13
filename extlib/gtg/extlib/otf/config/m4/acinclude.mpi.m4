AC_DEFUN([CHECK_MPI],
[
    mpi_error="no"
    check_mpi="yes"
    force_mpi="no"
    have_mpi="no"

    AC_ARG_WITH([mpi],
        AC_HELP_STRING([--with-mpi],
            [use MPI for some OTF tools, default: yes if found by configure]),
        [if test "$withval" = "yes"; then force_mpi="yes"; else check_mpi="no"; fi])

    AC_ARG_WITH([mpi-dir],
        AC_HELP_STRING([--with-mpi-dir],
            [give the path for MPI, default: /usr]),
        [mpi_dir="$withval/"])

    AC_ARG_WITH([mpi-inc-dir],
        AC_HELP_STRING([--with-mpi-inc-dir],
            [give the path dir MPI-include files, default: MPIDIR/include]),
        [mpi_inc_dir="$withval/"],
        [if test x"$mpi_dir" != x; then mpi_inc_dir="$mpi_dir"include/; fi])

    AC_ARG_WITH([mpi-lib-dir],
        AC_HELP_STRING([--with-mpi-lib-dir],
            [give the path for MPI-libraries, default: MPIDIR/lib]),
        [mpi_lib_dir="$withval/"],
        [if test x"$mpi_dir" != x; then mpi_lib_dir="$mpi_dir"lib/; fi])

    AC_ARG_WITH([mpi-lib],
        AC_HELP_STRING([--with-mpi-lib],
            [use given MPI]),
        [mpi_lib="$withval"])

    if test "$check_mpi" = "yes"; then
        sav_CPPFLAGS=$CPPFLAGS
        AS_IF([test x"$mpi_inc_dir" != x],
        [CPPFLAGS="$CPPFLAGS -I$mpi_inc_dir"])

        sav_LDFLAGS=$LDFLAGS
        AS_IF([test x"$mpi_lib_dir" != x],
        [LDFLAGS="$LDFLAGS -L$mpi_lib_dir"])

        AS_IF([test x"$mpi_lib" != x],
        [MPILIBS="$mpi_lib"])

        AX_MPI(
        [
            mpi_lib=$MPILIBS
            have_mpi="yes"
        ],
        [
            mpi_error="yes"
        ])

        CPPFLAGS=$sav_CPPFLAGS
        LDFLAGS=$sav_LDFLAGS
    fi

    MPI_LIB_DIR=$mpi_lib_dir
    MPI_LIB_LINE=$mpi_lib
    if test x"$mpi_lib_dir" != x; then
        MPI_LIB_LINE="-L$mpi_lib_dir $MPI_LIB_LINE"
    fi

    MPI_INCLUDE_DIR=$mpi_inc_dir
    MPI_INCLUDE_LINE=
    if test x"$mpi_inc_dir" != x; then
        MPI_INCLUDE_LINE="-I$mpi_inc_dir"
    fi

    AC_SUBST(MPI_LIB_DIR)
    AC_SUBST(MPI_LIB_LINE)
    AC_SUBST(MPI_INCLUDE_DIR)
    AC_SUBST(MPI_INCLUDE_LINE)
])
