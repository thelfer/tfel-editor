add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/mtest/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/utils/mfront/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${PROJECT_BINARY_DIR}/utils/mfront/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MTestMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/mtest/include/TFEL/GUI"
  MTestStudyOptions
  MTestMajorMode
  MTestSyntaxHighlighter
  PipeTestMajorMode
  PipeTestSyntaxHighlighter)

tfel_editor_package(MTestMode
	  ${MTestMode_SOURCES})

target_link_libraries(TFELEditorMTestMode
  TFELEditorMFrontImportWidgets
  TFELEditor
  TFELMTest
  TFELMFront
  TFELMaterial
  TFELSystem)
