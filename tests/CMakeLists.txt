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
add_tfel_editor_test(test-file-on-disk TestFileOnDiskModification.cxx)
add_tfel_editor_test(test-main-window TestMainWindow.cxx)
add_tfel_editor_test(test-lsp-protocol TestLspProtocol.cxx)
add_tfel_editor_test(test-lsp-client TestLspClient.cxx)
add_tfel_editor_test(test-grammar-check TestGrammarCheck.cxx)
add_tfel_editor_test(test-settings TestSettings.cxx)

# The grammar checker is a program of its own, and the cases which ask it anything
# are skipped when it is not installed. The library of the `LaTeX` mode is needed
# by the last cases, which load those modes as plugins: the context menu of a text
# mode is the one public face of hunspell a test can see, and the packaged modes
# are what the editor loads, not what it links
add_dependencies(test-grammar-check TFELEditorLaTeXMode)
target_compile_definitions(
  test-grammar-check PRIVATE
  TFEL_EDITOR_LATEX_MODE="$<TARGET_FILE:TFELEditorLaTeXMode>")

# The very last case looks at the menu of the `Markdown` mode, the other text mode
# which was asked to offer the checker. Loading it registers the mode for the rest
# of the run, which is why no case may follow it.
add_dependencies(test-grammar-check TFELEditorMarkdownMode)
target_compile_definitions(
  test-grammar-check PRIVATE
  TFEL_EDITOR_MARKDOWN_MODE="$<TARGET_FILE:TFELEditorMarkdownMode>")

# One case of `test-settings` loads a packaged major mode as the editor does, as
# a plugin, and asks the registry what that mode declared: the case needs the
# built library and has to know where it is. Loading it needs no help from the
# environment either, since a plugin of the build tree carries the absolute path
# of every library it needs in its own run path.
add_dependencies(test-settings TFELEditorCastemMode)
target_compile_definitions(
  test-settings PRIVATE
  TFEL_EDITOR_CASTEM_MODE="$<TARGET_FILE:TFELEditorCastemMode>")
