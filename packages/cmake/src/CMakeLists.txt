add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/cmake/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(CMakeMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/cmake/include/TFEL/GUI"
  CMakeSyntaxHighlighter
  CMakeMajorMode)

tfel_editor_package(CMakeMode
  ${CMakeMode_SOURCES})

target_link_libraries(TFELEditorCMakeMode
  TFELEditor
  Qt5::Core
  Qt5::Widgets
  )
