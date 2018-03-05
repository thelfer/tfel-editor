AM_CPPFLAGS = -I$(top_srcdir)/packages/sh/include  \
	          -I$(top_srcdir)/include              \
	          @QT_CXXFLAGS@                        \
	          `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libShQEmacsMode.la

libShQEmacsMode_la_SOURCES = ShMajorMode.cxx         \
				           ShSyntaxHighlighter.cxx
nodist_libShQEmacsMode_la_SOURCES = moc_ShMajorMode.cxx         \
					              moc_ShSyntaxHighlighter.cxx
libShQEmacsMode_la_CPPFLAGS = $(AM_CPPFLAGS) -DShQEmacsMode_EXPORTS

clean-local:
	$(RM) $(nodist_libShQEmacsMode_la_SOURCES)

moc_%.cxx : @top_srcdir@/packages/sh/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

EXTRA_DIST=CMakeLists.txt
