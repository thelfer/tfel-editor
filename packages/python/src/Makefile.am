AM_CPPFLAGS = -I$(top_srcdir)/include                        \
	      @QT4_CFLAGS@                                   \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libPythonQEmacsMode.la
libPythonQEmacsMode_la_SOURCES = PythonMajorMode.cxx
EXTRA_DIST=CMakeLists.txt
