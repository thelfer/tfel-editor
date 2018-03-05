add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/mtest/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MTestQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/mtest/include/QEmacs"
  MFMFilterProxyModel
  ImportBehaviour
  ImportMFMMaterialProperty
  ImportMFMBehaviour
  MTestStudyOptions
  MTestMajorMode
  MTestSyntaxHighlighter
  PipeTestMajorMode
  PipeTestSyntaxHighlighter
  MaterialPropertySelectorWidget)

qemacs_package(MTestQEmacsMode
  MFMDataBase.cxx
  BehaviourDescription.cxx
  ${MTestQEmacsMode_SOURCES})

target_link_libraries(MTestQEmacsMode
  TFELQEmacs
  TFELMTest
  TFELMFront
  TFELMaterial
  TFELSystem
  Qt5::Core
  Qt5::Widgets
  )
