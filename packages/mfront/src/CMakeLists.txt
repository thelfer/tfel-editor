add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/mfront/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MFrontMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/mfront/include/TFEL/GUI"
  MFrontMajorMode
  MFrontSyntaxHighlighter
  MFrontOptionsDialog
  ImplicitDSLWizard)

qt_add_resources(MFrontResources_RCC
  MFrontModeResources.qrc)

tfel_editor_package(MFrontMode
  ${MFrontResources_RCC}
  ${MFrontMode_SOURCES}
  MFrontOptions.cxx
  MFrontInitializer.cxx)

target_link_libraries(TFELEditorMFrontMode
  TFELEditor
  TFELMFront
  TFELMaterial
  TFELUtilities
  Qt5::Core
  Qt5::Widgets
  )
