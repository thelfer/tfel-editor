if QEMACS_PRIVATE_HUNSPELL
SUBDIRS = hunspell-1.3.2
endif

AM_CPPFLAGS = -I$(top_srcdir)/include          \
	      @QT_CXXFLAGS@                        \
	      `@TFELCONFIG@ --includes --cppflags`
if QEMACS_EXTERNAL_HUNSPELL
AM_CPPFLAGS += -I$(hunspell_CFLAGS)
endif
if QEMACS_PRIVATE_HUNSPELL
AM_CPPFLAGS += -I$(top_srcdir)/src/hunspell-1.3.2/src
AM_CPPFLAGS += -I$(top_builddir)/src/hunspell-1.3.2/src/hunspell
endif

localedir = $(datadir)/locale
AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"

lib_LTLIBRARIES = libTFELQEmacs.la
libTFELQEmacs_la_SOURCES = QEmacsResources.cxx       \
			   SecondaryTask.cxx                     \
			   SecondaryTaskManager.cxx              \
			   QEmacsFileDownloader.cxx              \
			   QEmacsBuildInformation.cxx            \
			   QEmacsShortCutStyle.cxx               \
			   Utilities.cxx                         \
			   SpellChecker.cxx                      \
			   QEmacsHunspellDictionariesManager.cxx \
			   QEmacsWidget.cxx                      \
			   QEmacsCommand.cxx                     \
			   QEmacsRectangleMode.cxx               \
			   QEmacsGrepCommand.cxx                 \
			   QEmacsMajorModeChangeCommand.cxx      \
			   QEmacsCommandFactory.cxx              \
			   QEmacsBuffer.cxx                      \
			   QEmacsLineEdit.cxx                    \
			   QEmacsProcessLineEdit.cxx             \
			   QEmacsShellProcessLineEdit.cxx        \
			   QEmacsTextEditBase.cxx                \
			   QEmacsPlainTextEdit.cxx               \
			   QEmacsTextEdit.cxx                    \
			   QEmacsTextBrowser.cxx                 \
			   QEmacsTextEditSearch.cxx              \
			   QEmacsTextEditKeyPressEventFilter.cxx \
			   QEmacsTextEditQueryReplace.cxx        \
			   QEmacsTextEditReplaceFilter.cxx       \
			   QEmacsMajorModeFactory.cxx            \
			   QEmacsMajorMode.cxx                   \
			   QEmacsMajorModeBase.cxx               \
			   CompiledLanguageMajorModeBase.cxx     \
			   GrepOutputMajorMode.cxx               \
			   CSyntaxHighlighterBase.cxx            \
			   CSyntaxHighlighter.cxx                \
			   CMajorModeBase.cxx                    \
			   CxxMajorMode.cxx                      \
			   CxxSyntaxHighlighter.cxx              \
			   ProcessOutputFrame.cxx                \
			   ProcessOutputMajorModeBase.cxx        \
			   CompilationOutputMajorMode.cxx        \
			   QAbstractScrollAreaWrapper.cxx        \
			   QWebEngineViewWrapper.cxx             \
			   ProcessInteractionFrame.cxx           \
			   DirectoryMajorMode.cxx

nodist_libTFELQEmacs_la_SOURCES = moc_QEmacsWidget.cxx      \
			   	  moc_QEmacsShortCutStyle.cxx               \
			   	  moc_QEmacsCommand.cxx                     \
			   	  moc_QEmacsLineEdit.cxx                    \
			      moc_QEmacsProcessLineEdit.cxx             \
			      moc_QEmacsShellProcessLineEdit.cxx        \
				  moc_QEmacsBuffer.cxx                      \
			      moc_QEmacsTextEditSearch.cxx              \
				  moc_QEmacsTextEditKeyPressEventFilter.cxx \
				  moc_QEmacsTextEditBase.cxx                \
				  moc_QEmacsPlainTextEdit.cxx               \
				  moc_QEmacsTextEdit.cxx                    \
				  moc_QEmacsTextBrowser.cxx                 \
				  moc_CSyntaxHighlighter.cxx                \
				  moc_CSyntaxHighlighterBase.cxx            \
			   	  moc_QEmacsMajorMode.cxx                   \
			   	  moc_QEmacsMajorModeBase.cxx               \
				  moc_CompiledLanguageMajorModeBase.cxx     \
				  moc_CMajorModeBase.cxx                    \
				  moc_CxxMajorMode.cxx                      \
				  moc_CxxSyntaxHighlighter.cxx              \
				  moc_ProcessOutputFrame.cxx                \
				  moc_ProcessOutputMajorModeBase.cxx        \
			   	  moc_QAbstractScrollAreaWrapper.cxx        \
			   	  moc_QWebEngineViewWrapper.cxx             \
			   	  moc_QEmacsFileDownloader.cxx              \
				  moc_ProcessInteractionFrame.cxx           \
				  moc_DirectoryMajorMode.cxx

libTFELQEmacs_la_CPPFLAGS = $(AM_CPPFLAGS) -DTFELQEmacs_EXPORTS
libTFELQEmacs_la_LIBADD   = @QT_LIBS@            \
			    -L@TFELHOME@/lib     \
			    -lTFELSystem         \
			    -lTFELUtilities
if QEMACS_EXTERNAL_HUNSPELL
libTFELQEmacs_la_LIBADD += $(hunspell_LIBS)
endif
if QEMACS_PRIVATE_HUNSPELL
libTFELQEmacs_la_LIBADD += -L$(top_builddir)/src/hunspell-1.3.2/src/hunspell \
			   -lhunspell-1.3
endif

bin_PROGRAMS  = qemacs
qemacs_SOURCES = qemacs.cxx            \
		 QEmacsMainWindow.cxx
nodist_qemacs_SOURCES = moc_QEmacsMainWindow.cxx
qemacs_DEPENDENCIES = libTFELQEmacs.la
qemacs_LDFLAGS = -export-dynamic
qemacs_LDADD   = -lTFELQEmacs        \
		 -L@TFELHOME@/lib    \
		 -lTFELSystem        \
		 -lTFELException     \
		 @QT_LIBS@

QEMACSRESOURCES=images/failure.png                \
		        images/success.png        \
		        images/restart.png        \
		        images/languages/c.png    \
		        images/languages/cxx.png

moc_%.cxx : @top_srcdir@/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

QEmacsResources.cxx : QEmacsResources.qrc
	@QT_RCC@ -name QEmacsResources @srcdir@/QEmacsResources.qrc -o QEmacsResources.cxx

EXTRA_DIST = QEmacsResources.qrc \
	     $(QEMACSRESOURCES)  \
	     CMakeLists.txt

clean-local:
	$(RM) $(nodist_qemacs_SOURCES)
	$(RM) $(nodist_libTFELQEmacs_la_SOURCES)
