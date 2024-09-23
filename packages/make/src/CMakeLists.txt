add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/make/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MakefileMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/make/include/TFEL/GUI"
  MakefileSyntaxHighlighter
  MakefileMajorMode)

tfel_editor_package(MakefileMode
  ${MakefileMode_SOURCES})

target_link_libraries(TFELEditorMakefileMode
  TFELEditor
  Qt6::Core
  Qt6::Widgets
  )
