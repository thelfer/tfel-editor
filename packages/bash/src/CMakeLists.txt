add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/bash/include")
include_directories("${PROJECT_SOURCE_DIR}/packages/sh/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/packages/sh/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(BashQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/bash/include/QEmacs"
  BashSyntaxHighlighter
  BashMajorMode)

qemacs_package(BashQEmacsMode
  ${BashQEmacsMode_SOURCES})

target_link_libraries(BashQEmacsMode
  ShQEmacsMode
  TFELQEmacs
  Qt5::Core
  Qt5::Widgets
  )
