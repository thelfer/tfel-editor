add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/markdown/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MarkdownMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/markdown/include/TFEL/GUI"
  MarkdownMajorMode
  MarkdownSyntaxHighlighter)

tfel_editor_package(MarkdownMode
  ${MarkdownMode_SOURCES})

target_link_libraries(TFELEditorMarkdownMode
  TFELEditor
  TFELUtilities
  Qt6::Core
  Qt6::Widgets
  )
