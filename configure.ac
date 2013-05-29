dnl Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining a
dnl copy of this software and associated documentation files (the "Software"),
dnl to deal in the Software without restriction, including without limitation
dnl the rights to use, copy, modify, merge, publish, distribute, sublicense,
dnl and/or sell copies of the Software, and to permit persons to whom the
dnl Software is furnished to do so, subject to the following conditions:
dnl
dnl The above copyright notice and this permission notice (including the next
dnl paragraph) shall be included in all copies or substantial portions of the
dnl Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
dnl IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
dnl FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
dnl THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
dnl LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
dnl FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
dnl DEALINGS IN THE SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([xhiv], [0.0.2],
	[https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], [xhiv])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Dependencies for automake's static library support
AC_PROG_RANLIB

# Require xorg-macros 1.16 or later for XORG_MEMORY_CHECK_FLAGS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.16 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.16)
XORG_DEFAULT_OPTIONS

# Transport selection macro from xtrans.m4
XTRANS_CONNECTION_FLAGS

# Dependencies for the xhiv framework itself
PKG_CHECK_MODULES(XHIV, [xproto >= 7.0.22 bigreqsproto xtrans])

# Use pkg-config to get required cflags & libraries for each library we test
PKG_CHECK_MODULES(LIBX11, [x11])
PKG_CHECK_MODULES(LIBXEXT, [xext x11])
PKG_CHECK_MODULES(LIBXI, [xi x11])
PKG_CHECK_MODULES(LIBXINERAMA, [xinerama x11])
PKG_CHECK_MODULES(LIBXRANDR, [xrandr x11])
PKG_CHECK_MODULES(LIBXRENDER, [xrender x11])

PKG_CHECK_EXISTS([xcb], [HAVE_XCB="yes"], [HAVE_XCB="no"])
if test "x$HAVE_XCB" = "xyes" ; then
   PKG_CHECK_MODULES(LIBXCB, [xcb])
   AC_DEFINE([HAVE_XCB], [1], [Define if you have the XCB library])
fi
AM_CONDITIONAL([HAVE_XCB], [test "x$HAVE_XCB" = "xyes"])

# Use memory debugging flags when running tests
XORG_MEMORY_CHECK_FLAGS
if test "x$XORG_MALLOC_DEBUG_ENV" != "x" ; then
   AM_TESTS_ENVIRONMENT="export ${XORG_MALLOC_DEBUG_ENV} ;"
fi
AC_SUBST([AM_TESTS_ENVIRONMENT])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	tests/Makefile
	tests/libX11/Makefile
	tests/libXext/Makefile
	tests/libXi/Makefile
	tests/libXinerama/Makefile
	tests/libXrandr/Makefile
	tests/libXrender/Makefile
	tests/libxcb/Makefile
        ])
AC_OUTPUT
