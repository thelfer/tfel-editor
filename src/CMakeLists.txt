include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")
link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_add_resources(Resources_RCC
  Resources.qrc)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/BuildInformation.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/BuildInformation.cxx" @ONLY)

qt_sources(TFEL_EDITOR_SOURCES
  "${PROJECT_SOURCE_DIR}/include/TFEL/GUI"
  EditorWidget
  LineEdit
  FileDownloader
  ShortCutStyle
  Command
  CommandLine
  ProcessLineEdit
  ShellProcessLineEdit
  Buffer
  TextEditSearch
  TextEditKeyPressEventFilter
  TextEditBase
  PlainTextEdit
  TextEdit
  TextBrowser
  CSyntaxHighlighter
  CSyntaxHighlighterBase
  MajorMode
  MajorModeBase
  CompiledLanguageMajorModeBase
  CMajorModeBase
  CxxMajorMode
  CxxSyntaxHighlighter
  ProcessOutputFrame
  ProcessOutputMajorModeBase
  QAbstractScrollAreaWrapper
  ProcessInteractionFrame
  DirectoryMajorMode)

if(WebEngine_FOUND AND WebEngineWidgets_FOUND)
  qt_sources(TFEL_EDITOR_SOURCES2
  "${PROJECT_SOURCE_DIR}/include/TFEL/GUI"
  QWebEngineViewWrapper)
endif(WebEngine_FOUND AND WebEngineWidgets_FOUND)

tfel_editor_library(TFELEditor
  ${CMAKE_CURRENT_BINARY_DIR}/BuildInformation.cxx
  VerboseLevel.cxx
  SecondaryTask.cxx
  SecondaryTaskManager.cxx
  Utilities.cxx
  SpellChecker.cxx
  StandardFunctionCommand.cxx
  HunspellDictionariesManager.cxx
  RectangleMode.cxx
  GrepCommand.cxx
  MajorModeChangeCommand.cxx
  CommandFactory.cxx
  TextEditQueryReplace.cxx
  TextEditReplaceFilter.cxx
  MajorModeFactory.cxx
  GrepOutputMajorMode.cxx
  CompilationOutputMajorMode.cxx
  ${Resources_RCC}
  ${TFEL_EDITOR_SOURCES})

set(QT_LIBRARIES
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::PrintSupport)

if(WebEngine_FOUND AND WebEngineWidgets_FOUND)
set(QT_LIBRARIES
  ${QT_LIBRARIES}
  Qt5::WebEngine
  Qt5::WebEngineWidgets)  
endif(WebEngine_FOUND AND WebEngineWidgets_FOUND)

if(HUNSPELL_FOUND)
message("dir: ${HUNSPELL_INCLUDE_DIRS}")
  target_include_directories(TFELEditor
    PRIVATE "${HUNSPELL_INCLUDE_DIRS}")
endif(HUNSPELL_FOUND)
  
if(HUNSPELL_FOUND)
  target_link_libraries(TFELEditor
    TFELSystem
    TFELUtilities
    TFELException
    ${QT_LIBRARIES}
    ${HUNSPELL_LIBRARIES})
else(HUNSPELL_FOUND)
  target_link_libraries(TFELEditor
    TFELSystem
    TFELUtilities
    TFELException
    ${QT_LIBRARIES})
endif(HUNSPELL_FOUND)

qt_sources(tfel-editor_SOURCES
  "${PROJECT_SOURCE_DIR}/include/TFEL/GUI"
  MainWindow)

tfel_editor_executable(tfel-editor
  tfel-editor.cxx
  ${tfel-editor_SOURCES})

target_link_libraries(tfel-editor
  TFELEditor
  TFELSystem
  TFELException)
