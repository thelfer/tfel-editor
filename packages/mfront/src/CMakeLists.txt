if(enable-qt4)
  include(${QT_USE_FILE})
endif(enable-qt4)
add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/mfront/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MFrontQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/mfront/include/QEmacs"
  MFrontMajorMode
  MFrontSyntaxHighlighter)

qt_add_resources(MFrontQEmacsResources_RCC
  MFrontModeResources.qrc)

qemacs_package(MFrontQEmacsMode
  ${MFrontQEmacsResources_RCC}
  ${MFrontQEmacsMode_SOURCES})

if(enable-qt4)
  target_link_libraries(MFrontQEmacsMode
    TFELQEmacs
    TFELMFront)
endif(enable-qt4)

if(enable-qt5)
  target_link_libraries(MFrontQEmacsMode
    TFELQEmacs
    TFELMFront
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
