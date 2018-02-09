AM_CPPFLAGS = -I$(top_srcdir)/packages/markdown/include \
	      -I$(top_srcdir)/include                 \
	      @QT_CXXFLAGS@                            \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libMarkdownQEmacsMode.la

libMarkdownQEmacsMode_la_SOURCES = MarkdownMajorMode.cxx         \
				 MarkdownSyntaxHighlighter.cxx
libMarkdownQEmacsMode_la_LIBADD  = -L@TFELHOME@/lib      \
			         -lTFELUtilities

nodist_libMarkdownQEmacsMode_la_SOURCES = moc_MarkdownMajorMode.cxx         \
				        moc_MarkdownSyntaxHighlighter.cxx

moc_%.cxx : @top_srcdir@/packages/markdown/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libMarkdownQEmacsMode_la_SOURCES)

EXTRA_DIST=CMakeLists.txt
