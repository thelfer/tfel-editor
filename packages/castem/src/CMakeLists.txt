include_directories("${PROJECT_SOURCE_DIR}/packages/castem/include")
include_directories("${PROJECT_SOURCE_DIR}/utils/mfront/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/utils/mfront/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(CastemMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/castem/include/TFEL/GUI"
  CastemMajorMode)

qt_add_resources(CastemResources_RCC
  CastemModeResources.qrc)

tfel_editor_package(CastemMode
  ${CastemMode_SOURCES}
  ${CastemResources_RCC}
  CastemSyntaxHighlighter.cxx
  CastemOutputMajorMode.cxx)
target_link_libraries(TFELEditorCastemMode
  TFELEditorMFrontImportWidgets
  TFELEditor
  TFELGlossary)
  
