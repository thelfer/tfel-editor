if(enable-qt4)
  include(${QT_USE_FILE})
endif(enable-qt4)
add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/gnuplot/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/packages/gnuplot/src")
link_directories("${TFEL_LIBRARY_PATH}")

qemacs_package(TPlotQEmacsPackage
  TPlotMajorMode.cxx)

if(enable-qt4)
  target_link_libraries(TPlotQEmacsPackage
    GnuplotQEmacsPackage
    TFELQEmacs
    TFELGraphics
    )
endif(enable-qt4)


if(enable-qt5)
  target_link_libraries(TPlotQEmacsPackage
    GnuplotQEmacsPackage
    TFELQEmacs
    TFELGraphics
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
