if(enable-qt4)
  include(${QT_USE_FILE})
endif(enable-qt4)
add_definitions(${QT_DEFINITIONS})
include_directories("${PROJECT_SOURCE_DIR}/packages/make/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

qt_sources(MakefileQEmacsMode_SOURCES
  "${PROJECT_SOURCE_DIR}/packages/make/include/QEmacs"
  MakefileMajorMode)

qemacs_package(MakefileQEmacsMode
  ${MakefileQEmacsMode_SOURCES})

if(enable-qt5)
  target_link_libraries(MakefileQEmacsMode
    TFELQEmacs
    Qt5::Core
    Qt5::Widgets
    )
endif(enable-qt5)
