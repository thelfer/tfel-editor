add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/sh/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(ShQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/sh/include/QEmacs"
  ShSyntaxHighlighter
  ShMajorMode)

qemacs_package(ShQEmacsMode
  ${ShQEmacsMode_SOURCES})

target_link_libraries(ShQEmacsMode
  TFELQEmacs
  Qt5::Core
  Qt5::Widgets
  )
