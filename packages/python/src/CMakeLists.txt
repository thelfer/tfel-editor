add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")
link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qemacs_package(PythonQEmacsMode
  PythonMajorMode.cxx)

target_link_libraries(PythonQEmacsMode
  TFELQEmacs
  Qt5::Core
  Qt5::Widgets
  )
