install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
  DESTINATION share/doc/tfel-editor)
install(FILES
  ${PROJECT_SOURCE_DIR}/docs/doxygen/img/background.svg
  ${PROJECT_SOURCE_DIR}/docs/mfront-en.svg
  ${PROJECT_SOURCE_DIR}/docs/mfront-fr.svg
  ${PROJECT_SOURCE_DIR}/docs/web/img/pipe.svg
  ${PROJECT_SOURCE_DIR}/docs/web/img/plate.svg
  DESTINATION share/doc/tfel-editor/html/img/
  COMPONENT doxygen)
