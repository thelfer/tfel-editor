include_directories("${PROJECT_SOURCE_DIR}/packages/gnuplot/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

tfel_editor_package(GnuplotMode
   GnuplotSyntaxHighlighter.cxx
   GnuplotMajorMode.cxx)

target_link_libraries(TFELEditorGnuplotMode
   TFELEditor)
