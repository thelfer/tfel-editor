include_directories("${PROJECT_SOURCE_DIR}/packages/latex/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(LaTeXMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/latex/include/TFEL/GUI"
  LaTeXMajorMode
  LaTeXSyntaxHighlighter)

qt_add_resources(LaTeXResources_RCC
  LaTeXModeResources.qrc)

tfel_editor_package(LaTeXMode
  ${LaTeXMode_SOURCES}
  ${LaTeXResources_RCC}
  LaTeXOutputMajorMode.cxx)

target_link_libraries(TFELEditorLaTeXMode
  TFELEditor
  Qt6::Core
  Qt6::Widgets)
