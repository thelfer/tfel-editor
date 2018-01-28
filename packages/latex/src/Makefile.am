AM_CPPFLAGS = -I$(top_srcdir)/packages/latex/include \
	      -I$(top_srcdir)/include                \
	      @QT_CXXFLAGS@                           \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libLaTeXQEmacsMode.la

libLaTeXQEmacsMode_la_SOURCES = LaTeXMajorMode.cxx          \
				LaTeXOutputMajorMode.cxx    \
				LaTeXSyntaxHighlighter.cxx

nodist_libLaTeXQEmacsMode_la_SOURCES = moc_LaTeXMajorMode.cxx         \
				       moc_LaTeXSyntaxHighlighter.cxx \
				       LaTeXModeResources.cxx

LATEXMODERESOURCES=images/LaTeXIcon.svg

LaTeXModeResources.cxx : LaTeXModeResources.qrc $(LATEXMODERESOURCES)
	@QT_RCC@ -name LaTeXModeResources @srcdir@/LaTeXModeResources.qrc -o LaTeXModeResources.cxx

moc_%.cxx : @top_srcdir@/packages/latex/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libLaTeXQEmacsMode_la_SOURCES)

EXTRA_DIST = LaTeXModeResources.qrc \
	     $(LATEXMODERESOURCES)  \
	     CMakeLists.txt

