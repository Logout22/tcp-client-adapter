#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tcp client adapter], [1.2], [abmelden22@gmx.de])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([LIBEVENT], [libevent >= 2.0])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNCS([atexit memset socket strdup strtol])

# Configure options
AH_TEMPLATE([HAVE_IPV6],
    [Define to enable IPv6. This flag should be set by default.])
AC_ARG_ENABLE([ipv6],
    [AS_HELP_STRING([--disable-ipv6],
        [disable IPv6 support])],
    [if test x$enableval != xno; then
        AC_DEFINE([HAVE_IPV6], [1])
    fi],
    [AC_DEFINE([HAVE_IPV6], [1])])

AC_ARG_ENABLE([tests],
    [AS_HELP_STRING([--enable-tests],
        [compile and run unit and integration tests (requires GLib)])],
    [PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.40])
    AC_TYPE_PID_T
    AC_TYPE_SIZE_T
    AC_TYPE_SSIZE_T
    AC_FUNC_FORK
    tests=true],
    [tests=false])
AM_CONDITIONAL([TESTS], [test x$tests = xtrue])

AC_ARG_ENABLE([coverage],
    [AS_HELP_STRING([--enable-coverage],
        [compile unit and integration tests with gcov information, run gcov])],
    [coverage=true],
    [coverage=false])
AM_CONDITIONAL([COVERAGE], [test x$coverage = xtrue])

AC_ARG_ENABLE([valgrind],
    [AS_HELP_STRING([--enable-valgrind],
        [enable running unit and integration tests with valgrind])],
    [valgrind=true],
    [valgrind=false])
AM_CONDITIONAL([VALGRIND], [test x$valgrind = xtrue])

# Automake setup
AC_CONFIG_FILES([Makefile])

AC_OUTPUT

