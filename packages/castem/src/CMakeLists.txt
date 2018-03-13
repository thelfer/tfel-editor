include_directories("${PROJECT_SOURCE_DIR}/packages/castem/include")
include_directories("${PROJECT_SOURCE_DIR}/utils/mfront/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/utils/mfront/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(CastemQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/castem/include/QEmacs"
  CastemMajorMode)

qt_add_resources(CastemQEmacsResources_RCC
  CastemModeResources.qrc)

qemacs_package(CastemQEmacsMode
  ${CastemQEmacsMode_SOURCES}
  ${CastemQEmacsResources_RCC}
  CastemSyntaxHighlighter.cxx
  CastemOutputMajorMode.cxx)
target_link_libraries(CastemQEmacsMode
  QEmacsMFrontImportWidgets
  TFELQEmacs
  TFELGlossary
  Qt5::Core
  Qt5::Widgets
  )
  
