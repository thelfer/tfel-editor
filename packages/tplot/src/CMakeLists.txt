add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/gnuplot/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFELPLOT_INCLUDE_PATH}")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/packages/gnuplot/src")
link_directories("${TFELPLOT_LIBRARY_PATH}")
link_directories("${TFEL_LIBRARY_PATH}")

tfel_editor_package(TPlotPackage
  TPlotMajorMode.cxx)

target_link_libraries(TFELEditorTPlotPackage
  TFELEditorGnuplotMode
  TFELEditor
  TFELPlot
  Qt6::Core
  Qt6::Widgets
  )
