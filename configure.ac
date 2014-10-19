#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tcpbridge], [1.0], [abmelden22@gmx.de])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([tcpbridge_main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([LIBEVENT], [libevent])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNCS([atexit socket strdup strtol])

# Configure options
AC_ARG_ENABLE([tests],
    [AS_HELP_STRING([--enable-tests],
        [compile and run unit and integration tests (requires GLib)])],
    [PKG_CHECK_MODULES([GLIB], [glib-2.0]); tests=true],
    [tests=false])
AM_CONDITIONAL([TESTS], [test x$tests = xtrue])

AC_ARG_ENABLE([coverage],
    [AS_HELP_STRING([--enable-coverage],
        [compile unit and integration tests with gcov information, run gcov])],
    [ #check if gcov is available
      #check if bash is available
    coverage=true],
    [coverage=false])
AM_CONDITIONAL([COVERAGE], [test x$coverage = xtrue])

# Automake setup
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
