## Process this file with autoconf to produce configure script

# (c) 2021 Divvun <https://divvun.org>

# autoconf requirements
AC_PREREQ([2.62])
AC_INIT([hfst-predict], [0.0.1], [https://github.com/divvun/hfst-predict/issues], [hfst-predict], [https://github.com/divvun/hfst-predict])

# init
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign check-news color-tests silent-rules])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/hfst-predict.cc])
AC_CONFIG_HEADERS([src/config.h])

PKG_PROG_PKG_CONFIG

# Settings

# Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries
AC_CHECK_LIB([ncurses], [getch])
AC_CHECK_LIB([tinfo], [main])

# Checks for header files
AC_CHECK_HEADERS([getopt.h error.h ncurses.h])

# Checks for types
AC_TYPE_SIZE_T

# Checks for structures

# Checks for compiler characteristics
AC_C_BIGENDIAN

# Checks for library functions
AC_CHECK_FUNCS([strndup error getline getch])

# Checks for system services

# Require highest supported C++ standard
AC_LANG(C++)
AX_CHECK_COMPILE_FLAG([-std=c++20], [CXXFLAGS="$CXXFLAGS -std=c++20"], [
 AX_CHECK_COMPILE_FLAG([-std=c++2a], [CXXFLAGS="$CXXFLAGS -std=c++2a"], [
  AX_CHECK_COMPILE_FLAG([-std=c++17], [CXXFLAGS="$CXXFLAGS -std=c++17"], [
   AX_CHECK_COMPILE_FLAG([-std=c++1z], [CXXFLAGS="$CXXFLAGS -std=c++1z"], [
    AX_CHECK_COMPILE_FLAG([-std=c++14], [CXXFLAGS="$CXXFLAGS -std=c++14"], [
     AX_CHECK_COMPILE_FLAG([-std=c++1y], [CXXFLAGS="$CXXFLAGS -std=c++1y"], [
      AC_MSG_ERROR([Could not enable at least C++1y (C++14) - upgrade your compiler])
     ])
    ])
   ])
  ])
 ])
])

# use HFST mainline for now
PKG_CHECK_MODULES([HFST], [hfst >= 3.12])

# config files
AC_CONFIG_FILES([Makefile src/Makefile])

# output
AC_OUTPUT

cat <<EOF
-- Building $PACKAGE_STRING
EOF
