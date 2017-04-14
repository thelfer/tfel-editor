AM_CPPFLAGS = -I$(top_srcdir)/packages/gnuplot/include \
	      -I$(top_srcdir)/include                         \
	      @QT4_CFLAGS@                                    \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libGnuplotQEmacsPackage.la
libGnuplotQEmacsPackage_la_CPPFLAGS = $(AM_CPPFLAGS) -DGnuplotQEmacsPackage_EXPORTS
libGnuplotQEmacsPackage_la_SOURCES  = GnuplotSyntaxHighlighter.cxx \
				      GnuplotMajorMode.cxx

EXTRA_DIST = CMakeLists.txt
