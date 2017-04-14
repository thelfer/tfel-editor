AM_CPPFLAGS = -I$(top_srcdir)/packages/licos/include \
	      -I$(top_srcdir)/include                \
	      @QT4_CFLAGS@                           \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libLicosQEmacsMode.la

libLicosQEmacsMode_la_SOURCES = LicosBlock.cxx                        \
			        StandardLicosBlock.cxx                \
			        LicosMainBlock.cxx                    \
			        LicosTokenizer.cxx                    \
			        LicosMajorMode.cxx                    \
			        LicosSyntaxHighlighter.cxx            \
			        LicosOutputFrame.cxx                  \
			        LicosStudyOptions.cxx                 \
			        LicosStudyThread.cxx                  \
			        LicosStudy.cxx                        \
			        LicosMaterialWizard.cxx               \
				MaterialPropertySelector.cxx          \
				LicosInsertMaterialPropertyDialog.cxx

nodist_libLicosQEmacsMode_la_SOURCES = moc_LicosMajorMode.cxx                    \
				       moc_LicosSyntaxHighlighter.cxx            \
				       moc_LicosStudyOptions.cxx                 \
				       moc_LicosStudyThread.cxx                  \
				       moc_LicosOutputFrame.cxx                  \
				       moc_LicosStudy.cxx                        \
				       moc_MaterialPropertySelector.cxx          \
				       moc_LicosInsertMaterialPropertyDialog.cxx \
				       LicosModeResources.cxx

LICOSMODERESOURCES=images/LicosIcon.png\
		   images/LicosPDFIcon.png

LicosModeResources.cxx : LicosModeResources.qrc $(LICOSMODERESOURCES)
	@RCC@ -name LicosModeResources @srcdir@/LicosModeResources.qrc -o LicosModeResources.cxx

moc_%.cxx : @top_srcdir@/packages/licos/include/QEmacs/%.hxx
	@MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libLicosQEmacsMode_la_SOURCES)

EXTRA_DIST = LicosModeResources.qrc \
	     $(LICOSMODERESOURCES)  \
	     CMakeLists.txt
