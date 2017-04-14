if(enable-qt4)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})
endif(enable-qt4)
include_directories("${PROJECT_SOURCE_DIR}/packages/licos/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(LicosQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/licos/include/QEmacs"
  LicosMajorMode
  LicosSyntaxHighlighter
  LicosStudyOptions
  LicosStudyThread
  LicosOutputFrame
  LicosStudy
  MaterialPropertySelector
  LicosInsertMaterialPropertyDialog)

qt_add_resources(LicosQEmacsResources_RCC
  LicosModeResources.qrc)

qemacs_package(LicosQEmacsMode
  ${LicosQEmacsMode_SOURCES}
  ${LicosQEmacsResources_RCC}
  LicosBlock.cxx
  StandardLicosBlock.cxx
  LicosMainBlock.cxx
  LicosTokenizer.cxx
  LicosMaterialWizard.cxx)

if(enable-qt5)
  target_link_libraries(LicosQEmacsMode
    TFELQEmacs
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
