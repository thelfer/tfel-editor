# The unit tests are not a part of the default build, see #123: they are
# compiled by the `build-tests` target, which the `check` target of the top
# level `CMakeLists.txt` depends on, so that
#
#   cmake --build . --target check
#
# compiles them and runs them through `ctest`, headlessly. Running the test
# binaries by hand needs `LD_LIBRARY_PATH` to hold the build directory and the
# libraries of TFEL, as the top level does for `ctest`.

find_package(Qt6 6.2 COMPONENTS Test QUIET)

if(NOT Qt6Test_FOUND)
  message(STATUS "Qt Test not found: the unit tests, hence the `build-tests`"
                 " target, are not available")
  return()
endif(NOT Qt6Test_FOUND)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${TFEL_INCLUDE_PATH}")

link_directories("${PROJECT_BINARY_DIR}/src")
link_directories("${TFEL_LIBRARY_PATH}")

# the test classes declare their `Q_OBJECT` macro in their own `.cxx`, which
# `qt_sources` can not handle: it only mocs `<header-dir>/<name>.hxx`. The
# tests are therefore the one place automoc is enabled, and each test includes
# `<its own file name>.moc` at the bottom.
set(CMAKE_AUTOMOC ON)

add_custom_target(build-tests)
add_dependencies(check build-tests)

# add_tfel_editor_test(<name> <sources...>)
#
# defines a test executable which is not built by default, registers it with
# `ctest` and makes `build-tests` depend on it. The offscreen platform plugin
# is imposed so that neither a display nor a window manager is needed; a
# window manager is what an earlier generation of probes had to build a
# private one for, see the note on \c MainWindow.
function(add_tfel_editor_test name)
  add_executable(${name} EXCLUDE_FROM_ALL ${ARGN})
  target_link_libraries(${name} TFELEditor Qt6::Test Qt6::Widgets)
  add_dependencies(build-tests ${name})
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES
    ENVIRONMENT
      "QT_QPA_PLATFORM=offscreen;LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}/src:${TFEL_LIBRARY_PATH}:$ENV{LD_LIBRARY_PATH}")
endfunction(add_tfel_editor_test)

add_tfel_editor_test(test-utilities TestUtilities.cxx)
add_tfel_editor_test(test-multi-line-line-edit TestMultiLineLineEdit.cxx)
add_tfel_editor_test(test-editor-widget TestEditorWidget.cxx)
add_tfel_editor_test(test-main-window TestMainWindow.cxx)
add_tfel_editor_test(test-lsp-protocol TestLspProtocol.cxx)
add_tfel_editor_test(test-lsp-client TestLspClient.cxx)
