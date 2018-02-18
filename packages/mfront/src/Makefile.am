AM_CPPFLAGS = -I$(top_srcdir)/packages/mfront/include \
	      -I$(top_srcdir)/include                 \
	      @QT_CXXFLAGS@                            \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libMFrontQEmacsMode.la

libMFrontQEmacsMode_la_SOURCES = MFrontInitializer.cxx \
                 MFrontMajorMode.cxx                   \
				 MFrontSyntaxHighlighter.cxx           \
				 MFrontOptions.cxx
libMFrontQEmacsMode_la_LIBADD  = -L@TFELHOME@/lib      \
			         -lTFELUtilities       \
				 -lTFELMFront

nodist_libMFrontQEmacsMode_la_SOURCES = moc_MFrontMajorMode.cxx   \
				        moc_MFrontSyntaxHighlighter.cxx           \
				        moc_MFrontOptions.cxx                     \
				        MFrontModeResources.cxx

MFRONTMODERESOURCES=images/MFrontIcon.png

MFrontModeResources.cxx : MFrontModeResources.qrc $(MFRONTMODERESOURCES)
	@QT_RCC@ -name MFrontModeResources @srcdir@/MFrontModeResources.qrc -o MFrontModeResources.cxx

moc_%.cxx : @top_srcdir@/packages/mfront/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libMFrontQEmacsMode_la_SOURCES)

EXTRA_DIST=CMakeLists.txt         \
	   MFrontModeResources.qrc \
	   $(MFRONTMODERESOURCES)
