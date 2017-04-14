AM_CPPFLAGS = -I$(top_srcdir)/packages/mfront/include \
	      -I$(top_srcdir)/include                 \
	      @QT4_CFLAGS@                            \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libMFrontQEmacsMode.la

libMFrontQEmacsMode_la_SOURCES = MFrontMajorMode.cxx         \
				 MFrontSyntaxHighlighter.cxx
libMFrontQEmacsMode_la_LIBADD  = -L@TFELHOME@/lib      \
			         -lTFELMFront

nodist_libMFrontQEmacsMode_la_SOURCES = moc_MFrontMajorMode.cxx         \
				        moc_MFrontSyntaxHighlighter.cxx
				        MFrontModeResources.cxx

MFRONTMODERESOURCES=images/MFrontIcon.png

MFrontModeResources.cxx : MFrontModeResources.qrc $(MFRONTMODERESOURCES)
	@RCC@ -name MFrontModeResources @srcdir@/MFrontModeResources.qrc -o MFrontModeResources.cxx

moc_%.cxx : @top_srcdir@/packages/mfront/include/QEmacs/%.hxx
	@MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libMFrontQEmacsMode_la_SOURCES)

EXTRA_DIST=CMakeLists.txt         \
	   MFrontModeResources.qrc \
	   $(MFRONTMODERESOURCES)
