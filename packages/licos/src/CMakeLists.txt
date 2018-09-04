include_directories("${PROJECT_SOURCE_DIR}/packages/licos/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(LicosMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/licos/include/TFEL/GUI"
  LicosMajorMode
  LicosSyntaxHighlighter
  LicosStudyOptions
  LicosStudyThread
  LicosOutputFrame
  LicosStudy
  MaterialPropertySelector
  LicosInsertMaterialPropertyDialog)

qt_add_resources(LicosResources_RCC
  LicosModeResources.qrc)

tfel_editor_package(LicosMode
  ${LicosMode_SOURCES}
  ${LicosResources_RCC}
  LicosBlock.cxx
  StandardLicosBlock.cxx
  LicosMainBlock.cxx
  LicosTokenizer.cxx
  LicosMaterialWizard.cxx)

target_link_libraries(TFELEditorLicosMode
  TFELEditor
  Qt5::Core
  Qt5::Widgets
  )
