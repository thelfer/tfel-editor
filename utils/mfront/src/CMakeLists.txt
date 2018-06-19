add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/utils/mfront/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(QEmacsMFrontImportWidgets_SOURCES
  "${PROJECT_SOURCE_DIR}/utils/mfront/include/QEmacs"
  MFMFilterProxyModel
  MaterialPropertiesSelectionPage
  SelectBehaviourPage
  SelectMFMBehaviourPage
  BehaviourSummaryPage
  ImportBehaviourWizard
  ImportMFMBehaviourWizard
  ImportMFMMaterialProperty
  SelectMFMBehaviour
  ImportMFMBehaviour
  MaterialPropertySelectorWidget)

qemacs_package(QEmacsMFrontImportWidgets
  MFMDataBase.cxx
  MaterialPropertyDescription.cxx
  BehaviourDescription.cxx
  ${QEmacsMFrontImportWidgets_SOURCES})

target_link_libraries(QEmacsMFrontImportWidgets
  TFELQEmacs
  TFELMTest
  TFELMFront
  TFELMaterial
  TFELSystem
  Qt5::Core
  Qt5::Widgets
  )