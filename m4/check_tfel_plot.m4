dnl
dnl
dnl

AC_DEFUN([CHECK_TFEL_PLOT],[

AC_CHECKING(for TFELPLOT location)
AC_ARG_WITH(tfel-plot,
            [  --with-tfel-plot=HOME      root directory path to TFELPLOT library installation ],
            [TFELPLOTHOME="$withval"
             AC_MSG_RESULT("select $withval as path to TFELPLOT library")
            ])

# checking for tfel-plot-config
if test "x${TFELPLOTHOME}" != "x" ; then
  TFELPLOTCONFIG="${TFELPLOTHOME}/bin/tfel-plot-config"
  if test ! [-x ${TFELPLOTCONFIG}];
  then	
    AC_ERROR("tfel-plot-config not found")	
  fi
else
  TFELPLOTCONFIG=""
  AC_CHECK_PROG(TFELPLOTCONFIG,[tfel-plot-config],[yes])
  if test "x$TFELPLOTCONFIG" != "xyes";
  then
      AC_ERROR("tfel-plot-config not found")	
  fi
  TFELPLOTCONFIG=`which tfel-plot-config`
fi

AC_CHECKING(for TFELPlot Library)

TFELPLOT_LIBS=$(${TFELPLOTCONFIG} --libs)

AC_SUBST(TFELPLOTHOME)
AC_SUBST(TFELCONFIG)
AC_SUBST(TFELPLOT_LIBS)

])dnl
