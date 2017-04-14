if(enable-qt4)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})
endif(enable-qt4)
include_directories("${PROJECT_SOURCE_DIR}/packages/castem/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(CastemQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/castem/include/QEmacs"
  CastemMajorMode)

qemacs_package(CastemQEmacsMode
  ${CastemQEmacsMode_SOURCES}
  CastemSyntaxHighlighter.cxx)

if(enable-qt5)
  target_link_libraries(CastemQEmacsMode
    TFELQEmacs
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
  