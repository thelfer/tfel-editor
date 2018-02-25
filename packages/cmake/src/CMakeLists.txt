add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/cmake/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(CMakeQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/cmake/include/QEmacs"
  CMakeSyntaxHighlighter
  CMakeMajorMode)

qemacs_package(CMakeQEmacsMode
  ${CMakeQEmacsMode_SOURCES})

target_link_libraries(CMakeQEmacsMode
  TFELQEmacs
  Qt5::Core
  Qt5::Widgets
  )
