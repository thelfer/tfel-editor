AM_CPPFLAGS = -I$(top_srcdir)/include                        \
	      @QT_CXXFLAGS@                                   \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libPythonQEmacsMode.la
libPythonQEmacsMode_la_SOURCES = PythonMajorMode.cxx
EXTRA_DIST=CMakeLists.txt
