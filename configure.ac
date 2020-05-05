# Initialization
AC_PREREQ([2.69])
AC_INIT([mac-address], [1.0])
AC_LANG([C++])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall foreign dist-xz])
AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Check compiler characteristics
AX_CXX_COMPILE_STDCXX_17([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Wno-unused-variable], [CXXFLAGS])

LT_INIT # Required for systemd linking

PKG_PROG_PKG_CONFIG
AC_ARG_WITH([systemdsystemunitdir],
    [AS_HELP_STRING([--with-systemdsystemunitdir=DIR], 
    [Directory for systemd service files])], 
    [], [with_systemdsystemunitdir=auto]
)
AS_IF([test "x$with_systemdsystemunitdir" = "xyes" -o "x$with_systemdsystemunitdir" = "xauto"],
    [def_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)
    AS_IF([test "x$def_systemdsystemunitdir" = "x"], 
        [AS_IF([test "x$with_systemdsystemunitdir" = "xyes"], 
            [AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])]
        )
        with_systemdsystemunitdir=no], 
        [with_systemdsystemunitdir="$def_systemdsystemunitdir"]
    )]
)
AS_IF([test "x$with_systemdsystemunitdir" != "xno"], 
    [AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])]
)
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

AC_ARG_VAR(SYSTEMD_TARGET, "Target for starting this service")
AS_IF([test "x$SYSTEMD_TARGET" == "x"], [SYSTEMD_TARGET="multi-user.target"])

# Create configured output
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([mac-address.service])
AC_OUTPUT
