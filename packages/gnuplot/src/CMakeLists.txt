include_directories("${PROJECT_SOURCE_DIR}/packages/gnuplot/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

tfel_editor_package(GnuplotMode
  GnuplotSyntaxHighlighter
  GnuplotMajorMode.cxx)

target_link_libraries(TFELEditorGnuplotMode
  TFELEditor
  Qt5::Core
  Qt5::Widgets
  )
