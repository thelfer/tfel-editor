# Unit tests

The tests are **not built by the default build** (issue #123): configuring
costs nothing and `cmake --build .` compiles none of them.

```bash
cmake --build . --target build-tests   # compile the test executables
cmake --build . --target check         # compile them and run them via ctest
```

`check` runs every test with `QT_QPA_PLATFORM=offscreen` and an
`LD_LIBRARY_PATH` pointing at the freshly built library and the libraries of
TFEL, so neither a display nor a window manager is needed. Running a test
binary by hand needs `LD_LIBRARY_PATH` to hold `<build>/src` and
`$TFELHOME/lib`; with `DISPLAY` set it runs on the live session instead, which
is how geometry invariants are checked against a window manager as well
(see the note on `metacity` in the repository `AGENTS.md`).

The framework is **Qt Test** (`Qt6::Test`): the widgets are what is under
test, so a `QApplication` and an event loop are part of the fixture. If
`Qt6::Test` is not found at configure time, the whole directory silently
skips itself and `check` keeps the behavior it had before the tests existed.

Unlike the rest of the project, the test classes carry their `Q_OBJECT` macro
in their own `.cxx`: `qt_sources` only mocs `<header-dir>/<name>.hxx`, so
automoc is enabled locally here and every test file ends with
`#include "<its own name>.moc"`.

What each binary covers:

| Test                     | Subject                                                    |
|--------------------------|------------------------------------------------------------|
| `test-utilities`         | the free functions of `TFEL/GUI/Utilities.hxx`              |
| `test-multi-line-line-edit` | the height of a multi line entry (issue #121)          |
| `test-editor-widget`     | the minibuffer pages, the prompt, the height propagation and `changeFont` |
| `test-main-window`       | the real application shape (needs the exported `MainWindow`) |
