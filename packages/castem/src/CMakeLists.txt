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
target_link_libraries(CastemQEmacsMode
  TFELQEmacs
  Qt5::Core
  Qt5::Widgets
  )
  
