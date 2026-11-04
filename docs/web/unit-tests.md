# Unit tests

The unit tests live in the `tests` directory of the sources and are based on
[`Qt Test`](https://doc.qt.io/qt-6/qttest-index.html), the test framework of
`Qt`, which is the library under test: see
[the coding guidelines](coding-guidelines.html) for why this framework was
chosen rather than `TFEL/Tests`.

The tests are **not built by the default build**. Two targets drive them:

~~~~{.bash}
cmake --build . --target build-tests   # compile the test executables
cmake --build . --target check         # compile them and run them
~~~~

`check` runs the tests through `ctest`, headlessly: the offscreen platform is
imposed, so neither a display nor a window manager is needed. The tests still
run in a real `QApplication`, with a real event loop, and can be run on a live
display (and should be, before a merge touching geometry).

What is covered:

| Test executable           | Subject                                                     |
|---------------------------|-------------------------------------------------------------|
| `test-utilities`          | the free functions of `TFEL/GUI/Utilities.hxx`               |
| `test-multi-line-line-edit` | the height of a multi line entry (one fixed height per line, the padding shared between the frame and the document margin, the measurement width) |
| `test-editor-widget`      | the pages of the minibuffer, the prompt of an entry, the height propagation, `changeFont` |
| `test-main-window`        | the real application shape: a main window with its menu bar and its central editor |

These encode the invariants fixed by
[issue #121](https://github.com/thelfer/tfel-editor/issues/121);
the infrastructure itself comes from
[issue #123](https://github.com/thelfer/tfel-editor/issues/123).
Adding a test is a matter of adding a `.cxx` to `tests/CMakeLists.txt`.
