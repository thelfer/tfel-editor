# Process this file with autoconf to produce a configure script.
# Process this file with automake to produce Makefile.in files.

AC_INIT(qemacs,1.1,thomas.helfer@cea.fr)
AC_CONFIG_SRCDIR([include/QEmacs/Config.hxx])
AC_CONFIG_COMMANDS
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CANONICAL_TARGET

# Automake specific stuff
AM_INIT_AUTOMAKE([foreign dist-bzip2])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

#disable static library generation by default
AC_ENABLE_STATIC(no)

# disable automatic declaration of
# CXXFLAGS by AC_PROG_CXX
if test "x$CXXFLAGS" = "x";
    then
    CXXFLAGS=""
fi

AC_PROG_CXX

# Libtool specific stuff
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(LIBTOOL_DEPS)

# Compiler specific stuff
AC_ARG_ENABLE([optimizations],
	       AC_HELP_STRING([--disable-optimizations],
		              [Do not try to guess what optimization to enable]))

# Enable debug
AC_ARG_ENABLE([debug],
	AC_HELP_STRING([--enable-debug], [Compile with debug options]))

#disable default flags
CFLAGS="";

#choose between intel icpc and g++ and ekopath
INTEL="no"
PATHSCALE="no"
if test "x$GXX" = "xyes" -o "${CXX##*/}x" = "icpcx" ; then
    if test "${CXX##*/}x" = "icpcx"; then
	GXX="no"
	INTEL="yes"
    fi
fi
if test "x$GXX" = "xyes" -o "${CXX##*/}x" = "pathCCx" ; then
    if test "${CXX##*/}x" = "pathCCx"; then
	GXX="no"
	PATHSCALE="yes"
    fi
fi

# check for compiler
COMPILER_SPECIFIC_OPTIONS="";
COMPILER_WARNINGS="";
OPTIMISATION_FLAGS="-DNO_RUNTIME_CHECK_BOUNDS -DNDEBUG"

# g++ specific treatement
if test "x$GXX" = "xyes"; then
    AC_CHECK_GXX
fi

# intel icc specific treatement
if test "x$INTEL" = "xyes"; then
    AC_CHECK_INTEL
fi

# check for compiler
COMPILER_FLAGS="";
COMPILER_CXXFLAGS="";
COMPILER_WARNINGS="";
OPTIMISATION_FLAGS0=""
OPTIMISATION_FLAGS=""
OPTIMISATION_FLAGS2=""

# g++ specific treatement
if test "x$GXX" == "xyes"; then
    COMPILER_CXXFLAGS="-std=c++11";
    COMPILER_WARNINGS="";
    OPTIMISATION_FLAGS0="-DNO_RUNTIME_CHECK_BOUNDS -DNDEBUG"
    OPTIMISATION_FLAGS=""
    OPTIMISATION_FLAGS2=""
    AC_CHECK_GXX
fi

# intel icc specific treatement
if test "x$INTEL" == "xyes";
then
    COMPILER_FLAGS="";
    COMPILER_CXXFLAGS="-std=c++11";
    COMPILER_WARNINGS="";
    OPTIMISATION_FLAGS0="-DNO_RUNTIME_CHECK_BOUNDS -DNDEBUG"
    OPTIMISATION_FLAGS=""
    OPTIMISATION_FLAGS2=""
    AC_CHECK_INTEL
fi

CFLAGS="$CFLAGS $COMPILER_FLAGS"
CXXFLAGS="$CXXFLAGS $COMPILER_FLAGS"
CXXFLAGS="$CXXFLAGS $COMPILER_CXXFLAGS"
CXXFLAGS="$CXXFLAGS $COMPILER_WARNINGS"
CXXFLAGS="$CXXFLAGS $OPTIMISATION_FLAGS0"

AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_MSG_CHECKING([C++11 noreturn attribute])
noreturn=no
AC_TRY_COMPILE([],
	       [[[noreturn]] void f();],
	       [noreturn=yes],
	       [noreturn=no])
AC_MSG_RESULT($noreturn)
if test "x$noreturn" == "xyes";
then
   COMPILER_CXXFLAGS="$COMPILER_CXXFLAGS -DTFEL_HAVE_NORETURN_ATTRIBUTE";
   CXXFLAGS="$CXXFLAGS -DTFEL_HAVE_NORETURN_ATTRIBUTE";
fi
AC_LANG_RESTORE

echo
echo ---------------------------------------------
echo testing tfel
echo ---------------------------------------------
echo

CHECK_TFEL

AC_ARG_ENABLE([tfel-plot],
	       AC_HELP_STRING([--enable-tfel-plot],
		              [enable support of the tplot module]))


if test "x$enable_tfel_plot" == "xyes";
then
    echo
    echo ---------------------------------------------
    echo testing tfel-plot
    echo ---------------------------------------------
    echo
    CHECK_TFEL_PLOT
    AM_CONDITIONAL([QEMACS_TPLOT],[true])
else
    AM_CONDITIONAL([QEMACS_TPLOT],[false])
fi

## QT

echo
echo ---------------------------------------------
echo testing Qt5
echo ---------------------------------------------
echo
m4_include([m4/m4-ax_have_qt.m4])
AX_HAVE_QT
if test "x$have_qt" == "xno"; then
  AC_MSG_ERROR([Qt5 not detected])
fi
echo "qt5 C++ flags: $QT_CXXFLAGS"
echo "qt5 libraries: $QT_LIBS"
echo "qt5 moc      : $QT_MOC"

AC_CONFIG_SUBDIRS([src/hunspell-1.3.2])
if test "x$enable_private_hunspell" == "xyes"; then
    AC_DEFINE(QEMACS_HUNSPELL_SUPPORT)
    AM_CONDITIONAL([QEMACS_PRIVATE_HUNSPELL],true)
    AM_CONDITIONAL([QEMACS_EXTERNAL_HUNSPELL],false)
else
    PKG_CHECK_MODULES(hunspell,[hunspell],
	[HAVE_HUNSPELL=yes],
	[HAVE_HUNSPELL=no])
    if test "x$HAVE_HUNSPELL" == "xyes"; then
        AM_CONDITIONAL([QEMACS_PRIVATE_HUNSPELL],false)
	AM_CONDITIONAL([QEMACS_EXTERNAL_HUNSPELL],true)
	AC_DEFINE(QEMACS_HUNSPELL_SUPPORT)
    else 
        AM_CONDITIONAL([QEMACS_PRIVATE_HUNSPELL],true)
	AM_CONDITIONAL([QEMACS_EXTERNAL_HUNSPELL],false)
    fi
fi

AC_CONFIG_FILES([Makefile
		 cmake/Makefile
		 cmake/modules/Makefile
		 include/Makefile
		 src/Makefile
		 src/QEmacsBuildInformation.cxx
		 packages/Makefile
		 packages/castem/Makefile
		 packages/castem/src/Makefile
		 packages/castem/include/Makefile
		 packages/licos/Makefile
		 packages/licos/src/Makefile
		 packages/licos/include/Makefile
		 packages/latex/Makefile
		 packages/latex/src/Makefile
		 packages/latex/include/Makefile
		 packages/mfront/Makefile
		 packages/mfront/src/Makefile
		 packages/mfront/include/Makefile
		 packages/make/Makefile
		 packages/make/src/Makefile
		 packages/make/include/Makefile
		 packages/mtest/Makefile
		 packages/mtest/src/Makefile
		 packages/mtest/include/Makefile
		 packages/python/Makefile
		 packages/python/src/Makefile
		 packages/gnuplot/Makefile
		 packages/gnuplot/include/Makefile
		 packages/gnuplot/src/Makefile
		 packages/tplot/Makefile
		 packages/tplot/src/Makefile])

AC_OUTPUT

