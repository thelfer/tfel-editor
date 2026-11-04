# Coding guidelines

This page gathers the conventions of the code base which are not enforced by
the compiler, the formatter or a review. It is a start: each section is
written down when a choice is made, not before.

## Unit testing

### The framework: `Qt Test`, not `TFEL/Tests`

The `TFEL` project ships its own test framework, `TFEL/Tests`
(`TFEL/Tests/TestCase.hxx`, with `TFEL_TEST_ASSERT`-style macros), and every
other project of the `TFEL` ecosystem uses it. `tfel-editor` does not, for
its GUI tests, and this is deliberate:

- What is under test is a widget: something which only settles once a real
  `QApplication` has run a real event loop. The bugs the tests encode (the
  height of the minibuffer pages, the propagation of a resize, a font change
  reaching every widget) are all about *who is told what, in which order*,
  so a test needs the event loop as part of its fixture, not as an
  inconvenience to work around. `Qt Test` (`Qt6::Test`, `QTEST_MAIN`) provides
  exactly that, with `initTestCase`/`cleanupTestCase` slots and assertions
  which name the expression that failed.
- `Qt Test` is part of the dependency the project already has: it adds no
  third-party library, no new license, and no configure-time requirement
  beyond a component of `Qt`.
- `TFEL/Tests` remains the right tool for code with no event loop — the two
  frameworks are not in conflict, and a future test of, say, a parsing
  utility may well use it. Nothing links `Qt Test` into the library: only the
  test executables do.

### What a test shall test through

Only the public interface. `QSyntaxHighlighter` formats, for instance, are
stored in the document's private extra selections and can not be observed
through public API, so the highlighters are not tested this way. The tests
which need protected members (`minibuffer`, `buffers`) expose them with a
`using` declaration in a subclass, in the test file, rather than making the
production class more public than it needs to be.

### Portability of the assertions

The suite is run headlessly (`QT_QPA_PLATFORM=offscreen`) and is expected to
also pass on a live display, under a window manager. Two traps were paid for:

- A widget shown as its own top-level window does **not** keep the height its
  code asked for: the window manager imposes its own minimum. Show widgets
  inside a host window, as the minibuffer does in the application.
- Absolute pixel heights depend on how the session resolves fonts (the
  offscreen font is not the X11 session font, and the font metrics need not
  agree with the document's line height). Assert equal *increments* per line,
  or equality between two widgets, rather than absolute numbers.

### The one automoc exception

The library declares its `Q_OBJECT` classes through `qt_sources`, which mocs
`<header-dir>/<name>.hxx`; `CMAKE_AUTOMOC` is off everywhere else. The test
classes carry their `Q_OBJECT` in their own `.cxx`, so automoc is enabled
**only** inside `tests/`, and every test file ends with
`#include "<its own name>.moc"`. A new test class with signals or slots
follows the same recipe; it does not add a `.hxx`/`.cxx` pair to the library.

### Other conventions

- The tests are compiled by `build-tests`, never by the default build, and
  run by `check`; a new test executable is registered with
  `add_tfel_editor_test` in `tests/CMakeLists.txt`, which takes care of the
  exclusion from the default build, of `ctest` and of the headless
  environment.
- A test which builds a widget shall isolate the settings first
  (`QCoreApplication::setOrganizationName`/`setApplicationName` in
  `initTestCase`): several widgets read `font/textedit` from `QSettings` when
  they are built, and the tests must neither read the settings of a real
  session nor write into them.
- A test shall call `getCurrentBuffer` (or anything which creates a buffer)
  before any user input is installed on the editor it drives.

See [the description of the unit tests](unit-tests.html).
