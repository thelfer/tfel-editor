add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/python/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(PythonMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/python/include/TFEL/GUI"
  PythonSyntaxHighlighter
  PythonMajorMode)

tfel_editor_package(PythonMode
  ${PythonMode_SOURCES})

target_link_libraries(TFELEditorPythonMode
  TFELEditor)
