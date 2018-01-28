AM_CPPFLAGS = -I$(top_srcdir)/packages/castem/include \
	      -I$(top_srcdir)/include                 \
	      @QT_CXXFLAGS@                            \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libCastemQEmacsMode.la

libCastemQEmacsMode_la_SOURCES        = CastemMajorMode.cxx         \
					CastemSyntaxHighlighter.cxx

nodist_libCastemQEmacsMode_la_SOURCES = moc_CastemMajorMode.cxx

clean-local:
	$(RM) $(nodist_libCastemQEmacsMode_la_SOURCES)

moc_%.cxx : @top_srcdir@/packages/castem/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

EXTRA_DIST=CMakeLists.txt
