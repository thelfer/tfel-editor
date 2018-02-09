AM_CPPFLAGS = -I$(top_srcdir)/packages/make/include \
	      -I$(top_srcdir)/include               \
	      @QT_CXXFLAGS@                         \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libMakefileQEmacsMode.la

libMakefileQEmacsMode_la_SOURCES = MakefileMajorMode.cxx         \
				   MakefileSyntaxHighlighter.cxx
nodist_libMakefileQEmacsMode_la_SOURCES = moc_MakefileMajorMode.cxx         \
					  moc_MakefileSyntaxHighlighter.cxx

clean-local:
	$(RM) $(nodist_libMakefileQEmacsMode_la_SOURCES)

moc_%.cxx : @top_srcdir@/packages/make/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

EXTRA_DIST=CMakeLists.txt
