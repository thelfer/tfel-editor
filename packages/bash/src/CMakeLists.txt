add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/bash/include")
include_directories("${PROJECT_SOURCE_DIR}/packages/sh/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/packages/sh/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(BashMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/bash/include/TFEL/GUI"
  BashSyntaxHighlighter
  BashMajorMode)

tfel_editor_package(BashMode
  ${BashMode_SOURCES})

target_link_libraries(TFELEditorBashMode
  TFELEditorShMode
  TFELEditor)
