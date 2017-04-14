if(enable-qt4)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})
endif(enable-qt4)
include_directories("${PROJECT_SOURCE_DIR}/packages/latex/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(LaTeXQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/latex/include/QEmacs"
  LaTeXMajorMode
  LaTeXSyntaxHighlighter)

qt_add_resources(LaTeXQEmacsResources_RCC
  LaTeXModeResources.qrc)

qemacs_package(LaTeXQEmacsMode
  ${LaTeXQEmacsMode_SOURCES}
  ${LaTeXQEmacsResources_RCC}
  LaTeXOutputMajorMode.cxx)

if(enable-qt5)
  target_link_libraries(LaTeXQEmacsMode
    TFELQEmacs
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
