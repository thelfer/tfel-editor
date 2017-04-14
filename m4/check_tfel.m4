dnl
dnl
dnl

AC_DEFUN([CHECK_TFEL],[

AC_CHECKING(for TFEL Library)

AC_CHECKING(for TFEL location)
AC_ARG_WITH(tfel,
            [  --with-tfel=HOME      root directory path to TFEL library installation ],
            [TFELHOME="$withval"
             AC_MSG_RESULT("select $withval as path to TFEL library")
            ])

# checking for tfel-config
if test "x${TFELHOME}" != "x" ; then
  TFELCONFIG="${TFELHOME}/bin/tfel-config"
  if test ! [-x ${TFELCONFIG}];
  then	
    AC_ERROR("tfel-config not found")	
  fi
else
  TFELCONFIG=""
  AC_CHECK_PROG(TFELCONFIG,[tfel-config],[yes])
  if test "x$TFELCONFIG" != "xyes";
  then
      AC_ERROR("tfel-config not found")	
  fi
  TFELCONFIG=`which tfel-config`
fi

TFEL_EXCEPTION_LIBS=$(${TFELCONFIG} --libs --exceptions)
TFEL_UTILITIES_LIBS=$(${TFELCONFIG} --libs --utilities)
TFEL_SYSTEM_LIBS=$(${TFELCONFIG} --libs --system)
TFEL_MATH_LIBS=$(${TFELCONFIG} --libs --math)
TFEL_TESTS_LIBS=$(${TFELCONFIG} --libs --tests)

AC_SUBST(TFELHOME)
AC_SUBST(TFELCONFIG)
AC_SUBST(TFEL_EXCEPTION_LIBS)
AC_SUBST(TFEL_UTILITIES_LIBS)
AC_SUBST(TFEL_SYSTEM_LIBS)
AC_SUBST(TFEL_MATH_LIBS)
AC_SUBST(TFEL_TESTS_LIBS)

# checking for mfront
if test "x${TFELHOME}" != "x" ; then
  MFRONT="${TFELHOME}/bin/mfront"
else
  MFRONT="mfront"
fi

if test ! [-x ${MFRONT}];
then
    AC_ERROR("mfront not found")	
fi

AC_SUBST(MFRONT)

# checking for tfel-doc
TFELDOC=""
if test "x${TFELHOME}" != "x" ; then
  TFELDOC="${TFELHOME}/bin/tfel-doc"
  if test ! [-x ${TFELDOC}];
  then	
    TFELDOC=""
  fi
else
  TFELDOC=""
  AC_CHECK_PROG(TFELDOC,[tfel-doc],[yes])
  if test "x$TFELDOC" != "xyes";
  then
      TFELDOC=""
  else
      TFELDOC=`which tfel-doc`      
  fi
fi

AC_SUBST(TFELDOC)
AM_CONDITIONAL(HAVE_TFELDOC,test "x$TFELDOC" != "x" )

])dnl
