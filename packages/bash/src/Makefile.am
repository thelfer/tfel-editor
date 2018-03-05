AM_CPPFLAGS = -I$(top_srcdir)/packages/bash/include \
	          -I$(top_srcdir)/packages/sh/include \
	          -I$(top_srcdir)/include               \
	          @QT_CXXFLAGS@                         \
	          `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libBashQEmacsMode.la

libBashQEmacsMode_la_SOURCES = BashMajorMode.cxx         \
				             BashSyntaxHighlighter.cxx
nodist_libBashQEmacsMode_la_SOURCES = moc_BashMajorMode.cxx         \
					                moc_BashSyntaxHighlighter.cxx

libBashQEmacsMode_la_LIBADD = -L@top_builddir@/packages/sh/src/ \
                              -L@top_builddir@/src/             \
                              -lShQEmacsMode                    \
                              -lTFELQEmacs

clean-local:
	$(RM) $(nodist_libBashQEmacsMode_la_SOURCES)

moc_%.cxx : @top_srcdir@/packages/bash/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

EXTRA_DIST=CMakeLists.txt
