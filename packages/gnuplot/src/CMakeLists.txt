if(enable-qt4)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})
endif(enable-qt4)
include_directories("${PROJECT_SOURCE_DIR}/packages/gnuplot/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qemacs_package(GnuplotQEmacsPackage
  GnuplotSyntaxHighlighter
  GnuplotMajorMode.cxx)

if(enable-qt5)
  target_link_libraries(GnuplotQEmacsPackage
    TFELQEmacs
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
