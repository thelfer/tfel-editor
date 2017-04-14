AM_CPPFLAGS = -I$(top_srcdir)/include                   \
	      -I$(top_srcdir)/packages/gnuplot/include  \
	      -I$(top_srcdir)/include                   \
	      @QT4_CFLAGS@                              \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libTPlotQEmacsPackage.la
libTPlotQEmacsPackage_la_SOURCES = TPlotMajorMode.cxx
libTPlotQEmacsPackage_la_LIBADD  = -L$(top_builddir)/packages/gnuplot/src \
                                   -lGnuplotQEmacsPackage

EXTRA_DIST=CMakeLists.txt
