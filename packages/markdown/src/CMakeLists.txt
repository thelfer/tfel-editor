add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/markdown/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MarkdownQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/markdown/include/QEmacs"
  MarkdownMajorMode
  MarkdownSyntaxHighlighter)

qemacs_package(MarkdownQEmacsMode
  ${MarkdownQEmacsMode_SOURCES})

target_link_libraries(MarkdownQEmacsMode
  TFELQEmacs
  TFELUtilities
  Qt5::Core
  Qt5::Widgets
  )
