AM_CPPFLAGS = -I$(top_srcdir)/packages/cmake/include \
	          -I$(top_srcdir)/include               \
	          @QT_CXXFLAGS@                         \
	          `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libCMakeQEmacsMode.la

libCMakeQEmacsMode_la_SOURCES = CMakeMajorMode.cxx         \
				              CMakeSyntaxHighlighter.cxx
nodist_libCMakeQEmacsMode_la_SOURCES = moc_CMakeMajorMode.cxx         \
					                 moc_CMakeSyntaxHighlighter.cxx

clean-local:
	$(RM) $(nodist_libCMakeQEmacsMode_la_SOURCES)

moc_%.cxx : @top_srcdir@/packages/cmake/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

EXTRA_DIST=CMakeLists.txt
