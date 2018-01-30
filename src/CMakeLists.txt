include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")
link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_add_resources(QEmacsResources_RCC
  QEmacsResources.qrc)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/QEmacsBuildInformation.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/QEmacsBuildInformation.cxx" @ONLY)

qt_sources(TFELQEmacs_SOURCES
  "${PROJECT_SOURCE_DIR}/include/QEmacs"
  QEmacsWidget
  QEmacsFileDownloader
  QEmacsShortCutStyle
  QEmacsCommand
  QEmacsLineEdit
  QEmacsBuffer
  QEmacsTextEditSearch
  QEmacsTextEditKeyPressEventFilter
  QEmacsTextEditBase
  QEmacsPlainTextEdit
  QEmacsTextEdit
  QEmacsTextBrowser
  CSyntaxHighlighter
  CSyntaxHighlighterBase
  QEmacsMajorMode
  QEmacsMajorModeBase
  CompiledLanguageMajorModeBase
  CMajorModeBase
  CxxMajorMode
  CxxSyntaxHighlighter
  ProcessOutputFrame
  QAbstractScrollAreaWrapper
  QWebEngineViewWrapper
  ProcessInteractionFrame)

qemacs_library(TFELQEmacs
  ${CMAKE_CURRENT_BINARY_DIR}/QEmacsBuildInformation.cxx
  Utilities.cxx
  SpellChecker.cxx
  QEmacsHunspellDictionariesManager.cxx
  QEmacsRectangleMode.cxx
  QEmacsGrepCommand.cxx
  QEmacsMajorModeChangeCommand.cxx
  QEmacsCommandFactory.cxx
  QEmacsTextEditQueryReplace.cxx
  QEmacsTextEditReplaceFilter.cxx
  QEmacsMajorModeFactory.cxx
  GrepOutputMajorMode.cxx
  Token.cxx
  CxxTokenizer.cxx
  ${QEmacsResources_RCC}
  ${TFELQEmacs_SOURCES})

target_link_libraries(TFELQEmacs
  TFELSystem
  TFELException
  Qt5::Core
  Qt5::Widgets
  Qt5::Network
  Qt5::WebEngine
  Qt5::WebEngineWidgets
  Qt5::PrintSupport)

qt_sources(qemacs_SOURCES
  "${PROJECT_SOURCE_DIR}/include/QEmacs"
  QEmacsMainWindow)

qemacs_executable(qemacs
  qemacs.cxx
  ${qemacs_SOURCES})

target_link_libraries(qemacs
  TFELQEmacs
  TFELSystem
  TFELException)