AM_CPPFLAGS = -I$(top_srcdir)/packages/mtest/include  \
	      -I$(top_srcdir)/include                 \
	      @QT4_CFLAGS@                            \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libMTestQEmacsMode.la

libMTestQEmacsMode_la_SOURCES = MTestStudyOptions.cxx         \
				MTestMajorMode.cxx            \
				MTestSyntaxHighlighter.cxx    \
				PipeTestMajorMode.cxx         \
				PipeTestSyntaxHighlighter.cxx
libMTestQEmacsMode_la_LIBADD   = -L@TFELHOME@/lib      \
			         -lTFELMTest

nodist_libMTestQEmacsMode_la_SOURCES = moc_MTestMajorMode.cxx            \
				       moc_MTestStudyOptions.cxx         \
				       moc_MTestSyntaxHighlighter.cxx    \
				       moc_PipeTestMajorMode.cxx         \
				       moc_PipeTestSyntaxHighlighter.cxx

moc_%.cxx : @top_srcdir@/packages/mtest/include/QEmacs/%.hxx
	@MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libMTestQEmacsMode_la_SOURCES)

EXTRA_DIST=CMakeLists.txt
