dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(battleship, 0.2)

MAX_SIZE=15
AC_SUBST(MAX_SIZE)

AC_SUBST(MAX_SIZE)

ENABLE_NLS=true
AC_SUBST(ENABLE_NLS)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC




dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
GETTEXT_PACKAGE=battleship
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])



AM_PROG_LIBTOOL



PKG_CHECK_MODULES(GTK_, [gtk+-3.0])
AC_SUBST(BATTLESHIP_CFLAGS)
AC_SUBST(BATTLESHIP_LIBS)

PKG_CHECK_MODULES(BATTLESHIP,gtk+-3.0)




AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])
