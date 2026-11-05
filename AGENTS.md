# tfel-editor

C++20 / Qt6 Emacs-like editor for `TFEL`/`MFront` files. Ships as a shared
library `TFELEditor` plus a thin executable `tfel-editor`; each language mode is
a separately loaded plugin `.so`.

## Build

There is no in-tree build config. Builds are out-of-source in sibling dirs
(`../build`, `../build-debug`, …); the repo has **no `.gitignore`**, so never
create build dirs inside the source tree.

```bash
source <TFEL-install-prefix>/env.sh   # sets TFELHOME, PATH and LD_LIBRARY_PATH
mkdir -p ../build && cd ../build      # sibling of the source dir, as in this checkout
cmake ../tfel-editor -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . -j 8
```

(`INSTALL.md` shows an in-tree `build/`; that would show up as untracked noise
because there is no `.gitignore`. The existing checkouts use sibling
`../build`, `../build-debug`, … instead.)

Single target (much faster while iterating): `cmake --build . --target TFELEditorMFrontMode`.

Gotchas, all verified:

- **Sourcing the TFEL `env.sh` is mandatory, not just setting `TFELHOME`.**
  `cmake/modules/tfel.cmake` runs `mfront --list-behaviour-interfaces` at
  *configure* time. If `mfront` cannot resolve its own `libTFELMFront.so`
  (i.e. `LD_LIBRARY_PATH` is unset), the symptom is a misleading
  `string sub-command REGEX, mode MATCHALL needs at least 5
  arguments` error from `cmake/modules/tfel.cmake`, not a "TFEL not found".
- A TFEL install providing `tfel-config` **and** `mfront` is required; without
  them configure stops with `FATAL_ERROR "tfel not found"`.
- **TFEL must match this branch's vintage.** `master` tracks `TFEL` `master`
  (see `README.md`); `rliv-1.1` ↔ TFEL 3.3.x, `rliv-1.0` ↔ 3.2.x. Fatal
  `MFront/….hxx: No such file or directory` errors in `packages/mfront` mean
  the TFEL install is the wrong side of an API change — pick a different
  `$TFELHOME/install-*`, do not "fix" the editor sources.
  (Verified: `install-python-3.10.12` builds the core library *and*
  `TFELEditorMFrontMode`, whose `MFrontOptionsDialog.cxx` includes
  `MFront/AbstractNonLinearSystemSolver.hxx`. `install-llvm-21.1.0` and
  `install-gcc-15.1.0` are older snapshots which only have the pre-rename
  `MFront/NonLinearSystemSolver.hxx`: with them the core library, the
  executable and 11 of the 12 modes build, only `mfront` fails — so check the
  mfront target too before concluding an install is unusable, and before
  blaming a stale pre-existing cache such as `../build`.)
- `cmake/modules/compiler.cmake` **overwrites** `CMAKE_CXX_FLAGS*` unless
  `-DUSE_EXTERNAL_COMPILER_FLAGS=ON`. Passing `-DCMAKE_CXX_FLAGS=…` on a normal
  configure is silently discarded.
- `-march=native` is added by default (via `OPTIMISATION_FLAGS_MARCH`), so
  release binaries are not portable.
- Warnings are aggressive (`-Wall -Wshadow -Wold-style-cast -Wcast-qual`-style
  list in `cmake/modules/common-compiler-flags.cmake`) but there is
  **no `-Werror`**; a successful build is not the same as a warning-free one,
  and the existing tree already emits many warnings.
- Useful options: `-Denable-tfel-plot=ON` (adds the `tplot` package, needs
  `tfel-plot`), `-Denable-developer-warnings=ON`, `-DUSE_EXTERNAL_COMPILER_FLAGS=ON`
  (for conda-style builds).
- Optional: `hunspell` (auto-detected; absent ⇒ `TFEL_GUI_HUNSPELL_SUPPORT` is
  not defined and spell-checking is compiled out). **It is found here**
  (`/usr/include/hunspell`, `libhunspell-1.7`), so the spell-check underlines
  and the context-menu suggestions are live code, not compiled-out stubs.
- `-Denable-tfel-plot=ON` was not tested here; `tplot` is the one mode absent
  from a default build (15 targets instead of 16).

### Syntax highlighter lifetime (issue #20, still worth knowing)

`QSyntaxHighlighter` registers itself on the `QTextDocument` it is built with,
so it is owned by the *document*, not by the mode. `MajorMode` therefore keeps
only a `QPointer<QSyntaxHighlighter>` and `~MajorMode()` detaches it
(`setDocument(nullptr)` + `deleteLater()`): `TextEditBase::setMajorMode()`
`deleteLater()`s the mode being replaced while the document, and hence the
highlighter, survive — and the LaTeX and Markdown highlighters hold a
`Mode&`, so the next rehighlight of the buffer would call into a destroyed
mode. `setHighlighter()` does the same on replacement, so the fix in
`src/MajorMode.cxx` covers every mode, and modes call it rather than
`new`ing a highlighter without storing it.

### Auditing a mode without a test suite

The only way to see what a highlighter decided is behavioural: a
`QSyntaxHighlighter` stores its formats in the document's *private* extra
selections, which `QTextCursor::charFormat()` ignores (verified with a
standalone highlighter: `setFormat(0, 5, red)` is invisible to
`charFormat()`). Drive the mode through a public entry point instead — for
spell checking, `MajorMode::completeContextMenu()` and the number of actions
the menu ends up with.

### Driving the GUI from a probe: offscreen, X11, and a window manager

Three levels, and they disagree, so use all three. `QT_QPA_PLATFORM=offscreen` is
the cheapest and the one that finds the real bugs (a vertical-scrollbar feedback
loop that made an input one line too tall showed up there and *only* there). Plain
X11 (`DISPLAY=:1`, the user's session) agrees with it on geometry but not on every
number, and root-window grabs there include other people's windows. For anything
involving the window manager — moving or resizing a window — start a private one:

```bash
Xvfb :99 -screen 0 1400x900x24 &      # metacity, kwin and xfwm4 are all installed
DISPLAY=:99 nohup metacity &          # ~2 s until _NET_SUPPORTING_WM_CHECK answers
```

Gotchas, all paid for: metacity moves a window on **Super+Button1** (keycode 133);
Alt+Button1 does nothing. Its decoration reparents the window, so
the client's `Absolute upper-left Y` from `xwininfo -name X` is ~47 px *below* the
frame: drag at `client_y - 12` to catch the title bar. XTEST through `ctypes`
segfaults unless `XOpenDisplay.restype = c_void_p` and the `argtypes` of every
`XTestFake*` are set — the `Display*` is truncated to 32 bits otherwise. There is no
`xdotool` or `wmctrl`; `xwininfo`, `xprop`, `import` and `compare` (ImageMagick) are
there, so screenshots come from `import -window root` and stale pixels are found
with `compare -metric AE`. A/B two trees by building the same probe twice and
switching `LD_LIBRARY_PATH` between the branch build and a pristine `master` build.

### One height per minibuffer page (issue #121, keep this invariant)

`EditorWidget`'s minibuffer is a `QWidget` with a `QVBoxLayout` holding several
pages (`um`, `eui`, and the `CommandLine`) and **is as tall as the page it shows**,
so a page whose single line is one pixel taller than another resizes `buffers`,
that is the text edit, every time the page changes. Keep the height of a single
line equal across pages: `MultiLineLineEdit::alignDocumentMargin()` shares one
fixed `line_padding` between the frame of the widget and the document margin,
because `CommandLine` builds a frameless input and the message area a framed one.
Check it by printing the heights of the visible minibuffer pages, not by looking;
the 1 s `Buffer::updateDate` timer and the 2 s `messageTimer` hide the symptom
inside a second or two, which is how it reads as "an empty zone that disappears
after a few seconds".

`updateHeight()` must also measure in the **width the text is drawn in**
(`width() - 2 * frameWidth() - viewportMargins()`), not in `viewport()->width()`:
the viewport of a widget resized while hidden still has its old width, and a shown
vertical scrollbar takes 14 px from it, which asks for a line more, which keeps the
scrollbar — a latch that leaves the input permanently one line too tall. Measured
with the widget fixed to one line, the window minimum is 240 px against 244 px for a
`QLineEdit` there, so the `minimumSizeHint` of 70 px a scroll area reports does not
leak into the window minimum.

### The prompt of a multi-line entry is on its first line (issue #121, `ac3ddec`)

A `QLabel` whose vertical size policy is `Fixed` in a `QHBoxLayout` is **centred** in
the cross direction by the box, whatever `QLabel::setAlignment` says — that only
places the text inside the widget. So the minibuffer prompt sat in the middle of an
entry that had grown to three lines. Fix: `hl->addWidget(label, 0, Qt::AlignTop)`
(alignment on the *layout item*), plus a top contents margin on the label equal to
`MultiLineLineEdit::firstLineTop()` (frame + viewport margin + document margin, 2 px
with the shared `line_padding`), which is why the prompt and the entry's text start
on the same line and not 2 px apart. `firstLineTop()` is public on the shim;
`singleLineHeight()` and `boundedWindow()` stay private. `/tmp/opencode/probe_events/fa121x.cxx`
prints the delta between the two in a real main window: 0 px at 1, 3 and 8 lines on
both platforms (it was +18 px and +51 px below the first line before).

### The height propagation stops at the widget the window sizes (issue #121, `747f291`)

`MultiLineLineEdit::propagateHeightToLayout()` hands the height of the input up the
parent chain, resizing each widget to its layout's hint. It must stop at the first
widget whose height is not its contents' business — a vertical policy of
`Expanding`/`MinimumExpanding`, a non-zero vertical stretch, a parent that is a
`QSplitter`, or a widget taller than `totalSizeHint() + singleLineHeight()`. In the
application the chain is input → `CommandLine` → minibuffer → **`EditorWidget`** →
`MainWindow`: resizing that last one to the 240 px its own layout asks for took the
677 px the window had given it, and because it is stuck to the bottom edge it moved
down, leaving the window's gray background above it. `Ctrl-X Ctrl-F` alone triggers it
(installing any user input asks it for its height).

A probe cannot catch this while it shows the editor as its own window: the walk has no
parent above the editor there. `MainWindow` is **not exported** (`struct MainWindow`,
no `TFEL_GUI_VISIBILITY_EXPORT`), so a probe cannot build it either — build a
`QMainWindow` with a menu bar and the `EditorWidget` as central widget instead, which
is the shape that reproduces it (`/tmp/opencode/probe_events/fa121w.cxx`).

### A font change has to reach every widget which read the setting itself

`font/textedit` in `QSettings` is applied **at construction** by
`setFontSizeAndContentsMargins` / `setQLabelFontSizeAndMargins` (`Utilities.cxx`,
called only from `Buffer.cxx`: the tab bar of the secondary tasks and the five
mode-line labels), `EditorWidget::setQLineEditFont` (`um`, `eui`),
`setCommandLineFont` (`CommandLine`), `Buffer::attachSecondaryTask` (attached
scroll areas) and `TextEditBase::initialize` (text edits). Those widgets are not
children of one another, and Qt never replaces an **explicit** font with the
inherited one, so `MainWindow::selectFont` has to hand the font to each of them:
that is `EditorWidget::changeFont` (was `changeMainFramesFont`, which only did
the main frames and left the mode line stale — a mode line is a horizontal box
whose height its tallest label asks for, so a stale label is a mode line the
height of two lines), which calls `Buffer::changeFont` (main frame, the four
text labels — not `ii`, the pixmap icon —, `stw->tabBar()`, and the
`TextEditBase`/`QAbstractScrollArea`/`QAbstractScrollAreaWrapper` pages of the
tab widget), sets `um` and `eui`, and calls `CommandLine::changeFont` for every
input waiting for an answer (container font, the input's explicit font, the
label's `firstLineTop()` margin, `completions`). Anything built afterwards reads
the setting itself, which is what made the staleness look selective.
`MultiLineLineEdit::changeEvent` recomputes its height on `FontChange`, so the
minibuffer follows the font by itself. `MainWindow`'s own buffer tabs are **not**
covered: they never read the setting, they use the application font, before and
after. Verify with a probe printing `font().family()/pointSize()` of each widget
(`/tmp/opencode/probe_events/fa121y.cxx`), not with `QWidget::grab()` under
`offscreen`, which is stale for font-only changes.
## Tests / CI

Tests exist (issue #123, Qt Test based) but are **not built by default**: they
live in `tests/`, their executables are `EXCLUDE_FROM_ALL`, and `check` depends
on a `build-tests` target. The verification commands are

```bash
cmake --build . --target build-tests   # compile the tests
cmake --build . --target check         # compile and run them via ctest
```

`ctest` and `check` work after configuring and running the project
(`env.sh` sourced): `check` sets `QT_QPA_PLATFORM=offscreen` and an
`LD_LIBRARY_PATH` holding `<build>/src` and `${TFEL_LIBRARY_PATH}` per test.
If `Qt6::Test` is not installed at configure time, `tests/` skips itself
silently and `check` degenerates to the old `No tests were found!!!`.
The test classes carry `Q_OBJECT` in their `.cxx`, so automoc is enabled
**only** inside `tests/` (the rest of the project uses `qt_sources`, which
mocs `<header-dir>/<name>.hxx` exclusively), and each test ends with
`#include "<own name>.moc"`.

There is still no CI and no pre-commit hook.

Geometry assertions must hold on both levels of `AGENTS.md`'s three: run the
binaries under `DISPLAY=:99` (Xvfb + metacity) as well as offscreen, at least
before merging. Two verified traps: a bare entry shown as its own top-level
window does **not** keep the height `updateHeight` asked for on a live display
(the window manager imposes its own minimum — show widgets inside a host
window), and absolute pixel heights differ between platforms because the
session font (X11: Sans 10) is not the offscreen font (Sans Serif 9, and
`fontMetrics().height()` there is not the document's line height) — assert
equal *increments* per line, not absolute heights.

`MainWindow` is exported (`TFEL_GUI_VISIBILITY_EXPORT`) and `MainWindow.cxx`
compiles into `libTFELEditor` (it used to belong to the executable only), so
tests and packages can construct the real application shape.

Behaviour of the *language modes* remains untestable through the public API
(a `QSyntaxHighlighter` stores its formats in the document's private extra
selections); the probes below remain the tool for that.

Minibuffer behaviour can still be checked headlessly: link a throwaway
`.cxx` against `<build>/src/libTFELEditor.so`, run it with
`QT_QPA_PLATFORM=offscreen` and `LD_LIBRARY_PATH` set to both the TFEL `lib`
dir and the build dir, `show()` the `EditorWidget` (secondary tasks are only
registered when the buffer is visible), call `processEvents()` and drive the
input through the public `CommandLine::keyPressEvent`. Two gotchas, both
verified: `getCurrentBuffer()` must be called *before* the user input is
constructed, and a user input must be reached through the protected `input`
member — `findChild<QLineEdit*>()` returns the stale `CustomLineEdit` the
constructor replaced, since `setLineEdit` drops it with `deleteLater`, which
`processEvents()` does not run.

### Answering a minibuffer question from a test class (issue #125)

Three traps, all paid for. The input of a `CommandLine` is a `LineEdit`, i.e.
a `QTextEdit` shim, **not** a `QLineEdit`, so `findChild<QLineEdit*>()` finds
nothing at all. An input which was answered lingers as a live child of the
editor until the `deleteLater` its removal asked for runs, so
`findChild<FilePathUserInput*>()` finds the *previous* question and answering it
does nothing: the editor keeps waiting and `hasUserInput()` stays true. And
focus is no help, because `setFocus` on a widget of an inactive offscreen window
puts the focus nowhere, so `QApplication::focusWidget()` is null. What is public
and unambiguous: the widget the layout of its parent still holds is the live one
(`parent->layout()->indexOf(w) >= 0`), and the answer is given through
`CommandLine::userEditingFinished()`, the public slot `Return` triggers. See
`TestLspClient::isLaidOut` and `TestLspClient::answerUserInput`.

### The clangd session is launched by the user (issue #125)

The mode never starts a server: `LspClient::launch(extra_arguments, error)` and
`isIdle()` replaced `ensureStarted`, `waitUntilReady` waits for a handshake which
was asked for instead of starting one, and the two entries of
`CxxMajorMode::getSpecificMenu` (menu "C/C++"), `launch-clangd` and
`launch-clangd-with-compile-commands`, are what starts a session, with
`Stop clangd`/`stop-clangd` (`LspClient::stop`, which reports "clangd is
not launched" when none was running) what ends one. Relaunching
ends the running session first (`LspClient::endSession`, shared with the
destructor, plus the `ending_session` flag that keeps that exit from reporting
"clangd exited" in the minibuffer), resets every document version to 0 so
`flushDocuments()` re-sends `didOpen` after the new handshake, and clears the
diagnostics the dismissed server published. The answer of the file question
becomes `--compile-commands-dir=<dir>` in
`LspCxxSupport::compileCommandsArguments`, tested without a server in
`test-lsp-protocol`.

### Which database a plain launch uses (issue #125)

A session launched without an explicit database takes
`LspCxxSupport::defaultCompileCommandsFile()` — the `--compile-commands-file=`
option parsed in `main` of `tfel-editor.cxx` into a file-scope static —, else
the result of `LspCxxSupport::searchCompileCommandsFile(directory_of_buffer)`,
else nothing. The search ascends while the *parent* holds a `CMakeLists.txt`
(a `src/` without its own `CMakeLists.txt` still belongs to the parent's
project; the alternative reading, ascending while the *current* directory has
one, stops at the first leaf and misses the database at the project root),
then looks into a `build*` sub-directory of the last project directory and of
the directory outside the project which holds it. The editor searches rather
than leaving it to clangd because `--compile-commands-dir` is fixed at launch
and the minibuffer names the database the session runs with. The plain launch
reports the outcome once, not the miss as it happens: `startClangd` takes a
`report` which replaces the `clangd launched/restarted` message whole — `using
<path> to start clangd` or `no compile_commands.json found, clangd is
restricted to current C++ file`, with `(default file <path> not found)` appended
only when a bad default actually played no part. Two test traps:
a database left in a shared `QTemporaryDir` root is found by every later plain
launch of the mode tests (`testModeLaunchesWithACompilationDatabase` therefore
lives in its own `db-project/` subtree), and a database proving "the search
found it" must be named something other than `build*` when it is meant to be
reachable by the command-line default only — the `#ifdef X` + `-DX` entry with
an error inside is what makes "the database reached clangd" a positive wait.

### `build` wins among several `build*` directories (issue #125, `d9611f6`)

`databaseInBuildSubDirectory` tries plain `build` first (it is what a bare
`cmake -B build` gives), the others in name order, and a candidate without a
`compile_commands.json` hands over to the next. The order must be made explicit:
`QDir::entryList` without sorting flags returns the file system's order, and Qt
6.2 **has no three-argument `entryList(nameFilters, Filters, SortingFlags)`
overload** — `QDir::Name | QDir::CaseSensitive` mixes two enums into a
`QIncompatibleFlag` and does not compile; sort the `QStringList` by hand.
`test-lsp-protocol`'s `pri` case covers both the precedence and the hand-over.

### Only the cursor's word is underlined; the mouse gets a jump icon (issue #125)

`CxxMajorMode` tracks two words, the one under the text cursor and the one
under the mouse, through one shared 150 ms single-shot `navigation_timer`
(restarted by `cursorPositionChanged` and by `MouseMove` on the viewport, for
which the constructor enables `setMouseTracking` and installs the mode's
`eventFilter`). Each word is asked about once (`requestDefinition` *and*
`requestDeclaration`, combined by a shared pending counter) and its answer
kept in a `TrackedWord` until the word, the document revision or the session
changes (`resetNavigation` runs on rename and on `LspClient::stopped`). The
cursor's word gets a `setExtraSelections("clangd-navigation", ...)` underline
in `QPalette::Link`; the mouse's word gets the viewport cursor replaced by
`jump_cursor("go-jump-definition"/"go-jump-declaration")` — theme icon, else
`go-jump`, else a 22 px pixmap, else `Qt::PointingHandCursor`.

Three traps, all paid for. `TrackedWord::capabilities == -1` means "unknown",
and `-1` has **every bit set**: a `(capabilities & HAS_DEFINITION) != 0` test
on an unknown word is true, so an unresolved hover *set* the pointing-hand
fallback on the viewport permanently — gate any bit test with
`capabilities > 0`. `QWidget::unsetCursor()` does **not** restore the IBeam of
a text edit's viewport (Qt sets it *explicitly*, so the unset falls back to
the inherited Arrow over the text); hand back `Qt::IBeamCursor` explicitly.
And a test's precondition "the viewport has no jump cursor yet" is exactly the
assertion that finds such a bug — do not relax it, find the setter (gdb
breakpoint on `QWidget::setCursor` with a `bt` names it in one run).

The jump itself stays on `Alt+.` and the context menu, never on left click:
a left click must keep pointing inside the file. And the context menu asks
`textDocument/definition` **only** — `clangd` already answers that question
with the declaration of a symbol which has no body (verified against
clangd-15), so a second `textDocument/declaration` question per right click
would double the menu's bound wait and never change the answer.

### Diagnostic messages, and the references menu (issue #125)

The text of a diagnostic is echoed by `echoDiagnosticAtCursor` when the
settling timer expires with the cursor on an underlined range (memory of
what was last echoed, `echoed_diagnostic`, keeps a restarted timer from
repeating it; it clears when the cursor reaches a clean spot, on rename,
and on every `publishDiagnostics`). The same text goes up as a tooltip on
`QEvent::ToolTip` in the viewport filter. **`Qt 6.2` delivers ToolTip
events as a contentless `QShowEvent`** — `QToolTipEvent` only exists from
Qt 6.4, and `QShowEvent::globalPos()` with it — so the position is
`QCursor::pos()` at the event, which is where the mouse has been resting.
A test can therefore synthesize the event: `QTest::mouseMove` to place the
virtual cursor, then `sendEvent(viewport, QEvent(QEvent::ToolTip))`, and
read the tip back as the visible top-level `QLabel` (the tip label is not
`Q_OBJECT`, `qobject_cast<QLabel*>` finds it).

`Alt+,` (`Key_Comma`, plain `AltModifier`) asks `textDocument/references`
with `context.includeDeclaration: true` — the only location request that
needs parameters beyond `textDocument`/`position`, hence the `extra`
argument of `LspClient::requestLocations`. The answer pops a `QMenu`
(`objectName` `clangd-references`, so tests and users of `findChild` can
single it out — note `QMenu(QString, parent)` sets the *title*, not the
name, so `setObjectName` explicitly), with `popup()`, **not** `exec()`: a
nested event loop would stack on the `boundedWait` the key press already
sat in. It deletes itself on `aboutToHide`.

### Renaming a symbol, and applying a workspace edit (issue #125)

`Alt+R` (`Key_R`, plain `AltModifier`) asks the new name in the minibuffer:
`renameSymbol` checks the `identifier_range` under the cursor (reporting "no
symbol under the cursor" off a symbol) and installs a `RenameSymbolInput`
(a `CommandLine` of the mode's anonymous namespace, `objectName`
`rename-symbol`, **empty** prefill so a bare Return cannot rewrite a symbol
with its own name). Three shape facts:

- `CommandLine`'s parent is an **`EditorWidget &`**, not any widget — and the
  input holds the symbol's *position*, captured when the question is raised:
  by the time the answer comes, the cursor of another buffer knows nothing of
  the symbol. That is why the answer arrives at the public
  `CxxMajorMode::renameToAt(name, position)`, not a `renameTo(name)`.
- `LspClient::requestRename` parses **both** answer shapes — `changes` (a map
  uri → edits) and `documentChanges` (`textDocument`+`edits` per entry) — into
  `QMap<file, QVector<LspTextEdit>>`; `LspTextEdit` lives in `LspProtocol.hxx`,
  which `CxxMajorMode.hxx` now includes for the private `applyRename`. The
  client capability is `{"rename", {{"prepareSupport", false}}}`.
- `applyRename` converts every range with `LspProtocol::positionOf` **once**
  and applies a document's edits **sorted by descending start** through one
  `QTextCursor`, so no edit moves the place the next cuts. Files not visited
  are brought in with `editor.openFile` (which reuses a buffer of the file
  anyway); visited ones with `changeBuffer(buffer id)`; the mode's own buffer
  is restored by its id afterwards, since opening a file makes its buffer
  current. A file whose buffer did not come up is skipped, not edited blind.

Test traps, all paid for: a `std::unique_ptr` **cannot be captured by value**
in a helper lambda (`[ed = editor.get()]`); an answered input lingers as a
child until its `deleteLater` runs, so the live question is the one still
**laid out** (`isLaidOut`), and the whole ask-and-answer ritual is retried
against a still-parsing clangd until the document text says the rename went
through; `clangd` refusing a name like `123` is the negative case, and its
error text arrives verbatim in the `error` of the reply — reported, with the
document left alone.

### Signature help, and what the handshake decides (issue #125)

The characters a signature question is worth asking on come from the
**handshake**, not from a list in the mode: `handleInitializeAnswer` reads
`signatureHelpProvider.triggerCharacters` (clangd-15 says `( ) { } < > ,`) and
`watchSignatureHelp` asks on those only. Two protocol details the feature
rests on:

- the client must advertise `signatureInformation.parameterInformation.
  labelOffsetSupport: true` to get parameters as **offsets into the label of
  their signature** (clangd then answers `{"label": [6, 11]}`); without the
  advertisement it sends the parameter text alone and a parameter cannot be
  picked out of the label. `requestSignatureHelp` parses both, `LspSignature`
  and `LspParameterInfo` live in `LspProtocol.hxx`, and the offsets are UTF-16
  code units, which is what the indices of a `QString` are
- `CxxMajorMode::keyPressEvent` runs **before** the typed character is in the
  document: `TextEditBase::eventFilter` on the underlying widget hands the key
  to `handleKeyPressEvent`, and only a false return lets Qt's own handler
  insert it. So the trigger does not ask anything, it starts `signature_timer`
  (120 ms, single-shot, and its own timer: a settling *cursor* asks about a
  word, a typed `(` asks about a call, and the answers belong in different
  places)
- the answer is **asynchronous**, never a `boundedWait`: a bounded wait per
  typed parenthesis is a waiting keyboard. Staleness is position +
  `QTextDocument::revision()` + a `signature_request` generation counter,
  voided along with the tracked words by `resetNavigation`. The automatic path
  is **silent** on an empty answer — braces and template arguments would make
  the minibuffer unreadable — while the key reports "no call around the
  cursor"

`clangd` ranks the candidates for you: at `store(|` every overload is still a
candidate, deep in the third argument only the fitting one comes back, and
`activeParameter` counts the commas. It answers even when the call is broken
(a stray typed comma does not stop it), which is what makes the automatic half
testable by typing a comma — and `QTest::keyClick(widget, Qt::Key_Comma)` with
no modifiers does reach the buffer through the filter, a precondition the test
asserts rather than assumes.

### Quick fixes, and the diagnostics the question must carry (issue #125)

`clangd` answers `textDocument/codeAction` with the fixes of the diagnostics it
is **handed** in `context.diagnostics` — pass an empty context and the answer is
empty even though the file is full of errors (verified by probe; it is the one
request of this integration whose answer is not computed from the document).
`LspClient::requestCodeActions` therefore serializes what `publishDiagnostics`
stored (`LspProtocol::jsonOf`), and the mode asks it with the *same* zero-width
range as the definition question: a zero-width range works, the diagnostics
decide the answer, and the two questions share one 1 s `boundedWait` through a
countdown the two replies decrement (`waiter`), so a late answer costs nothing
and the menu uses what came back. The client capability is
`codeActionLiteralSupport.codeActionKind.valueSet` plus `isPreferredSupport`.

An action is offered only when `edits` is non-empty. The AST tweaks of clangd —
`ExtractVariable`, `add override` — come as `command: "clangd.applyTweak"` with
**no** edit, because the change is computed by a second round trip the server
would answer with `workspace/applyEdit`; offering an entry that writes over the
head of the editor, or that does nothing, is what the filter is for. The
`documentChanges` of a `WorkspaceEdit` are reduced to their plain text edits by
the shared `LspProtocol::editsOf` (which `requestRename` now uses too), and
`applyRename` became `applyWorkspaceEdit`, the one path rename and fixes share.

Menu-shape traps, both verified: entries the mode adds must go through one
`insertAction(before, …)` where `before` is the *first pre-existing* action (the
text edit's own undo/copy/paste entries), so a group added in **reverse** order
reads in forward order and nothing lands below the stock entries; and
`QMenu(QString, parent)` sets the *title*, not the object name. The status tip of
a fix entry is the message of the diagnostic it carries (`LspCodeAction::diagnostic`,
the first of the action's `diagnostics` array), which is also what makes the test
assert the entry is the right fix and not merely *a* fix.

### The overview of a file, and the shape the handshake decides (issue #125)

`Alt+O` (and `Go to symbol...` of the `C/C++` menu) asks `textDocument/documentSymbol`
and puts the answer in the minibuffer question which names the symbol to go to. What
the handshake says decides the answer, and only one shape is worth asking for: with
`documentSymbol: {hierarchicalDocumentSymbolSupport: true}` the server answers a tree
whose entries carry a `detail` — the signature of a method, the kind of a class,
nothing for a namespace — while a client which advertises nothing gets the flat list
of `SymbolInformation`, which names no detail at all and, for the members of a struct
in a file clangd reads without a compilation database, leaves even `containerName`
empty (verified by probe, `/tmp/opencode/docsym/probe4.py`). `documentSymbolsOf` reads
both shapes and flattens the tree depth-first, which is the order the file spells its
symbols in, qualifying each child with the scopes it lives in.

Numbers and places, all verified against `clangd` 15:

- The `SymbolKind` values follow the version of the protocol which inserted `module`
  after `file`: `class` is 5, `method` 6, a static data member 7, `field` 8,
  `function` 12, a global `variable` 13. An enumerator answers 10, the number of its
  own `enum`, and it is the `detail` of that entry which names the enumeration.
  `symbolKindName` is the one place those numbers are spelled; what a test asserts on
  the wire is what that function returns.
- A definition written out of the class it belongs to is a *separate top-level entry*,
  named by the qualified name the file spells it with. The in-class declaration is a
  child of its class and is qualified by the flattening, so the two entries of
  `Widget::paint` — the prototype and the body — are exact answers to the same name.
  The rule is the first of the document, and the minibuffer says which one was taken.
- Only the start of the `selectionRange` is kept: the `range` of a class begins at its
  `class` keyword and would land the cursor a line above the name. Nesting depth is
  not kept either, see the display below.

The surface is the minibuffer question of a file name (`EditorWidget::ChangeBuffer`),
not a popup menu, and three shape facts of that machinery are worth knowing before
touching it:

- The list is the `*completions*` secondary task, whose entries go through
  `QString::toHtmlEscaped()` in a two-column HTML table: a signature full of `<` and
  `>` is safe, runs of spaces collapse, and a character reference written by the
  caller — `&#160;` — shows up as its own name. That is why an entry reads
  `paint — void (int)` with a real dash rather than a padded column.
- `showCompletions` connects `textChanged` to `hideCompletions`, so a list which is to
  follow what is typed has to be shown again *after* the base class emitted the signal:
  that is `DocumentSymbolInput::inputTextChanged`, and `narrowed` filters the whole
  overview on a case-insensitive substring, since `paint` is what one types for
  `Widget::paint` and a prefix match would answer nothing to it.
- The `QCompleter` of a minibuffer question is *never installed on the widget*
  (`setCompleter(c, false)`): completion runs through `CustomLineEdit::complete()` on
  `Tab`, which walks the rows of the completer's model itself. Whatever
  `completionMode` says, no popup appears while typing, and the list of the secondary
  task is the only visible completion.
- The title a secondary task is registered under is cut to 8 characters, so the tab of
  the overview reads `*complet`: a test asserting on `getSecondaryTaskTitle` has to
  assert the prefix.

The question is asked before it is put — `LspClient::requestDocumentSymbols` takes no
position, its answer is the file — and the question holds the overview it was built
with, so answering never asks the server again. Its `objectName` is `go-to-symbol`,
which is how a test singles it out among the inputs the minibuffer has held, and an
empty answer cancels as it does for a rename. `testDocumentSymbols` asserts the tree
reached the screen by what only the tree holds: the qualified name of the out-of-line
definition and the `void (int)` beside it. The two shapes of answer are covered
without a server by `testDocumentSymbolsOf` of `test-lsp-protocol`.

### The minibuffer shows the start of a message (`MultiLineLineEdit::setText`)

The message area `um` is a read-only `LineEdit`, i.e. a `QTextEdit` shim, and the
view follows the text cursor: `setText` putting the cursor at the end (the
`QLineEdit` emulation an editable input needs) scrolled long messages to their
tail, and the visible line of `using <path> to start clangd` was just `clangd`.
A read-only shim now puts the cursor at the start. When a message reads as
"only the last word", think view-scrolls-to-cursor before think text-content.

### What the identifiers of a file are, and the four faces that say so (issue #125)

`textDocument/semanticTokens/full` asks the server what the names in the file
*are*, which is the one thing said about a name that no regular expression of a
highlighter can tell. `clangd` 15 answers it, and its provider says
`{full: {delta: true}, range: false, legend: …}` — **a `range` request is
refused**, so the only shape of the question is the whole file, and the answer
is the whole file's identifiers every time (`full/delta`, which would answer only
what changed, is not asked).

The legend, all of it read off the handshake of a session whose capabilities are
exactly what `LspClient::launch` sends (`/tmp/opencode/sectok/probe2.py`):

- The legend is the **server's own**, in its own order: `clangd` answers
  `variable, variable, parameter, function, method, function, property, variable,
  class, interface, enum, enumMember, type, type, unknown, namespace,
  typeParameter, concept, type, macro, comment`, with the modifiers
  `declaration, deprecated, deduced, readonly, static, abstract, virtual,
  dependentName, defaultLibrary, usedAsMutableReference, functionScope,
  classScope, fileScope, globalScope`. Three kinds are named `variable` and three
  are named `type`, so the kind of a token is its **index**, and whoever maps
  kinds to faces keys on the *word* — which loses that distinction, and does so
  on purpose: a restrained face set wants `variable` to be one face.
- No kind names a keyword, a string, a number or a comment. The hand-written
  `CxxSyntaxHighlighter` therefore stays the owner of those words whatever the
  server says; semantic tokens color identifiers and nothing else.
- `auto` is answered as the *type it is deduced to*, kind `type` or `enum`, with
  the `deduced` modifier. Coloring that word would ink over the keyword, so
  tokens carrying `deduced` are skipped.
- The answer is a flat array of five integers per token — `deltaLine,
  deltaCharacter, length, kind, modifiers` — where the line and the place are
  written as what they differ from the previous token, **except that on a new
  line the place is the place itself**, never a difference: `semanticTokensOf`
  reads `delta_line != 0 ? delta_character : character + delta_character`, and a
  well-formed server never sends a negative place. A `result` which is `null` or
  names no `data` is a file the server resolved nothing about; the integers of a
  repeat call over unchanged text are identical (`resultId` needs no bookkeeping
  for this editor); the tail of an answer whose length is not a multiple of five
  is dropped; a kind number or a modifier bit the legend does not name names
  nothing; and the modifier loop stops at bit 31, because `1 << 31` is not
  something to write.
- Faces are drawn as a keyed contribution to the extra selections,
  `setExtraSelections("clangd-tokens", …)` — the keys in use are `current-line`,
  `clangd`, `clangd-navigation` and this one — so no highlighter code changed and
  a stopped session removes the contribution whole. `semanticFaceOf` knows four:
  dark cyan for `function`/`method`, dark magenta (not bold: that face is the
  highlighter's keywords) for `macro`, dark blue for `class`, `struct`, `enum`,
  `type`, `typeParameter`, `concept`, and a strike-out for the `deprecated`
  modifier, whatever else the name is. `variable`, `parameter`, `property`,
  `namespace`, `enumMember` and `unknown` are asked about and left alone.

Numbers and traps, all verified:

- `Qt 6.2` **inks** a strike-out and a foreground through the plain text edit's
  painting path, even though it does not ink a wave underline style: a pixel
  grab of the same six characters with and without the face differs by 43 pixels
  for `setFontStrikeOut`, 221 for a foreground, 15 for bold
  (`/tmp/opencode/sectok/strike.cxx`, `DISPLAY=:99`).
- `LspProtocol::positionOf(document, line, character)` answers the **last block**
  for a line the document does not have, so a stale answer would paint the last
  line of the file: check `document.findBlockByNumber(line).isValid()` first. Both
  ends of a token are asked for through `positionOf`, which clamps a character to
  its own line, so an answer about a text which moved a little cannot paint across
  a line break.
- Staleness is guarded twice, by the `QTextDocument::revision()` the question was
  asked at and by a `tokens_request` counter which `clearSemanticTokens` increments
  so that a request on its way is voided along with the faces it would have drawn.
- Three settling timers live in the mode now: 150 ms for the tracked words, 120 ms
  for a signature, **700 ms** for the tokens of the file — the question is about the
  whole file and its answer is the color of the buffer, not something anybody typed
  is waiting for. It is restarted by `QTextDocument::contentsChanged`, by
  `LspClient::ready`, by `openOrRenameDocument`, and by `updateDiagnostics`, which
  is the cue that `clangd` has finished parsing the file and can answer about it.
- The setting `clangd/semantic-tokens` (true by default) is read **on every
  question**, not at the launch of a session, because the colors of a buffer are
  the kind of thing a person turns off the first time they dislike them and expects
  the text to change back under their hands. The checkable `Color identifiers` entry
  of the `C/C++` menu writes that setting and repaints the buffer of the file on
  screen at once; the other buffers follow as they settle.
- Two spelling notes: the prose of the tree is American (`color`, `colors`; the one
  British occurrence is a pre-existing comment in `DirectoryMajorMode.hxx`), and the
  menu entry is `Color identifiers`, so a test which asserts on the text of the entry
  has to spell it that way.
- The `C/C++` menu has seven entries: the two launches, a separator, `Go to symbol...`,
  `Color identifiers`, a separator, `Stop clangd` — which moved `Stop clangd` from
  index 5 to 6 of what `testModeMenu` walks.

Three things worth remembering:

- **The `stop-clangd` command was never registered.** `stopClangd` existed, the menu
  entry worked, the comment above the function and this page both named the command,
  and `StandardFunctionCommandProxy<stopClangd>` was simply absent, so
  `launchCommand("stop-clangd")` displayed "unknown command" and stopped nothing. The
  assertions which were supposed to catch it passed: they ran after the menu entry had
  already stopped the session, so `isIdle()` was true whatever the command did. A test
  of a command which ends something has to start that thing again first — `testModeMenu`
  now launches, then stops through the command, then asserts.
- **A GCC false positive is still a new warning.** A lambda whose guard dereferences a
  `QPointer` inside the condition which checks it for null —
  `if ((self == nullptr) || (sequence != self->signature_request))` — draws
  `-Wnull-dereference` twice, in `_M_invoke`, and the tree has no `-Werror`, so it would
  have gone unnoticed in the build log. What silences it, and reads no worse: take the
  pointer out first (`auto *const mode = self.data();`), return on null in its own
  statement, and use `mode->` everywhere after.
- A face is visible to a test through the public `TextEditBase::extraSelections()` — the
  highlighter's faces are not, see "Auditing a mode without a test suite" — so
  `testSemanticTokens` asserts on a rendered list of `word:#color|strike` entries and
  reports that list when an assertion fails, which is what names the survivor of a stop
  rather than only saying that something stayed painted. `initTestCase` removes
  `clangd/semantic-tokens` from the test settings and the test restores it to true at the
  end: a run which ended early would leave the colors off for every later test.

### A regenerated `compile_commands.json` reaches the parse, not the index (issue #125)

Nothing in the editor watches the database, and nothing needs to for the file
being edited: `clangd` 15 answers `initialize` with
`"compilationDatabase":{"automaticReload":true}` and re-reads the file itself.
Verified against a live session, all of it in `/tmp/opencode/cdb`:

- The reload goes **with a parse, not with the clock**: `Reloaded compilation
  database from <path>` appears in the `--log=verbose` output 2–5 s after the
  rewrite and only when the server next needs a command line. Thirty seconds of
  idle time produce no reload at all (`probe14.py`), and a parse 0.1, 0.3, 0.5,
  0.8, 1.2 or 2.0 s after the rewrite still used the old command while the one at
  3.0 s used the new (`probe7.py`). Nothing is ever reparsed on its own either, so
  the underlines of a buffer say what the old options made of it until something
  is typed there.
- Both directions work: a `-DEXTRA_FLAG` added to the command of an open file
  makes its published error go away, taken away again it comes back
  (`probe2.py`).
- `workspace/didChangeWatchedFiles` naming the database changes nothing — no
  reload, no reparse — and advertising `didChangeWatchedFiles` with
  `dynamicRegistration` in the handshake brings no `client/registerCapability`
  asking to register it. There is nothing on the client side to implement here.
- Neither the cwd of the process nor the layout matters: the reload works with
  the database beside the sources, with the process started in the project, and
  with the database in a `build` directory named by `--compile-commands-dir` while
  the sources sit one level above and the process runs in a third directory
  (`probe16.py`, `probe17.py`) — the last of which is what a real session looks
  like, since `LspTransport::start` never calls `setWorkingDirectory`.
- **The index does not follow.** On a reload the server logs `Enqueueing N
  commands for indexing` and then rebuilds only what it has not indexed in that
  session: a file whose new flag unveils a symbol stayed out of
  `workspace/symbol` for 42 s with unchanged contents (`probe5.py`) and for 60 s of
  polling when it was rewritten on disk as well (`probe14.py`). A file *newly
  added* to the database is the exception — indexed 2 ms after the reload, and
  `textDocument/definition` answered inside it 7 s after the regeneration
  (`probe13.py`).
- A **relaunch** does not clear the index either, because a persisted shard is
  keyed on the *contents* of the file, not on the command which compiled it: one
  file, compiled once with `-DEXTRA_FLAG` and once without, is written to the same
  `a.cc.D88B3FE2F6FD8463.idx`, and the session started on the new database never
  logs `Indexed a.cc` (`probe15.py`). When the contents differ from the shard, or
  when the cache is gone, it indexes within seconds (`probe12.py`, `probe11.py`).
  The shards live under the directory `--compile-commands-dir` names —
  `build/.cache/clangd/index` for the layout of `probe17.py`, and never under the
  cwd of the process.

Three traps, all paid for:

- A probe must remove the cache where clangd wrote it, which is next to the
  database, not under the cwd of the process. Removing only the probe's work
  directory leaves the shards in place and the case then reads as "relaunching
  does not help either": `probe12.py`'s `clean` case is wrong for exactly this
  reason, `probe11.py`'s `3b`, which removes both, is the control.
- Short-lived sessions are no evidence about the index. Since the reload waits for
  a parse and a parse soon after the rewrite still reads the old command, a session
  which rewrites the database, types once and then only polls `workspace/symbol`
  shows no `Reloaded` line at all and says nothing. Claims about the index belong to
  runs whose log names the reload.
- `workspace/symbol` is what shows what the index knows about a file nobody opened.
  `textDocument/definition` is not: on a call to a function whose body is unknown it
  answers the *declaration* in the current file, a jump which looks like a success.
  Assert on the file the answer names.

So the sentence the docs carry — the database a session reads is chosen when it
launches — is about the directory and about the index. The options a file is
compiled with are not frozen for the session, which is why no `QFileSystemWatcher`
restarting clangd was added: it would buy the index of unchanged files nothing (the
shards survive the restart) and would throw away a session whose parse is already
current. `docs/web/clangd-integration.md` keeps the same conclusion, and the one
part of the idea that would still pay — watching the file to *report* a stale
index, not to fix it.

### The configuration dialog: entries are declared, never discovered (issue #127)

`Options > Configuration` and `configure-tfel-editor` open a dialog built from
`SettingsRegistry`, and what it shows is what the code **declared**, not what
`QSettings` holds: the settings file answers the addresses that were *written to*,
which says nothing about the addresses that are *read*, and a dialog built on it
would promote a typo in the file to an entry of the editor. A declaration is a
`SettingsKey` (address, label, description, type, default, choices, min/max) and
sits next to the code that reads it; the machinery is the familiar one — a
`SettingsRegistry` singleton and `StandardSettingsGroupProxy` static initialisers,
like `CommandFactory` and `MajorModeFactory`.

**What is shown is what one chooses, never what the editor remembers.** Every
`.../history` address (the commands of `Alt+X`, `Alt+!`, `grep`, `Alt+%`, the
compilation command `Alt+M` proposes per mode), the `recent files` list, and the
`.../last-interface` addresses of the `MFront` mode are deliberately **not**
entries: a `CommandLine` is given an address and writes it back when its answer is
taken, so such a field shows a stale list and is overwritten by the next prompt on
its own account. A test asserts those addresses are not declared
(`theRegistryDeclaresTheEntriesOfTheEditor`), which is what keeps the next person
from putting them back. The themes that remain are `General` and `Spell checking`
from the core, `clangd` from the C++ mode and `castem` from the Cast3M mode; no
other packaged mode exports anything today, because the only entry of the
configuration those modes read is a history.

How a mode exports its entries, since that is the part another mode has to imitate:

- `MajorModeProxy::getSettingsGroups()` is virtual and answers nothing by default;
  `StandardMajorModeProxy<T>` fills it through `if constexpr (requires {
  T::declareSettings(); })`, so a mode opts in by declaring a static
  `QVector<SettingsGroup> declareSettings()` and needs nothing else. The proxy fills
  the name, description and icon of a group which names none of its own from the
  mode; a mode whose entries are about a program it drives names the group itself,
  which is why the C++ mode answers a theme called `clangd`.
- An entry with an empty `group` belongs to the theme it is declared in;
  `SettingsKey::inGroup` is how an entry says otherwise.
- `MajorModeFactory::getMajorModeSettingsGroups(name)` is what feeds
  `SettingsRegistry::getGroups()`: core themes first, then the modes' sorted
  case-insensitively. A mode which declares nothing has no theme — a group emptied
  by the merge is dropped, not shown as an empty page.
- The merge (`appendGroup`) keeps the **first** declaration of an address and
  reports it only when the two declarations *disagree*: `agree()` compares
  label/description/type/default/min/max/choices but **not** `group`, because
  `MTest` and `PipeTest` inherit `CxxMajorMode::declareSettings` and asking twice
  about the same four `clangd` addresses is exactly right, not a clash. Without
  that, every session printed eight lines of `qDebug` noise about correct code.

Behaviour worth not re-litigating: `SettingsKey::write()` **removes** the address
when the value equals the default, so the file holds only what was changed;
`Restore Defaults` only sets widgets, it writes nothing until the dialog is
accepted; and the three entries whose effect must be seen under the hands —
`font/textedit` (`EditorWidget::changeFont`), `shortcut style`
(`ShortCutStyle::setStyle`), `hunspell default dictionary` (per-buffer
`setSpellCheckLanguage`) — are applied by `accept()`, everything else waits for the
widget that reads it to be rebuilt.

Qt 6.2 traps, all paid for:

- There is **no `QFontButton`** in the widget set shipped here; the dialog uses an
  anonymous-namespace `FontSelector : QPushButton` which shows the font's own name
  and text and drives a `QFontDialog`.
- `QDialogButtonBox` has the `RestoreDefaults` role but no `reset()` slot to lean
  on: take `bb->button(QDialogButtonBox::RestoreDefaults)` and connect
  `QPushButton::clicked`.
- `QMetaType::QStringList` is *the enum value*, not a type: comparing a
  `QVariant`'s kind needs `v.metaType().id()`, which is what `kindOf()` does.
- `QFormLayout` sizes its label column to the **longest label of the theme**, so on a page
  whose longest name is 265 px wide the field of a program's path is left 187 px. The dialog
  therefore uses `form->setRowWrapPolicy(QFormLayout::WrapAllRows)` — label above, field the
  width of the page. `QFormLayout` has **no `setFormStyle`** in Qt 6.2 (the style is a
  constructor argument only), but the row wrap policy and the field growth policy do have
  setters. Verified with the GUI probe (`castem 458 x 22` where the two-column form gave 274,
  clangd 187) and a screenshot of each page on Xvfb + metacity.
- The INI format writes a `QStringList` as comma-separated text, so a stored value
  **containing a comma** reads back as a list and a list of **one** entry reads back
  as a plain string. `toText`/`toStringListOf` accept both readings; a test asserts
  the round trip of a one-entry list, through `clangd/arguments`, which is the one
  list the dialog edits.

Test traps:

- `for (const auto& k : QStringList() << "a" << "b")` is a **use-after-scope**:
  `operator<<` answers a reference to the temporary, which a range-for does not
  extend the lifetime of. Valgrind blames the test's own frame and says nothing
  useful; compiling the single `.cxx` by hand with `-fsanitize=address -fPIC
  -DNDEBUG -DTFEL_ARCH64` and the build's own `-I`/`-isystem` flags against
  `libTFELEditor` and `Qt6Test` names the line in one run. Bind the list to a named
  `const auto` first.
- Isolation: `initTestCase` points `QSettings` at `tfel-editor-tests`/`test-settings`
  and every case starts with `forgetEverything()`, since the assertions "an entry
  nobody changed is not in the file" are otherwise answered by the previous case.
- Only the **C++** mode is registered in-process: the packaged modes are plugins, so
  what the plugins do to the registry — exporting the entries they inherit, and
  declaring nothing at all — is checked by loading the built `.so` files into a
  probe that prints the registry (`/tmp/opencode/settings127/probe.cxx`). That probe
  is what found the two merge bugs above.
- In a dialog probe, only the **visible** page of the `QStackedWidget` has a real
  geometry; hidden pages are still `100 x 30`, which is what a widget of a hidden
  window is left with (`/tmp/opencode/settings127/gui.cxx` prints the visible page
  only, and `import -window root` is what catches a page that does not fit).

### The Cast3M mode launches the program the configuration names (issue #127)

`castem/executable` decides which `castem` the mode starts, and
`CastemMajorMode::declareSettings` names the theme `castem` after that program — as the C++
mode does for `clangd` — so no page appears under `Cast3M`. The address is read at every launch
(`castemExecutable()`, anonymous namespace of the mode), and both launch sites use it: the
session of `startCastem` and the notice of `displayHelp`, which used to run a hard-coded release
while the session could be told otherwise. The default of the entry is the name the mode ran
before the entry existed (`castem2018_PLEIADES`, `castem17.bat` on Windows) and is declared with
it, so the field shows the program the mode would run and an answer equal to that name is removed
from the file rather than stored.

Numbers about `QProcess` on this Qt 6.2.4 (`/tmp/opencode/castem131/probe3.cxx`) — they are what
the failure path of `startCastem` rests on:

- a program which does not exist is **not** reported inside `start()`: the call returns with
  `state() == Starting` and `error() == UnknownError`, and `FailedToStart` arrives about 10 ms
  later through the event loop. That is why the `errorOccurred` handler is connected before the
  start and runs *after* `attachSecondaryTask` — a handler firing inside `start()` would leave
  the very next statement attaching a frame the handler had already detached and nulled.
- a program which does not exist is reported as `FailedToStart`, which is the only error
  `startCastem` reports. What an earlier run of the probe read as `Crashed` for the same name was
  the artifact of `kill()`ing a half-alive process: the control case (`/bin/sh`) reports nothing
  at all while it lives.
- because the state right after a start is `Starting` and not `Running`, the first
  `sendToCastem` writes nothing (it asks for `Running`) and `sendLineToCastem` answers false: the
  first line is dropped. Pre-existing and unrelated to the entry, but worth knowing before
  "fixing" the return value of the slot.
- the minibuffer names the program which did not start (`"%1 could not be started"`), which is
  the whole report of a wrong path now that the path is what the user wrote.

Driving a packaged mode although its class is not exported: the mode is built by name and its
slot is invoked by name, which leaves the launched program visible through the exported frame.

~~~~{.cxx}
auto *const mode = b.getMainFrame().setMajorMode("Cast3M");
auto started = false;
QMetaObject::invokeMethod(mode, "sendLineToCastem", Q_RETURN_ARG(bool, started));
auto *const f = qobject_cast<ProcessOutputFrame*>(b.getCurrentSecondaryTask());
f->getProcess().program();  // the program the mode launched
~~~~

`/tmp/opencode/castem131/launch.cxx` does exactly that with the built `.so` and reports
`castem2018_PLEIADES` when nothing is stored, `/bin/cat` in state `Running` when the entry names
it, and `/opt/castem/17/bin/castem` — with that same name in the minibuffer a few milliseconds
later — when the entry names a program which is not there.

Two test traps of the case which loads a plugin
(`aMajorModeLoadedAsAPluginExportsTheEntriesItUses`):

- `dlopen` of a build-tree plugin needs nothing from `LD_LIBRARY_PATH`: the RUNPATH of
  `libTFELEditorCastemMode.so` holds the absolute path of `libTFELEditorMFrontImportWidgets.so`,
  of `libTFELEditor.so` and of the TFEL libraries. The test therefore asks CMake for the file
  (`$<TARGET_FILE:TFELEditorCastemMode>` in a `target_compile_definitions`) and makes sure it is
  built (`add_dependencies`); without the macro the case `QSKIP`s.
- such a case must come **last** in the list of slots: a loaded mode stays registered for the
  rest of the run, so from then on `SettingsRegistry::getGroups()` answers a `castem` theme and
  every earlier assertion about the set of themes would disagree with it.

### The configuration dialog keeps the themes of the modes apart (issue #127)

`SettingsGroup::mode` — the name of the major mode which exported the theme, empty
for a theme of the editor — is what tells the two kinds of themes apart, because a
theme named after a program it drives (`clangd`, `castem`) says nothing of the kind
by its name. `StandardMajorModeProxy` fills it as it fills the name, description and
icon of a theme which names none of its own, and `appendGroup` keeps the first
attribution, as it keeps the name. `ConfigurationDialog` splits `getGroups()` on that
field and puts a non-selectable title (`Qt::NoItemFlags`, bold, the grey of disabled
text) over each block, with an empty row between them, and a bold caption naming the
mode at the top of every page of a mode theme (`objectName`
`configuration-page-mode`, which is how a test singles it out).

Four traps, all paid for:

- A `QListWidgetItem` is **not a `QObject`**: no `setObjectName`, and a test cannot
  single a row out by name. What a row means is kept in the item itself,
  `setData(Qt::UserRole, page)`, and `QListWidgetItem::data` of an **unset** role
  answers an invalid variant whose `toInt()` is `0` — a real page. Every item,
  headings and the gap included, has to carry the role explicitly (`-1` for what
  shows nothing), and `toInt()` of it is then meaningless without that guard.
- The old `connect(themes, currentRowChanged, pages, setCurrentIndex)` cannot be
  kept once rows exist which name no page: `currentRowChanged` fires for a
  non-selectable row too. The slot is a lambda which sets the page only when the
  role is not negative, and a test asserts that choosing a title leaves
  `currentIndex` alone.
- `QLabel` has **no `setForeground`** (that is `QListWidgetItem`): the grey of a
  caption is a `QPalette` of the label itself.
- Adding a member to `SettingsGroup` is an **ABI break for the packaged modes**: a
  probe which loaded a `castem` plugin built against the old layout into a core
  library built with the new one segfaulted in the reallocation of a
  `QVector<SettingsKey>` inside `appendGroup`, three frames away from anything that
  looks like the cause. `cmake --build . --target TFELEditor` is enough to see a
  change of the dialog; it is not enough to run a probe against the plugins.

`TestSettings` asserts the shape with `theThemesOfTheMajorModesAreSeparatedFromThose
OfTheEditor` (the titles are the non-selectable rows and show no page, every theme is
on its own side of the titles, a title changes no page, a caption names the mode and
only mode pages have one), and `theDialogShowsWhatIsDeclared` reads the themes as the
**selectable** rows — `getGroupsNames()` and `pages->count()` both disagree with
`themes->count()` now.

## Architecture

- `include/TFEL/GUI/` — public headers of the core library. `.hxx` are
  declarations, `.ixx` hold inline/template bodies and are `#include`d at the
  **bottom** of the matching `.hxx`. Everything is in namespace `tfel::gui`,
  and intra-project includes always use the `TFEL/GUI/…` prefix.
- `src/` — the `TFELEditor` shared library and `tfel-editor.cxx` (the
  executable entrypoint).
- `packages/<lang>/` — one plugin per language mode (`bash`, `castem`, `cmake`,
  `gnuplot`, `latex`, `licos`, `make`, `markdown`, `mfront`, `mtest`, `python`,
  `sh`, and `tplot` when enabled).
- `utils/mfront/` — the `MFrontImportWidgets` plugin (the "import MFM
  behaviour" wizard). It is *not* a language mode and is linked by the `mtest`
  and `castem` modes, not by `mfront`.
- `packages/*/include/` headers also install to `include/TFEL/GUI/`, so header
  names must be **globally unique across core and all packages**.

### Adding a Qt class with `Q_OBJECT` — the main trap

`CMAKE_AUTOMOC` is **off**. Each `CMakeLists.txt` uses the legacy
`qt_sources(<VAR> <header-dir> ClassName …)` macro from `cmake/modules/qt.cmake`,
which runs `QT6_GENERATE_MOC` on `<header-dir>/ClassName.hxx` and prepends
`ClassName.cxx` to the source list. Consequences:

- A `Q_OBJECT` class must be listed by name in the `qt_sources` call of the
  `CMakeLists.txt` that owns it, or linking fails with an undefined vtable.
- The class needs *both* `<header-dir>/ClassName.hxx` and `<source-dir>/ClassName.cxx`
  with exactly that name; the `.cxx` is not in `<header-dir>`.

### Headers and exports

- Every new header (and `.ixx`) must be added to the owning
  `include/CMakeLists.txt` via `install_header(TFEL/GUI Foo.hxx)`. The build
  works without it (it compiles from the source tree) but `make install`
  silently ships an incomplete SDK.
- The build uses `-fvisibility=hidden`. Any class in `TFELEditor` that a
  package links against must be declared
  `struct TFEL_GUI_VISIBILITY_EXPORT …` (macro from `TFEL/GUI/Config.hxx`),
  otherwise the plugin fails to load at runtime.

### Adding a language mode

1. `add_subdirectory(<name>)` in `packages/CMakeLists.txt`.
2. `tfel_editor_package(<Name> …)` in `packages/<name>/src/CMakeLists.txt`
   creates target `TFELEditor<Name>` and installs it to
   `share/tfel-editor/packages`.
3. Registration is a **static initialiser** at the bottom of the mode's `.cxx`,
   not a factory call — see `packages/markdown/src/MarkdownMajorMode.cxx`:
   `static StandardMajorModeProxy<MarkdownMajorMode> proxy("Markdown", <file-name regexes>, <icon>);`

### Running without installing

`src/tfel-editor.cxx` scans `<prefix>/share/tfel-editor/packages` (and
`$TFEL_EDITOR_LOAD_PATH`, colon-separated, **non-recursive**) for `.so` files
and loads every one it finds. To run from a build tree, list each
`packages/*/src` and `utils/mfront/src` dir separately. `install` strips RPATH,
so `LD_LIBRARY_PATH` must contain both `<prefix>/lib` and
`<prefix>/share/tfel-editor/packages` (see `install.sh`).

Modes also read runtime env vars for their example galleries: `MFMHOME`,
`MFRONTGALLERYHOME`, `LICOS_USER_FILES`, `LICOSHOME`, `MFRONTMATERIALSHOME`,
`LICOSSTUDIESHOME`.

## Conventions that differ from defaults

- **Do not re-sort includes.** `.clang-format` sets `SortIncludes: false`, so
  an auto-formatter that reorders includes produces spurious diffs across the
  whole tree.
- Formatting: Google-based, `ColumnLimit: 80`, `NamespaceIndentation: All`,
  `BinPackParameters: false`. Run `clang-format` on changed files only —
  `clang-format.sh` reformats every `*xx` under the cwd, including the vendored
  `include/TFEL/GUI/utf8/` copy.
- `docs/` is the GitHub Pages source (jekyll `tactile` theme); `.md` files link
  to `.html` outputs and use pandoc fenced blocks (```` ~~~~{.bash} ````).
- `QWebEngineViewWrapper.cxx` is dead code: the `WebEngine` `find_package` is
  commented out in `cmake/modules/qt.cmake`, so `WebEngine_FOUND` is always
  false and the `if(WebEngine_FOUND …)` block in `src/CMakeLists.txt` never
  compiles.

## Branching

`master` tracks `TFEL` `master`; `rliv-1.1`/`rliv-1.0` track the TFEL 3.3.x/3.2.x
series (see `README.md`). Issue branches (e.g.
`9-audit-the-code-improve-documentation-generate-website`) are cut from
`master` and may be identical to it.
