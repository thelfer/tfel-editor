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
- **A sanitizer which reaches only the test executable proves nothing.**
  `compiler.cmake` sets `CMAKE_CXX_FLAGS` from `${COMPILER_WARNINGS} $ENV{CXXFLAGS}`
  when `-DUSE_EXTERNAL_COMPILER_FLAGS=ON` is given and from its own lists otherwise, so
  a `-DCMAKE_CXX_FLAGS=-fsanitize=address` on the command line is **discarded**: the
  "ASAN" library is an ordinary build and only the test executable links `libasan`,
  which makes every clean run worthless. What instruments the library is
  `export CXXFLAGS="-fsanitize=address -g -O1 -fno-omit-frame-pointer"` with that option
  on, and what verifies it is
  `grep -m1 CXX_FLAGS src/CMakeFiles/TFELEditor.dir/flags.make | grep fsanitize`, before
  believing a report. (What this cost: an intermittent `malloc(): unsorted double linked
  list corrupted` in `test-grammar-check`, read as a use-after-free in the session
  teardown; a genuinely instrumented tree named it on the first run as a dangling
  `QStringList() << a << b` temporary in the test's own range-`for`, which is a
  use-after-scope ASAN reports by name.)
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
### The buffer watches the file it visits (cross modifications)

`TextEditBase` keeps the state of the visited file on disk — modification
**time and size**, a `FileOnDiskState` — as of the last moment the buffer
read or wrote it, recorded by `noteFileStateOnDisk` at exactly three
moments: `setFileName` when the name *changes*, a `save` which reached the
disk, and `reloadFile`. `fileModifiedOnDisk()` is the one question
(`QFileInfo` exists and is a file, and time or size differ); the file
*vanishing* is deliberately not a modification, and a buffer writing under
a new name takes that file's state as its reference, which is what keeps
the `WriteFile`/`OverWriteFile` flows — they reach an existing file without
reading it — from reporting the difference with the file left behind as an
external change.

Detection is a poll, not a watcher: `Buffer`'s own 1 s timer (the one that
refreshes the date) calls `e->checkFileModifiedOnDisk()`, which shows the
banner `Buffer::showFileModifiedBanner` unless the file sits in the state
the user dismissed (`dismissedFileStateOnDisk`, written by
`acknowledgeFileModifiedOnDisk`, the ✕ button). `QFileSystemWatcher` buys
nothing here: this editor does not need to react in less than a second, a
watcher of a *saved* file would fire on the editor's own writes, and files
replaced through a temporary file (the way many programs write) drop their
watch — the mtime+size pair catches exactly those rewrites.

While the banner is up, the buffer is **frozen**: `showFileModifiedBanner`
sets the main frame read-only and `hideFileModifiedBanner` hands it back
(the freeze flag and the one holding the state captured before the freeze
are declared with the banner pointers *above* `e`, because
`hideFileModifiedBanner` runs already from the main frame's constructor,
where the flag answers `false` and the frame is not touched). The freeze
lasts exactly as long as the banner, so each of the ways out — reload,
dismissal, or the save which overwrote the file and hides the banner —
unfreezes with it; `save` itself is not frozen, because a frozen buffer
must still be able to reach the overwrite question the banner waits for.
The `Ctrl-/`/`Ctrl-U` undo binding checks `isReadOnly()` too — undo walks
the document back as surely as a typed key writes into it, and it used to
reach `undo()` ungated (there is no keyboard binding for redo).

`isReadOnly` is the *existing* gate of the editing commands and of the
self-insert of keys, and the arrows / the mark-based selection already
worked in read-only buffers (compilation output, directories); freezing
therefore costs no navigation — except for the keys listed in the trap
below.

The traps of the freeze, in addition to those of the banner:

- **A read-only `QPlainTextEdit` ignores even forwarded navigation keys.**
  It has no text cursor of its own and its `keyPressEvent` returns before
  reaching the cursor movements, so the *Compilation Output*-style
  exception which forwards `PageUp`/`PageDown`/`Home`/`End` to the widget
  moved nothing while frozen (verified: `End` left the cursor at 4, with
  the fix it reaches 7 — while the arrows, handled through `moveCursor`
  before the gate, had already kept working). The exception now clears
  the read-only state around the single forwarded event and restores it
  after, which is safe because a navigation key can not mutate the
  document — all the read-only state guards.
- A test of that gate sends its keys with `QTest::keyClick(widget, …)`;
  through the build system this is uneventful, but a throwaway probe
  compiled by hand must add `-DQT_GUI_LIB -DQT_WIDGETS_LIB`, or the
  `QWidget` overloads of `keyClick` — which are guarded on those macros —
  are simply not declared.

The two confirmation paths of a save are a deliberate split, not an
oversight: both ask the **same question — overwrite ?** — through the
confirmation mechanism each can afford. Menus go through
`saveWithDialogConfirmation` — set `confirmSaveWithDialog`, call `save`,
reset — which answers a conflict with a modal `QMessageBox` (Save /
Cancel), while `save` itself, the keyboard path (`Ctrl-X Ctrl-S`, and
`Ctrl-C Ctrl-C`, newly bound in the `ctrlc` branch *after*
`MajorMode::handleShortCut`, which no mode claims), installs the blocking
`FileModifiedOnDiskInput` (objectName `file-modified-on-disk`): `y` sets
the one-shot `overwriteFileModifiedOnDisk` and saves again — `save` skips
the conflict branch when it is set and spends it at once, which is what
keeps the confirmation from asking twice —, `n` or `Ctrl-G` saves nothing
and leaves the banner up. Re-adopting the file is the banner's **Reload**
button's office, never the question's: `reloadFile` reads with the same
`QTextStream` setup as the `PlainTextEdit` file constructor, restores the
cursor's *line* through `gotoLine` (which stops at the last line when the
document shrank), resets the modified flag and hides the banner.

Four traps, all paid for:

- **`Buffer`'s member declaration order is load-bearing.** A buffer
  visiting a file builds its `PlainTextEdit` in the *member initializer
  list*, and that constructor reaches `hideFileModifiedBanner` through
  `setFileName` — before `initialize` has run. A banner pointer declared
  after `e` is then read before its default member initializer, which is a
  garbage pointer: `EditorWidget::openFile` segfaulted on the first
  offscreen test run until the two banner pointers were declared *above*
  `e`. `hideFileModifiedBanner` null-checks for the other half of the same
  problem (it also runs before `initialize` created the widgets); the
  declaration order is what makes the check answer "not built yet" rather
  than "crash".
- **A `Preferred` vertical policy lets the layout grow the banner**: with
  one, the strip ate the top half of the buffer (a live-display screenshot
  says it; offscreen geometry hides it). `QSizePolicy::Fixed` keeps it a
  one-line strip over the text.
- **The banner wears the editor's colors, not gedit's**: it sets **no style
  sheet** — a plain `QWidget` child of `Buffer` paints no background, so the
  buffer's own background shows through, which is exactly the surface the
  information line is drawn on — and its label takes the mode line's font
  through `setQLabelFontSizeAndMargins` at construction and through
  `Buffer::changeFont` afterwards. A `background-color`/`color` stylesheet
  here would disconnect it from the theme and from the font dialog.
- **A blocking input can not hand the minibuffer to another input** —
  `setUserInput` refuses with "command attempted to use minibuffer while
  in minibuffer" when the topmost input blocks. So
  `SaveInput::treatUserInput` (`getSaveInput`, the save-before-kill
  question the modes install) now calls `editor.removeUserInput(this)`
  *before* `save`, following the `OverWriteFile` pattern: a save answering
  that question can uncover a file modified on disk and has to be able to
  ask about it.
- The check draws through the buffer, but only the **main frame** may
  speak: `checkFileModifiedOnDisk` returns unless `isMainFrame()`; a
  secondary task which was written to a file with `Ctrl-X Ctrl-W` still
  gets the save-time confirmation, just never a banner of its own.

`docs/web/file-modified-on-disk.md` is the user-facing page; the buttons of
the banner are found by a test through `findChild<QPushButton*>` /
`<QToolButton*>` under the objectName `file-modified-banner`, and
`test-file-on-disk` covers detect, dismiss, re-appear, reload, the `n` and
`y` answers of the question (answered the `TestLspClient` way: text of the
visible `LineEdit` plus `userEditingFinished`), and the new-file save which
must ask nothing.

### No Unicode line separator inside a block (`normalizeLineSeparators`)

A `QTextDocument` keeps the U+2028 LINE SEPARATOR and the U+0085 NEXT LINE
character **inside a block**, while `QTextLayout` breaks the **displayed** line
on them: such an invisible character makes a line the screen draws on two rows
and the document knows as one. `highlightCurrentLine` covers both rows (what
reads as "the first *and* the second line are highlighted"), and `Ctrl-E`, which
is `QTextCursor::EndOfBlock`, stops at the end of the block — that is, on the
*second* row, one character past where the line the reader sees ends — which is
"the end of the first line is not detected correctly". Qt itself turns U+2029
PARAGRAPH SEPARATOR into the end of a block, so that one needs nothing: what
matters is U+2028, the character the clipboard of a text cut on a folded line,
or a Java or JavaScript source, carries.

`normalizeLineSeparators` (`Utilities.cxx`, declared in `Utilities.hxx`) is the
one place that turns the two characters into `'\n'`, and the rule is that **every
text which reaches a document of the editor goes through it**:
`PlainTextEdit::setPlainText` / `insertPlainText` / `appendPlainText` (hence the
file constructors, `reloadFile`, the templates and the output of a process), the
same three of `TextEdit` and `TextBrowser`, `MultiLineLineEdit::setText` and
`insert`, and the two `insertFromMimeData` overrides.

Traps, all paid for:

- **Word wrap is not the cause, and a screenshot cannot tell them apart without
  measuring.** With wrap on (`WrapAnywhere`, what `PlainTextEdit` sets), the band
  of the current line spans the whole viewport on both readings; what distinguishes
  them is that the *text* stops well short of the right edge while the band runs to
  it, and `block.layout()->lineCount() > 1` for a block no visible break explains.
  Measured for a first line of 83 characters which fits the width: the band of
  `darkYellow` at alpha 60 is 17 pixel rows of one text row, and 38 — two rows —
  for the same line ending with U+2028 (a live display, `DISPLAY=:99` with
  metacity, `QWidget::render` of the buffer of the real editor); and the caret of
  `Ctrl-E` goes to x=484 on the row the line is drawn on, against x=4, y=18, the
  start of the phantom row, with it (offscreen).
  Fixing the movement primitives instead (`Ctrl-E` moving to the end of the visual
  line) would leave the two rows, the highlight over both, and the arrow keys, which
  do follow the layout, disagreeing with `Ctrl-E`.
- **Only the separators do this, so filter those and nothing else.** U+200B ZERO
  WIDTH SPACE and U+0085 NEXT LINE were checked the same way: the first is an
  ordinary zero-width character of the block, the second is a line break in the
  encodings which have it and does break the displayed line, so it is normalized
  with U+2028. U+2029 needs nothing, as Qt makes it the end of a block.
- **The entry point is the text edit, and paste is not one of its methods.**
  `QTextEdit` and `QPlainTextEdit` have no setter of the pasted text to wrap: the
  clipboard, the context menu, the X11 middle-click selection *and* drag and drop
  all funnel through the protected virtual `insertFromMimeData`, which the editor
  therefore overrides — in the anonymous-namespace `EditorTextWidget` of
  `PlainTextEdit.cxx` (no `Q_OBJECT`, so no `qt_sources` entry, and the base class
  vtable dispatches to it), and in `MultiLineLineEdit`, whose declaration of it is a
  vtable change: `cmake --build . --target TFELEditor` alone leaves the packaged
  modes built against the old layout, see the note on `SettingsGroup`.
- **The shim of the minibuffer needs it as much as the buffers.** Its height comes
  from `documentSize()`, so a phantom row is a row the entry pays for and `text()`
  says nothing about — a two-rows-tall input holding a one-line command.
- A raw `QTextCursor::insertText` is deliberately **not** filtered: what the modes,
  the kill ring and the LSP edits insert is text the document already holds, so
  whatever reaches such an insertion with a separator came from a document which
  was normalized when it was read.

`LspProtocol::textOf` keeps a U+2028 it finds inside a block as the character it
is (`testTextOf` asserts that), because the protocol counts a block as one line and
turning the character into a line break would move every offset behind it; that is
consistent with the invariant above, since no block of a document of the editor
holds one of the two characters any more.

`test-line-separators` covers the helper, the file which is read with a separator
at the end of a line (no block holds one, and the first line is drawn on **one**
row), `Ctrl-E` reaching the end of the line it was asked on, the paste of a text
which carries one, and `MultiLineLineEdit` — the input's `text()` and its number of
blocks. Run against a library which normalizes nothing, five of its seven cases
fail; the assertions are what the defect is, not an echo of the fix.

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

### The clangd session is started on being asked, and by the file of a project (issue #125)

The mode starts no server on its own account except in the one case below, and
the API is named for being asked: `LspClient::launch(extra_arguments, error)` and
`isIdle()` replaced `ensureStarted`, `waitUntilReady` waits for a handshake which
was asked for instead of starting one, and the two entries of
`CxxMajorMode::getSpecificMenu` (menu "C/C++"), `Start clangd` and
`Start clangd with a compile_commands.json` — commands `start-clangd` and
`start-clangd-with-compile-commands` — are what starts a session, with
`Stop clangd`/`stop-clangd` (`LspClient::stop`, which reports "clangd is
not started" when none was running) what ends one. Relaunching
ends the running session first (`LspClient::endSession`, shared with the
destructor, plus the `ending_session` flag that keeps that exit from reporting
"clangd exited" in the minibuffer), resets every document version to 0 so
`flushDocuments()` re-sends `didOpen` after the new handshake, and clears the
diagnostics the dismissed server published. The answer of the file question
becomes `--compile-commands-dir=<dir>` in
`LspCxxSupport::compileCommandsArguments`, tested without a server in
`test-lsp-protocol`.

**The one session the editor starts on its own** is decided in
`CxxMajorMode::startClangdIfNeeded()`, called from both paths of
`openOrRenameDocument()` (the document was already registered, or is registered
now) and so from the constructor of the mode of every buffer of a file. It starts
a session when three things hold: `LspCxxSupport::startedByDefault()` — the
setting `clangd/start-by-default`, read on **every call**, so turning it off needs
no restart and reaches the next file opened; `client->isIdle()`, because the
database a running session reads was chosen when it started and a buffer of
another project is none to replace the one somebody is working with; and
`implicitCompileCommands(editor, QFileInfo(documentFile).absolutePath())` answers
a non-empty `database`. The directory of *that file* is given to the search, not
the current buffer's directory, which is what the two menu entries pass. The
report replaces the message of a start whole — `clangd started using <path>` —
because a session nobody asked for has to say what it reads; `report()` is
`MajorModeBase::report`, i.e. `EditorWidget::displayInformativeMessage`, which is
what a test overrides to see it. A program which does not start is reported once
per mode (`auto_start_failure_reported`), since the mistake is in the settings and
every buffer of the project would repeat the same line.

`ImplicitCompileCommands` therefore holds `database` (the path the session is
started with, the command-line default included), and `implicitCompileCommands`
takes the directory to start the search from, empty meaning the current buffer.

**`Ctrl-Left click` is the mouse's `Alt-.`**: `CxxMajorMode::mousePressEvent`
(reached through `TextEditBase::handleMousePressEvent`, which asks the mode before
the text edit sees the press) takes plain Ctrl+Left only — another modifier,
another button, or a buffer with no session, no document or no file is handed to
`CMajorModeBase`. `jumpFromMouse(position)`: no identifier under the pointer
answers false at once; else the cached answers of `hovered_word` when its word,
its start and the document revision match and `capabilities >= 0` (the click which
follows the icon of the jump asks nothing), else both questions with the shared
countdown of `WordAnswers` and a 1 s `boundedWait` — a session which is not up is
not waited for, because a click which starts nothing and moves nothing is a click,
not a request. The definition wins over the declaration, as it does for `Alt-.`,
and nothing found reports "no definition found" and hands the click back.
`TrackedWord` hence stores `definitions` and `declarations`, not only what they
say of the word (`WordAnswers` replaced a `QPair<int,int>` of capabilities), and
`askTrackedWord` copies them from the cursor's word to the mouse's when the two
are the same word.

Four traps, all paid for:

- **The release of a consumed press has to be consumed too.** The mode sets
  `mouse_press_consumed` and its `eventFilter` swallows the next
  `MouseButtonRelease`: let through, the release moves the cursor back under the
  pointer after the jump put it on the definition.
- A test of a jump **must move the cursor off the target line first**: a freshly
  opened buffer has its cursor at position 0, which is the line a definition in
  that same file answers with, and the assertion then holds whatever the click did.
- `QTest::mouseClick(viewport, Qt::LeftButton, Qt::ControlModifier, over)` does
  reach the mode, and the answer lands where `QTextBlock::position() + character`
  says; `testCtrlClickFollowsTheDefinition` asserts on `blockNumber()` and not on
  the position, since the column of an answer is clangd's, not the mode's.
- A click which asks on its own waits a second, and clangd answers about a file it
  has parsed, so that case retries the click (up to eight times) rather than
  assuming the first press is answered; a click on a *cached* word lands inside
  the press.

### Which database a plain start uses (issue #125)

A session started without an explicit database takes
`LspCxxSupport::defaultCompileCommandsFile()` — the `--compile-commands-file=`
option parsed in `main` of `tfel-editor.cxx` into a file-scope static —, else
the result of `LspCxxSupport::searchCompileCommandsFile(directory_of_buffer)`,
else nothing. The search ascends while the *parent* holds a `CMakeLists.txt`
(a `src/` without its own `CMakeLists.txt` still belongs to the parent's
project; the alternative reading, ascending while the *current* directory has
one, stops at the first leaf and misses the database at the project root),
then looks into a `build*` sub-directory of the last project directory and of
the directory outside the project which holds it. The editor searches rather
than leaving it to clangd because `--compile-commands-dir` is fixed when the
session starts and the minibuffer names the database the session runs with. The
plain start reports the outcome once, not the miss as it happens: `startSession`
takes a `report` which replaces the `clangd started/restarted` message whole — `using
<path> to start clangd` or `no compile_commands.json found, clangd is
restricted to current C++ file`, with `(default file <path> not found)` appended
only when a bad default actually played no part. Three test traps:
a database left in a shared `QTemporaryDir` root is found by every later start of
the mode tests, by the automatic ones included (the cases which build a project
therefore live in subtrees of their own, `db-project/`, `auto-project/`,
`off-project/`, and a "no database" case needs the *parent* of its project to hold
no `build*` directory either, since that is the second place the search looks); a
database proving "the search found it" must be named something other than `build*`
when it is meant to be reachable by the command-line default only — the `#ifdef X`
+ `-DX` entry with an error inside is what makes "the database reached clangd" a
positive wait; and a test which asserts what a *start* says has to compare against
what the search answers, `QFileInfo::absoluteFilePath()`, so the collected
messages are matched with `startsWith`/`contains`, not with an equality on a path
the test wrote itself.

What keeps the automatic session out of the way of the cases which start their
session through the menu — and assert on what the menu's message says — is a
setting, not a mock: `TestLspClient::isolateSettings()` writes
`clangd/start-by-default=false`, and the `cleanup()` slot re-applies that after
every case, because a case which failed on its way out would otherwise leave the
switch on for the cases which run after it. The three cases which are about the
switch (`testAFileOfAProjectStartsClangd`,
`testAFileWithoutADatabaseStartsNothing`, `testStartingByDefaultCanBeTurnedOff`)
write what their name says and stop the client they started.

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
  question**, not at the start of a session, because the colors of a buffer are
  the kind of thing a person turns off the first time they dislike them and expects
  the text to change back under their hands. The checkable `Color identifiers` entry
  of the `C/C++` menu writes that setting and repaints the buffer of the file on
  screen at once; the other buffers follow as they settle.
- Two spelling notes: the prose of the tree is American (`color`, `colors`; the one
  British occurrence is a pre-existing comment in `DirectoryMajorMode.hxx`), and the
  menu entry is `Color identifiers`, so a test which asserts on the text of the entry
  has to spell it that way.
- The `C/C++` menu has seven entries: the two which start a session, a
  separator, `Go to symbol...`, `Color identifiers`, a separator,
  `Stop clangd` — which moved `Stop clangd` from
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
is started — is about the directory and about the index. The options a file is
compiled with are not frozen for the session, which is why no `QFileSystemWatcher`
restarting clangd was added: it would buy the index of unchanged files nothing (the
shards survive the restart) and would throw away a session whose parse is already
current. `docs/web/clangd-integration.md` keeps the same conclusion, and the one
part of the idea that would still pay — watching the file to *report* a stale
index, not to fix it.

### One session per buffer and per server: `LspNavigationSession` (issue #135)

The half of a mode which speaks the protocol is a core class of its own,
`LspNavigationSession` (`src/LspNavigationSession.cxx`, 2019 lines), which
`CxxMajorMode` (643 lines of clangd policy, down from 2285) and the `Python`
mode both drive. It is a **composed `QObject` owned by the mode**, one per text
edit — the `GrammarCheckSession` shape — not a base class of the mode: the
questions and the answers are the protocol's, not the language's, and what a
language adds is small enough to be a handful of overrides.

A mode opts in by deriving, in the **anonymous namespace of its own `.cxx`**, a
file-local session which answers the few questions the generic code cannot:

- `menuName()` and `startCommandName()` — where a session is asked for, which is
  what the message of a request with no session points at;
- `declarationSupported()` — `pylsp` answers `-32601` to
  `textDocument/declaration`, and asking a refused question per tracked word
  makes every word the cursor rests on wait on nothing;
- `semanticTokensWanted()` and `semanticFaceOf()` — which identifiers are asked
  what they are and what the answer is drawn with;
- `startSessionIfNeeded()` — the one place a mode may start something nobody
  asked for;
- `report()` — override for a message the generic wording cannot shape.

Such a derived class has **no `Q_OBJECT`**: it declares no signal, slot or
property of its own, and `qt_sources` mocs `<header-dir>/<name>.hxx` only, which
a class of an anonymous namespace is not.

Everything the session says and draws is keyed on the **server name** the
constructor is given: the faces are the extra-selection keys `<name>`,
`<name>-navigation` and `<name>-tokens`, the references menu is the object name
`<name>-references`. That is what keeps the diagnostics of two servers, or a
server's and a proofreader's, off one another, and it is what lets the `Python`
mode draw under `pylsp` while every `clangd` string a test asserts on stays as
it was.

Two shape traps, both about the order things are built and destroyed:

- **The constructor does not call `openOrRenameDocument()`.** That is what asks
  `startSessionIfNeeded()`, a virtual, and a virtual call from a base
  constructor reads the base answer — the C++ mode would never start clangd, and
  no crash would say so. Each mode calls it as the **last statement of its own
  constructor**.
- **The destructor may not ask the text edit for its widget or document.** A
  mode is destroyed as a child of the text edit, whose own `QWidget` is gone by
  then. Hence the `QPointer<QTextDocument> document` member, and hence
  `textEdit.widget()` is nowhere in `~LspNavigationSession`.

The client is found through `LspClient::get(EditorWidget &, name)`: one client
per editor widget **and per server**, since a `C++` buffer holds clangd and a
proofreader at once. A *stop* uses `editor.findChild<LspClient *>(name)` instead
of `get`, which would build the client of a server nobody ever started.

`tests/TestLspClient.cxx` is the acceptance test of the extraction: every clangd
behaviour the mode used to hold itself must answer the same through the session.

### A whole-document edit is applied where it differs (issue #135)

`jedi` renames a symbol by answering `textDocument/rename` with **one edit whose
range is the whole file**, and `autopep8` reformats it the same way through
`textDocument/formatting`. Applying such an answer as it comes is right and
wasteful: the cursor goes to the end of the file, the buffer reads as modified
down to its last line, and two letters changed is reported as one edit of a
thousand. `documentChangesOf`, in the anonymous namespace of
`src/LspNavigationSession.cxx`, is the one place that decides the shape of every
edit of an `applyWorkspaceEdit`, and the rule is:

- a range which does **not** name the whole document is applied as it came —
  which is the shape of every clangd rename and every quick fix;
- otherwise the text the two answers hold in common at their **ends** goes first
  (`common_prefix` / `common_suffix`), and what is left is compared line by
  line: when the old and the new text have the same number of lines, each
  differing line becomes its own edit, trimmed to what differs within it, so the
  text *between* the changed lines — which the answer repeats for nothing — is
  left where it is;
- when the line counts disagree, one edit is what it stays, trimmed at the ends;
- when the trimmed texts are both empty, the answer says what the text already
  says: **no edit at all**, which is what lets `formatDocument()` answer false
  and the mode stay silent on an already-formatted buffer.

Positions are the reason this is expressible at all: the places of a document and
the indices of its `toPlainText()` are the same numbers, because a block separator
is one character in both and no block of the editor holds a character
`toPlainText` would write as more than one — the invariant
`normalizeLineSeparators` keeps.

### The overview names a symbol the way its language spells it (issue #135)

`LspProtocol::scopeSeparatorOf(languageId)` answers `.` for `python` and the
languages of the JVM and `::` for everything else, the empty name included —
which is what the `C++` mode sends, so its overview is untouched.
`documentSymbolsOf(result, separator)` takes it, and `LspClient::requestDocumentSymbols`
captures it from the client's own `language_id` when the question goes out. A
reader who copies `Widget.paint` out of the list pastes something Python reads,
and `Widget::paint` is nothing of that language.

**Both branches are qualified**, not only the tree: `pylsp` ignores
`hierarchicalDocumentSymbolSupport` and answers the flat `SymbolInformation`
list, whose `containerName` is filled, so the flat branch is where the Python
overview is made.

### The Python mode starts its server with the file (issue #135)

The policy is the opposite of clangd's, and the reason is what the server needs:
`pylsp` requires no compilation database to say what a name is, so there is no
project to find first and no choice to make — a file of the language is enough.
`PythonNavigationSession::startSessionIfNeeded` therefore starts a session when
three things hold: the buffer visits a file (`*scratch*` and a process output are
none of the server's business — the protocol identifies documents by name),
`pylsp/start-by-default` (read on **every call**, so turning it off reaches the
next file opened with no restart), and `client().isIdle()` (the root a session
reads is chosen when it starts, and a buffer of another project is none to
replace what somebody is working with). The client is told
`setLanguageId("python")` and `setRootUri(<directory of that file>)`, and the
report names what it reads: `pylsp started with project root <dir>`.

Three string contracts tests assert on: `pylsp started with project root %1` for
the automatic session, `pylsp restarted` when a session was running and the menu
replaced it (`LspClient::launch` ends the running one and re-sends every document
after the new handshake), and `pylsp stopped` / `pylsp is not started` for
`stopPylsp`. A program which is not to be found is reported **once per mode**
(`auto_start_failure_reported`) — the mistake is in the settings and every buffer
of the project would repeat the line — while `Start pylsp` says it as often as it
is asked. Requests never start a session, `Alt-Q` included: `PythonMajorMode::format`
reports `pylsp is not started: …` **only** when the client `isIdle()`, because an
already-formatted buffer is the common answer of that key and a message per press
would make it unusable.

### What `pylsp` answers, measured (issue #135)

All of it read off `pylsp` 1.15.0 by hand in `/tmp/opencode/pylsp/`:

- the handshake takes ~0.6 s and `textDocument/documentSymbol` is answered ~0.1 s
  after it; `textDocumentSync` says `{change: 2}`;
- `textDocument/declaration` is refused with `-32601`, the semantic tokens
  provider is absent, and `textDocument/codeAction` answers the empty list — which
  is why the three clangd features those answers feed (the declaration half of a
  jump, the identifier colors, the `Fix:` entries of the menu) are asked of nobody
  here, rather than asked and found empty on every buffer;
- `textDocument/formatting` answers `[]` for a file which is already formatted;
- the `autopep8` plugin fixes the spacing after a comma and around a parenthesis
  and **not** `a+b` — `E226` is in its default-ignored list — so a test of the
  format key formats `f(a,b)`, not `a+b`;
- `jedi`, the completion library behind it, is what answers a rename as one
  rewrite of the whole file, see the narrowing above.

### The Python highlighter is a scanner, not a list of regular expressions (issue #135)

`PythonSyntaxHighlighter` walks each block itself, because three decisions of
this language are not matchable:

- **A docstring is prose, the same three quotes elsewhere is a literal.** What
  decides is `opens_a_statement(previous)`: walk the blocks back over the blanks
  and the `#`-only lines, and the first statement above the line says whether the
  string which opens the body of a `def`, a `class` or the module is prose.
  **Block states** carry that across lines: `0` is nothing open, `1`/`2` are an
  open `'''`/`"""` literal, `3`/`4` the same open as a docstring, which is why the
  state names both the quote and the reading. A `previousBlockState()` outside
  `0..4` is treated as `0`.
- **A string prefix is part of the token**: at most two letters of `rRbBuUfF` and
  a quote right after them, which is what tells `f"…"` from the tail of a name
  ending in `f`. Inside the literal a backslash hides the next character **in the
  raw forms too** (`r"\""` is a two-character string), and inside a triple-quoted
  one a run of one or two quotes is prose and is walked over.
- **A decorator is a statement.** An `@` at the start of a line is a decorator,
  the same character mid-line is an operator and is left alone.

The soft keywords — `match`, `case`, `type` — are deliberately **not** in the
keyword set: coloring them by name inks over the variable a great many files call
`case`. Faces: keywords dark magenta bold, strings dark green, docstrings dark
green italic, numbers, comments and `True`/`False`/`None` dark red (the comment
lightened), builtins, decorator and the name a `def` introduces dark cyan, the
name a `class` introduces dark blue bold.

### Test traps of the Python mode (issue #135)

`tests/TestPythonMode.cxx` is the fourteenth executable; what it costs to test a
mode whose class is not exported:

- the mode is loaded **as a plugin**: `TFEL_EDITOR_PYTHON_MODE` is a
  `target_compile_definitions` holding `$<TARGET_FILE:TFELEditorPythonMode>`,
  with `add_dependencies`, and the case `QSKIP`s when `QLibrary::load` fails. Such
  a load registers the mode for the rest of the run — the rule `TestSettings`
  carries for `castem`; no other suite is affected because each test is its own
  executable;
- the faces of a highlighter are invisible to `charFormat` and to
  `extraSelections()` (see "Auditing a mode without a test suite"), so
  `countFaces` classifies the pixels of `widget()->grab()` by the shape of their
  colour — the channels which stand for a face well above the third — which tells
  the five faces apart and leaves black text, its antialiasing and the band of the
  current line alone. **A zero-control is what makes that evidence**: a second
  buffer of the same mode holding `qqqq qqqq` must count 0 of every face, or the
  counts say nothing about the highlighter. `block.userState() != -1` is how a
  test waits for "the highlighter has been through here";
- the answers the `MajorMode` interface does not declare, `getCommentSyntax`
  among them, are reached by casting the mode to the **exported base which
  declares them**: `qobject_cast<MajorModeBase *>(mode)`;
- `isolateSettings()` writes `pylsp/start-by-default=false` **and removes
  `Python/compilation/history`**: the `Alt-m` prefill is the last answer, not the
  default, and a case asserting the prefill is `python3 <name>` is answered by the
  previous case otherwise. `cleanup()` re-applies the isolation, as a case which
  failed on its way out would leave the switch on;
- `pylsp` is a `pip` program, not one this machine is owed: `hasServer()` is
  `QStandardPaths::findExecutable("pylsp")` non-empty, and every case which asks
  it something skips otherwise (which is why `hasServer` is a plain predicate —
  `QSKIP` only leaves the function it is written in, so a helper cannot skip for
  its caller). Running the suite by hand needs `PATH` to hold the `bin` directory
  `pip install --user` wrote to;
- the live minibuffer question is found the `TestLspClient` way — `CommandLine` by
  objectName (`rename-symbol`, `go-to-symbol`) which is **still laid out** — and
  answered with `lineEditOf(q)->setText(…)` plus `q->userEditingFinished()`.

The ABI of `CxxMajorMode` changed with the extraction (its members moved to the
session), so a probe or a packaged mode built against the old layout is a crash
far from the cause: rebuild the whole tree, not `--target TFELEditor`.

### The Tab key walks the levels `python.el` computes (issue #135)

The indentation of the language is a model of `python-indent-line-function` of
`python.el`, **GNU Emacs 27.1**, measured press by press against the real thing:
`/tmp/opencode/pylsp2/indent28.el` drives `emacs -Q --batch` on the extracted
`python.el` of this machine and writes the cycles of fourteen shapes, which
`/tmp/opencode/pylsp2/c28.txt` holds the engine answering the same shapes in
C++ next to. A cycle is not a thing a description pins down; the numbers are the
specification, and where the two lists differ the mode is wrong.

Two questions are answered for a line, as by the real function: which
indentation the file above allows it (`proper`, by the order the contexts are
tried, in the header comment of the engine, `PythonMajorMode.cxx:755`), and which
levels a run of presses walks it through (`Indentation::levels`). The rule of the
presses:

- a fresh press puts the line at `max(levels)`; each further press takes it to
  the highest level **strictly below** the place it holds; the press which sits
  at the lowest level goes back to `max(levels)`. So the cycle length is the
  number of levels and the first shape of the user's report,
  `print / if True: / if False: / print`, answers `8 4 0 8` from *whatever*
  indentation its last line started at;
- for a plain `proper`, the levels are the multiples of the level under it plus
  `proper` itself (`ladderLevels`), which is how a continuation line of
  `def f(a,` gets `6 4 0`: 6 is right behind the bracket, and is no multiple of
  four;
- a dedenter carries its own levels, the indentations of the blocks it closes,
  and they are nowhere near a ladder. Measured: an `else` whose `if` is the body
  of an `if` whose body already holds a line answers `4 4 4 4` (no press moves a
  line already where the language puts it), an `else` of three nested `if`s
  answers `8 4 0 8`, a line after the `return` of the block above answers
  `0 0 0 0`;
- a press continues the cycle only when it finds the line the previous press
  indented standing at the place that press gave it — hence the members
  `cycle_document`, `cycle_block`, `cycle_target`, and hence a fresh buffer per
  shape in a test: a reused buffer inherits the cycle of the run before it.

`Shift-Tab` is **the mode's own key**: 27.1 binds no `S-TAB` and has no
`python-indent-shift-backwards`, so its behaviour is designed on the measured
ladder — highest level below the current place, and it *stops* at the lowest
level instead of wrapping, since a key which wrapped would put the line back in
the block the other key takes it out of. It is claimed in
`PythonMajorMode::keyPressEvent` before the text edit binds it to completing the
previous item; completion is `Alt-Tab` here. With a selection over more than one
line both keys call `shiftRegion` (one level right, one level left): the lines
are **moved**, not reindented, and a blank line of the region is left blank —
an empty line is how two functions are separated in this language. A selection
inside one line is no region and falls back to `indentLine`.

Three traps, all paid for:

- **`QTest::keyClick` is overloaded on `char`, and an `int` key argument selects
  that overload.** It maps its argument through `QTest::asciiToKey`, which
  asserts on `Qt::Key_Tab`: `ASSERT: "false" in file ./src/testlib/qasciikey.cpp,
  line 228`, in the *test*, not in the mode. What makes it costly is that
  `build_probe.sh` compiles with `-DQT_NO_DEBUG`, where that assert is simply not
  there and the answer is the key `0`: a press which does nothing at all, so the
  hand-built probe which reproduced the report read "the engine never indents"
  while the suite died on the first press. The parameters of a helper which
  presses keys are `Qt::Key`, never `int`, and its comment says why.
- A case which asserts that a **single-line selection indents its line** needs a
  line with somewhere to go: the top lines of the shapes of the ladder all answer
  `0 0 0 0`, so on them the assertion holds whatever the key does. `def f():` +
  `q` in a buffer of its own is what gives the line a level to reach.
- The mark which `shiftRegion` restores around the move **leaves no selection
  behind**, so a test which presses the key twice has to select the region again
  before each press.

### The Python interpreter session, and the two shapes of a send (issue #135)

`Ctrl-C Ctrl-L` / `Ctrl-C Ctrl-R` / `Ctrl-C Ctrl-B` send the line, the region and
the buffer to an interpreter, modelled on `CastemMajorMode::startCastem` (read
that pair of sections first, the shape is shared): a `ProcessInteractionFrame`
with `WA_DeleteOnClose`, a `destroyed` connection which forgets the frame only if
it is still that very frame, a write guard of `!= QProcess::NotRunning`, the text
of the block taken with `block().text()`, the bytes handed over as `toUtf8`, and
an `errorOccurred` handler connected *before* `attachSecondaryTask` which names
the program on `FailedToStart` and takes the frame away through
`detachPythonFrame`. The frame is titled `Python Output` and gets no
`setMajorMode`. The program is `python/executable`, started `-i -u` in the
directory of the file.

Numbers and shapes, all measured over a pipe (`/tmp/opencode/pylsp2/probe*.cxx`):

- `-i` keeps the interpreter up after what it was handed; without it the line
  which started the session is the whole session. `-u` is what lets the banner
  and the `>>> ` prompts reach the frame as they are made: without it the frame
  reads as empty while the interpreter has been answering for a while. An error
  inside what is sent does **not** end an `-i` process, which is what a session
  wants: a traceback is an answer, not a reason to close the page.
- **A line is written as it stands, plus an empty line when it opens a block**
  (`sendStatement`, the same extra line Emacs sends); a text of several lines is
  written to a `QTemporaryFile` and executed from it,
  `exec(compile(open(<path>, encoding='utf-8').read(), <path>, 'exec'))`, because
  the interactive parser ends an indented block at an empty line and a function
  whose body holds a blank line comes back as `IndentationError` when written as
  text. Two defects of that route hid as "the keys do nothing", and both were
  found by hand, not by reading: the path was written **unquoted**, so the
  interpreter read `open(/tmp/x.py, …)` — the division of a name by two names, a
  `SyntaxError` — and the two parentheses which close `exec` and `compile` were
  missing, which is not an error it prints and forgets but a line it waits for
  the end of: the parser goes into continuation mode, the frame shows `...`, and
  every send after it is swallowed as the tail of that one.
- **The three keys are claimed whatever the outcome.** `handleShortCut` returns
  the value of the send in the Cast3M mode, which is how an interpreter which was
  simply not started became `unknown shortcut : Ctr-C - L`, a report about the
  key bindings of the editor for a problem of a path. The Python mode claims the
  keys outright and lets the slots report.

Two facts of the settings, both asserted by a test:

- the **declared default of `python/executable` is `python3`**, through the
  constant `default_python_interpreter` that `pythonInterpreter()` falls back to,
  as `castem/executable` does: the field shows the program the mode would run and
  an answer equal to it is removed from the file instead of stored. With an empty
  declared default, `theToolsOfTheDialogChooseRuffAndNoHistory` fails on
  `program.default_value`;
- the two `.../history` addresses the `Format with` and `Sort imports with`
  questions write (`pylsp/formatter/history`, `pylsp/import-sorter/history`) are
  deliberately **not** declared entries, the rule the configuration dialog
  carries.

### The transcript of a session shows the sends at their prompts (issue #135)

A `QProcess` pipe is not a terminal: `python3 -i -u` echoes nothing, writes its
`>>> ` without a line break *before* reading the line, and answers each send in
one piece shaped `<answer>\n>>> ` (all measured over a pipe,
`/tmp/opencode/pylsp2/stream.cxx`, `prompt.cxx`). Left to itself the frame shows
`>>> toto` — the answer behind the prompt of the line it answers — with the sent
line nowhere, the report's two symptoms. `ProcessOutputFrame::displayProcessOutput`
therefore appends with a `QTextCursor` at `End` and not `appendPlainText`, which
forces a new paragraph whatever came before and can never continue the prompt's
line (a `follows = c.atEnd()` test keeps a reader who clicked into the scrollback
— the grep mode navigates through `textCursor()` — from being dragged to the end
by the next chunk; the frame's other consumers write nothing but process bytes).

And the mode, not the frame, owns the conversation: `sendToPython` no longer
writes — it queues a `PythonSend{text, whole_block}` and
`PythonMajorMode::flushPythonSends` hands one send to the interpreter per prompt,
writing `echoed(text)` at the end of the document (`insertBlock` between lines,
never a `\n` into `insertText`) *before* the bytes. Prompt-gating the **bytes**
is what makes the first send right: the banner and the answer of the first line
can share one read of the pipe, so a line written at once was answered before any
echo could be placed (`>>> toto` with the echo below it, measured). The temp file
of the whole-block route is written at the flush, not at the enqueue — two queued
sends share the file, and text written at the enqueue would be what the first
exec line reads. `ends_at_prompt` accepts `>>>` and `...` with trailing spaces
stripped; a program which prompts otherwise (an `In [1]:` shell) never reads the
queue, the same trade `comint` makes.

The connection trap, paid for in full: `displayProcessOutput` **disconnects and
reconnects itself on every chunk**, which moves its own connection to the *back*
of the signal's slot list — a directly connected judge added after the frame's
constructor runs *before* the text is appended on every chunk after the first,
and no send was ever placed (the flush log named the guard: "no prompt, last
block `\"\"`" while the banner sat on screen). The flush connection is
`Qt::QueuedConnection` instead, which is correct and not merely enough: a flush
then always runs after `displayProcessOutput` of its chunk, and nothing can be
in flight from the interpreter that a flush did not itself write, because the
bytes wait behind the same prompt as the echo.

The frame half of that fix is invisible to the Python transcript test — the
mode's echo always ends its own line, so `appendPlainText` and the cursor
append build the same document when every chunk boundary lands at a line break
(a verified mutant of `displayProcessOutput` passed the transcript case). What
pins the contract itself is
`TestSharedCompilationOutput::theOutputOfACommandContinuesTheLineOfThePrompt`:
a `sh -c` which writes `>>> `, waits, writes `toto\n>>> `, waits, writes
`again\n` — and the assertions say the first two pieces are **one** block
(`>>> toto`), that the reader who parked its cursor at `Start` is not dragged
by the third piece, and that the third piece still completes the prompt's line
(`>>> again`). Mutating the frame to `appendPlainText` fires it (3 blocks where
2 were asked). Two traps of writing that case:

- `QCoreApplication::processEvents()` **never delivers the bytes of a pipe** —
  `QProcess` reads through a socket notifier, and plain event pumping does not
  expire it: a wait loop built on `pump()` saw an empty document for the whole
  timeout while `framedump` (the same shape, probing) showed every piece. What
  works is `QTest::qWait`, which runs socket events — the `waitFor` helper of
  that suite. (`waitForReadyRead` would expire the notifier too, but it spins
  its own loop inside the slot and re-enters whatever judges the test connected.)
- the reader of a frame's document cuts on **both** separators: this Qt 6.2
  answers `document()->toPlainText()` of a plain-text-edit with bare `\n`
  between the blocks, no U+2029 at all (a code-point scan of the whole text is
  pure ASCII, `/tmp/opencode/pylsp2/framedump.cxx`), while other readings of
  these documents spell the end of a block with U+2029 — `replace(0x2029, '\n')`
  then `split('\n')` is what `receivedLines` of the Cast3M test already does and
  what the frame case copies.

### Test traps of the interpreter (issue #135)

- A file which ends with a newline gives the document an **empty last block**, so
  `movePosition(End)` then `StartOfBlock` puts the cursor on that empty block:
  the send reaches the interpreter as an empty line, the frame gains one more
  `>>> ` and nothing else, and the case reads as a session which is up and answers
  nothing. The line is looked for by its **block number** and the text found there
  is asserted before it is sent.
- The text of the session is `qobject_cast<QPlainTextEdit *>(frame->widget())
  ->toPlainText()`: `ProcessInteractionFrame` is a frame of the editor, not a text
  edit, and `out->toPlainText()` does not exist.
- `QTest::keyClick(frame.widget(), …)` reaches `indentLine` and `handleShortCut`
  of the mode, and a report is waited for with the `RecordingEditor`'s
  `told(…)`: `FailedToStart` arrives through the event loop, some ten
  milliseconds after `start()` returned.
- The three cases about the tools of the language —
  `theMenuOffersTheToolsOfTheLanguage`,
  `theToolsOfTheDialogChooseRuffAndNoHistory`,
  `theFormatterChoiceDecidesWhoAnswers` — failed once together on a shared
  `~/.config/tfel-editor-tests/test-python-mode.conf` of 77 KB and were green
  again with the declared default above in place. If they fail in a group again,
  re-run with `XDG_CONFIG_HOME=/tmp/opencode/pylsp2/cfg` before believing the
  mode.

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
compilation command `Alt+M` proposes per mode), the `recent files` list, the
`.../last-interface` addresses of the `MFront` mode, and the `explorer/panel-shown`
and `explorer/panel-width` addresses which carry the file explorer panel from one
window to the next are deliberately **not**
entries: a `CommandLine` is given an address and writes it back when its answer is
taken, so such a field shows a stale list and is overwritten by the next prompt on
its own account, and a panel's open state and dragged width are the same kind of
memory — the window writes them on its own, whatever the dialog says.
A test asserts those addresses are not declared
(`theRegistryDeclaresTheEntriesOfTheEditor`), which is what keeps the next person
from putting them back. The themes that remain are `General` and `Spelling and
grammar` (with the `Harper` and `LanguageTool` subthemes) from the core, `clangd`
from the C++ mode, `castem` from the Cast3M mode, `pylsp` and `python` from the
`Python` mode, and `latex` from the `LaTeX` mode; a packaged mode which reads
nothing but a history exports no theme of its own. The proofreader themes belong
to the core, not to the C++ mode: `harper-ls` and LanguageTool read prose, and
prose is read in the three modes which hold a buffer of text and in none of them
alone.

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
- because the state right after a start is `Starting` and not `Running`, a command
  which asks for `Running` before writing loses the line which started the session;
  what the state is worth is in the section below.
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

### The line which starts the solver reaches the solver (issue #131)

`Ctrl-C Ctrl-L` / `Ctrl-C Ctrl-R` / `Ctrl-C Ctrl-B` of the `Cast3M` mode send a
line, the region and the whole buffer to the interpreter. Two independent defects
made the *first* line of a session do nothing, and the symptom was doubled by a
second report: the minibuffer said `unknown shortcut : Ctr-C - L` because
`handleShortCut` returns the value of `sendLineToCastem`, which was answering false.

- **A process which is starting is not a process which must not be written to.**
  The guard was `state() == QProcess::Running`, and the state right after
  `startCastem` is `Starting`: the first `write` is not reached inside `start()`
  either — `QProcess` on this Qt 6.2.4 answers `Starting` / `UnknownError` when
  `start()` returns for a program which does not exist, and reports
  `FailedToStart` through `errorOccurred` about 10 ms later (`probe3.cxx`). The
  guard is now `!= QProcess::NotRunning`, and **`QProcess` itself queues a write
  made while `Starting`** and flushes it at `started` (`probe.cxx`: `write`
  returned 11 with `bytesToWrite() == 11`, the state became `Running`, and
  `/bin/cat` echoed the line back). So no `waitForStarted()`: that blocks the
  GUI for up to the timeout of a program which is only slow to appear.
- **The selection of the block under a cursor carries the *preceding* line
  separator, and that separator is U+2029.** `select(BlockUnderCursor)` on the
  second block of a `QPlainTextEdit` selects `U+2029` plus the text; the mode
  converted that with `(l + '\n').toLatin1()`, which has no rune for it and
  answers `'?'` — the interpreter was fed `?result = 2*2;`. Verified on Qt 6.2.4
  and Qt 6.9.1, offscreen and on a live X11 display, with and without wrapping
  and with hard breaks in between (`block5.cxx`, `block5_69.cxx`: the first block
  has no separator, the second begins with `U+2029`). The three slots now take
  `cursor.block().text()` (and the lines of `cursor.block()` for a region), which
  is the text of the line and nothing else.
- The same `U+2029` is what the frame of the session shows: `QPlainTextEdit` renders
  it as a **paragraph separator**, so `toPlainText()` of the echo of `/bin/cat` reads
  `    1|?result = 2*2;` with the parts separated by U+2029 and *no* `'\n'`. A test
  must cut on the code point and drop the empty parts.

`~ProcessOutputFrame` had the same `Running`-only test for its `terminate()`, so a
child still `Starting` when the buffer was killed was left to `~QProcess`, which
`SIGKILL`s it and prints `QProcess: Destroyed while process (...) is still
running`; `terminate()` does reach a `Starting` process (`term.cxx`: the state goes
`Starting` → `NotRunning`). The condition is now `!= QProcess::NotRunning`.

Test traps, all paid for: the *class* is exported but not the `Session` helper
which follows it in the same header-less `.cxx`, so the out-of-class definition
must not qualify it; two `sendLineToCastem` calls in a row send the **same** line
until the cursor is moved; and the assertion which proves the fix is the *first*
line of a session — everything after it worked before.

Two adjacent defects of exactly this kind are deliberately left alone, as they are
outside the mode this issue is about and each needs its own fix: the shell command
of `GnuplotMajorMode.cxx:80` sends the `BlockUnderCursor` selection, and the file
name `DirectoryMajorMode.cxx:99` opens is taken from a token which keeps its
leading separator (`dir.cxx`: the first token of `U+2029 + "base.o"` is
`U+2029 + "base.o"`, so the entry of a line which is not the first of the buffer
would ask for a name no file has).

### The configuration dialog keeps the themes of the modes apart (issue #127)

`SettingsGroup::mode` — the name of the major mode which exported the theme, empty
for a theme of the editor — is what tells the two kinds of themes apart, because a
theme named after a program it drives (`clangd`, `castem`) says nothing of the kind
by its name. `StandardMajorModeProxy` fills it as it fills the name, description and
icon of a theme which names none of its own, and `appendGroup` keeps the first
attribution, as it keeps the name. `ConfigurationDialog` splits `getGroups()` on that
field and puts a non-selectable title (`Qt::NoItemFlags`, bold, the grey of disabled
text) over each block — the title alone keeps the blocks apart, a blank row between
them read as a hole in the list — and a bold caption naming the
mode at the top of every page of a mode theme (`objectName`
`configuration-page-mode`, which is how a test singles it out).

The list is a **`QTreeWidget`** since the proofreaders: `SettingsGroup::parent` — the
fourth argument of `StandardSettingsGroupProxy`, before the registering `bool` — says
that a theme is a corner of another one, `SettingsRegistry::getGroups()` answers such
a theme directly after its parent (`orderUnderParents`, which keeps orphans and a
cycle of promises in incoming order rather than losing a theme), and the dialog hangs
the item of a sub-theme under the item of the name. `Harper` and `LanguageTool` are
declared as subthemes of `Spelling and grammar`, and the theme of the choice carries
the hunspell dictionary entry, which is no a theme of its own (`Spell checking` is
gone; the entry is in `spelling_entries()` of `SpellingAndGrammar.cxx`, under
`TFEL_GUI_HUNSPELL_SUPPORT`). `expandAll()` runs **after** the items are built.

Five traps, all paid for:

- A `QTreeWidgetItem` is **not a `QObject`**: no `setObjectName`, and a test cannot
  single an item out by name. What an item means is kept in the item itself,
  `setData(0, Qt::UserRole, page)` — and **every** method wants the column —, and
  `data` of an **unset** role answers an invalid variant whose `toInt()` is `0`, a
  real page. Every item, headings and the gap included, has to carry the role
  explicitly (`-1` for what shows nothing), and `toInt()` of it is then meaningless
  without that guard. `setForeground` wants a **brush**, not a colour: the palette
  answers `brush(QPalette::Disabled, QPalette::Text)`.
- A **top-level** item has `parent() == nullptr`, *not* the invisible root: whoever
  asserts "this theme stands on its own" compares against null. And `QVERIFY2`
  evaluates its message **whether the assertion holds or not**, so a message built
  with `item->parent()->text(0)` segfaults on the path which passes — take the
  parent in a named local and make the message null-safe before the test.
- The old `connect(themes, currentRowChanged, pages, setCurrentIndex)` cannot be
  kept once items exist which name no page: the signal fires for a non-selectable
  item too. The slot is a lambda which sets the page only when the role is not
  negative, and a test asserts that choosing a title leaves `currentIndex` alone.
- `QLabel` has **no `setForeground`** (that is a list item): the grey of a caption
  is a `QPalette` of the label itself.
- Adding a member to `SettingsGroup` is an **ABI break for the packaged modes**: a
  probe which loaded a `castem` plugin built against the old layout into a core
  library built with the new one segfaulted in the reallocation of a
  `QVector<SettingsKey>` inside `appendGroup`, three frames away from anything that
  looks like the cause. `cmake --build . --target TFELEditor` is enough to see a
  change of the dialog; it is not enough to run a probe against the plugins.

`TestSettings` asserts the shape with `theThemesOfTheMajorModesAreSeparatedFromThose
OfTheEditor` (the titles are the non-selectable items and show no page, every theme is
on its own side of the titles, a sub-theme is a child of its parent and of nothing
else and its submenu is expanded, a title changes no page, a caption names the mode
and only mode pages have one), and `theDialogShowsWhatIsDeclared` reads the themes as
the **selectable** items of a depth-first walk (`readList`) — `getGroupsNames()` and
`pages->count()` both disagree with `themes->topLevelItemCount()` now.

### The check box of an entry stands at the right edge of the line of its name (issue #127)

A boolean is a question with a yes or no answer, and a page of them is read as
a list, so the name and the box stand on **one line** and the boxes of a theme
stand in **one column, flush with the right edge of the page**:
`form->addRow(row)` with a spanning `QWidget` whose `QHBoxLayout` (margins 0)
holds the name, then a stretch, then the box
`Qt::AlignRight | Qt::AlignVCenter`. Every other kind keeps `WrapAllRows`, the
name above a field the width of the page, because two columns starve the path
of a program (187 px on the `clangd` page).

- **The column is the edge of the page, not the place behind the longest name.**
  What this replaced pinned every name to `max(sizeHint().width())` over the
  entries of the theme which are a `BOOLEAN`, which gave a column of boxes in
  the middle of the line and an empty right half reading as a field waiting for
  something. The **stretch between the name and the box** is what makes the
  column: a `QLabel` of a `QHBoxLayout` is stretched to what the box leaves, so
  without it every box stands at the end of its own name, a ragged edge which is
  not read as a column. Measured at 760 x 520: `clangd` 3 boxes at x=453
  offscreen (page 476 px), `pylsp` 9 boxes at 439 offscreen where the pinned
  column left them at 257 (page 462 px); on `DISPLAY=:97` with metacity the same
  gives 465 and 439 against 465 and 278 — the `clangd` page lands on the same
  pixel either way because its longest name is itself a boolean and the page is
  at its minimum width there, which is why a live display is worth checking on
  both pages and not one.
- The row of a boolean is built in the **one loop over the entries**: nothing is
  measured any more, and the first loop which only existed to know the column
  with it.
- `l->setBuddy(e)` names the `QCheckBox` itself, which is what makes a click on
  the name toggle the box; `Field::widget` stays the box, so `valueOf`,
  `setValueOf` and `restoreDefaults` read and write it as they did.
- `TestSettings::theCheckBoxesOfAThemeStandInOneColumnOnTheirOwnLine` holds the
  three halves — one line, one column, that column the right edge of the line —
  and each was found by mutating the layout: `addRow(l, e)` for a boolean fires
  the line assertion (and fires it on the *caption* three rows above, since the
  box's parent is then the page and `findChild<QLabel*>` answers its first
  label), and a stretch *after* the box, the ragged shape, fires the flush one at
  `x=141` against a line 457 pixels wide. The slack of three pixels the assertion
  allows is what a style may leave between the last item of a line and its edge;
  the pinned column this replaced stood 23 pixels away on the `clangd` page and
  182 on the `pylsp` one, so no slack hides it.
- Only the **shown** page has a geometry, so the test and the probe switch pages
  (`setCurrentIndex`, or `setCurrentItem` on the list) and pump events. The
  probe is `/tmp/opencode/box127/boxes.cxx` — it prints the x of every box, the
  width of the page and of the viewport, the gap between the widest name and its
  box, and whether a horizontal scrollbar appeared, at two window widths — and
  `/tmp/opencode/box127/shot.cxx` shows one theme by name for
  `import -window root`. Both load the built `.so` files themselves:
  `EditorWidget` reads no `TFEL_EDITOR_LOAD_PATH` — that is the executable's — so
  a probe which does not load them sees no `pylsp` theme, which is nine of the
  twelve boxes, and a run of three boxes reads as clean. `QLibrary::load` is **no
  static** taking a file name: `setFileName` then `load()`.
- A horizontal scrollbar on the `clangd` page of a live display, at 760 px wide,
  is **not** this change: the name `Start clangd when the project of a file has a
  compile_commands.json` is 450 px there and, with its box, the minimum of the
  page, 488 px, over what the 476 px viewport gives. The pinned column asked for
  the same minimum.

### The proofreader of a buffer: one choice, two programs (`harper-ls`, LanguageTool)

What reads the words of a buffer is a **single choice for the editor**:
`SpellingAndGrammar::Checker` (`None`, `Hunspell`, `HunspellHarper`,
`LanguageTool`) in the address `spelling/checker`, whose default is
`Hunspell` and is kept by the *absence* of the address. `SpellingAndGrammar`
(`src/SpellingAndGrammar.cxx`) is the one place which knows the choice, the
language each checker reads, and the menu which offers them; it declares the
`Spelling and grammar` theme of the configuration, the hunspell dictionary
entry included (`spelling_entries()`), since which program reads the words and
which words that reading holds to be spelled right is one decision — and
`Harper` and `LanguageTool` name that theme as their `parent`, which is how the
dialog opens them as its submenus. **`harper/launch-by-default`
is gone**: what once made a *new* buffer take a session is replaced by a
choice every buffer follows, the open ones through
`SpellingAndGrammar::applyTo(editor)`, the later ones built with it.

`LspClient` is no longer the clangd client: it gained `setServerName`/`setLanguageId`/`setConfiguration`/`setFullTextSync`
and `launch(program, args, error)`, and **`get()` and `CxxMajorMode::stopClangd` look
clangd's client up by `objectName() == "clangd"`** because a `C++` buffer now holds
two clients at once. `LspHarperSupport` (static: settings, discovery, the
`workspace/configuration` answer) and `GrammarCheckSession` (per mode instance, one
backend in use, draws the lints) are core classes, and so are `LanguageToolSupport`
(finding the jar and the runtime, the request body, the answer) and
`LanguageToolClient` (the process, the requests); the three hosting modes — `C++`,
`LaTeX`, `Markdown` — each keep one `GrammarCheckSession` member.

The menu is `SpellingAndGrammar::addToMenu(menu, whole_text_is_prose)`: the four
choices are one exclusive question (what reads the words of the buffers), each is a
**plain clickable entry** which calls `setChecker` + `applyTo` and reports
`"checking with %1"`; a choice whose program is not to be found is not offered at all.
Below the four entries stands one submenu of the languages of the choice **in use**
(`objectName` `spelling-languages`), which is built **when it is opened**, not when the
menu was constructed — so it always names the languages of the current choice, whether
the click on a choice above it happened before or after this menu was built. The choice's
languages are the installed dictionaries for hunspell, the five varieties for harper, the
codes (plus `auto`) for LanguageTool, and its entries call `setChecker` + `setLanguage` +
`applyTo` — because the leaf writes the choice it names, picking a language is picking
that checker, and a stored language of its own must never reach a buffer of another
program. `setLanguage(c, language)` writes the address of the choice named *and* the
addresses of the two others wherever they have an entry for that same variety, which is
what keeps the languages of each choice agreeing with each other. The four entries stand
in `Options > Orthograph/Grammar` (`MainWindow::updateOptionsMenu`, icon
`tools-check-spelling`) and in the menus of the three modes, all writing the same
address. **`SpellingAndGrammar` has no `changed` signal**: a test which sets the
checker must call `applyTo(editor)` itself, and a menu entry does it for the user.

**A choice with languages must not be a submenu header** (issue #131, the complaint
"Selecting LanguageTool in the menu does nothing"). Qt opens the entry above a submenu as
a popup instead of triggering it, so a checkable entry which carries the submenu of its
own languages can not be clicked — its languages are what one reaches, not the checker.
Putting each program's languages behind the entry which named it therefore made that
entry inoperative: clicking `LanguageTool (spelling and grammar)` opened the submenu of
codes, ticked nothing, left the checker unmodified, and the person was still reading
their buffer with the choice which was in use before the click. The fix which keeps both
the choice and its languages on screen is what the paragraph above describes: the four
choices as plain entries, their languages in one submenu of their own below, which names
those of the choice the setting holds. A test of the menu asserts it — no entry which
names a checker carries a submenu — and the probe which reproduces the complaint
(`/tmp/opencode/menu_probe/clickprobe.cxx`, offscreen and on `DISPLAY=:99`) reaches the
entries with `QTest::mouseClick`, which is what the mouse does, rather than with
`QAction::trigger()`, which is not: `trigger()` works on a submenu header and a real
click never does, which is why the old shape passed its test suite while doing nothing
under the hand.

`effectiveChecker(whole_text_is_prose)` is the one question the choice cannot answer on
its own: LanguageTool is handed a text and reads no source file, so a buffer whose whole
text is *not* prose is read by the choice which names hunspell — the entry is labelled
`" (text files)"` there, and `GrammarCheckSession::reportOf` says
`LanguageTool reads no code: this buffer is checked for spelling only`. Prose-ness is the
fourth argument of the `GrammarCheckSession` constructor, and
`MajorMode::grammarCheckSession()` with `readsWholeTextAsProse()` are how the menu and
`MainWindow` ask it. The off switch of the misspelled-words face is
`TextEditBase::setSpellCheckLanguage("")`, answered by
`SpellingAndGrammar::spellCheckLanguage(Checker)`: a choice which does not name hunspell
hands **no dictionary**, which is not the same thing as a checker being stopped.

Two traps of that surface, both paid for:

- **`applyTo` may not require a major mode.** It used to skip a `TextEditBase` whose mode
  was not built, which silently dropped the empty buffers of the editor: the dialog's live
  entry then reached the buffers on screen and left a `*scratch*` with the dictionary of
  the locale, which is what `TestSettings::acceptingTheDialogAppliesTheLiveEntries`
  asserts against (it got `fr_FR` where `en_GB` was written). A mode-less text edit keeps
  the language all the same, and `setMajorMode` hands that stored language to the mode it
  installs.
- **`MainWindow` must count the buffers before asking for the current one**: asking for
  the current buffer of an editor which holds none *creates* one, from the menu.

The two checkers of a buffer read with `HunspellHarper` work side by side, and nothing
coordinates them: `harper-ls` says what is wrong with the agreement of two words, hunspell
what is wrong with a word nobody spells, so a `LaTeX` buffer carries both faces
at once — the misspelled words drawn by the highlighter of the mode, the lints
drawn as the `grammar` contribution to the extra selections, which is drawn over
the words and so wins on the rare run that carries both reports. What was asked
at first, and reverted on purpose, is the opposite for *that* choice: an exclusion which
handed the text of a checked buffer to one checker at a time, per mode, through
`setSpellCheckLanguage` answering `SpellChecker` an empty language while
`grammar->isActive()` and the session's `stateChanged` re-applying
`textEdit.getSpellCheckLanguage()`. A grammar checker which knows nothing of
spelling has no business taking a dictionary away. `LanguageTool` is the exception, and
the reason it is a choice of the user and not of the mode: that program *does* check
spellings, in the language it was given, and a buffer it reads is handed an empty
dictionary. So the rule is one line of `spellCheckLanguage(Checker)` — a dictionary for
the choices which name hunspell, nothing for the two which do not — and not a per-mode
policy.

Protocol facts, all read off `harper-ls` 3.0.0 (`/tmp/opencode/harper/probe8.py` drives
it by hand, which is how each of these was found):

- started with `--stdio`; answers only `capabilities` and `serverInfo` — **no
  `textDocumentSync`**, hence `setFullTextSync(true)`: whole text on every change.
- asks `workspace/configuration`; the answer must be `{"harper-ls": {"dialect": …}}`
  (a `QJsonObject` without that key is refused by the server), and it asks again before
  every analysis, so the dialect is not frozen at the handshake.
- every lint answers severity **Hint (4)**: one face for all of them is the honest
  drawing. Codes seen: `AnA`, `SpellCheck`, `InflectedVerbAfterTo`,
  `PronounVerbAgreement`.
- `American` flags `an tests`, `to eats`, `don't` and `colour`; `British` the first
  three only — which is what `aVarietyOfEnglishChangesWhatIsFound` asserts with.
- **`publishDiagnostics` arrives in pieces**: an early answer is not the last word, and
  a test which reads the screen of the first answer asserts on a state the checker had
  not reached. `TestGrammarCheck::settledWords` samples the words until they stop
  changing (3 × 100 ms), which is what the negative assertions ("a clean word is left
  alone") lean on.

Three traps, all paid for:

- **`~GrammarCheckSession` must hand the `"grammar"` key an EMPTY list**
  (`text_edit->setExtraSelections("grammar", {})`), never `applyDiagnostics()`: the
  client is a *child* of the session, so it is still alive in the destructor body and
  still holds the answer it published — redrawing republishes a dead session's
  underlines on the text edit the next mode inherits. Verified by reverting the line:
  `stoppingTheModeTakesTheUnderlinesAway` fails.
- A `Q_OBJECT` struct needs an explicit `public:` **after the macro**: `Q_OBJECT` opens a
  section of its own and leaves the access at `private`, so the members which follow it
  are private whatever the keyword which introduced the class. A minimal case, verified:
  `struct A : QObject { Q_OBJECT A(int) {} };` fails with
  `'A::A(int)' is private within this context`, raised inside the generated `.moc` — some
  way from anything which looks like the cause.
- A test which aims the mouse at an underlined run to get its tooltip must aim at the
  **middle position** of the run, not at `cursorRect(selection_cursor)`: the rectangle of
  a cursor which holds a selection is the whole run and its right edge is the character
  the lint stops *before*, so `rect.x() + 3` of such a rectangle can be a clean spot and
  the filter answers nothing. The tip itself is the message and the rule in brackets —
  `Did you mean to spell `colour` this way? [SpellCheck]` — which is what
  `theLintsOfABufferStandUnderTheWordsTheyAreAbout` asserts on.

Three levels of evidence, and the third one is what says a face is *inked*: the tests
read the keyed selections (`TextEditBase::extraSelections()`), and
`/tmp/opencode/harper/pixel.cxx` counts the pixels of the lint color in a real window —
372 → 510 on `DISPLAY=:99`, 239 → 368 offscreen, back to the exact baseline when the
session is stopped. A lint's red is `QColor(200, 0, 0)`, chosen because clangd's error
red is `(170, 0, 0)` and the two can sit beside each other on one buffer.

### LanguageTool is a Java HTTP server, not a language server

LanguageTool 6.4 (`languagetool-server.jar`) speaks no protocol the editor speaks: it is
started as

~~~~{.bash}
java -cp <installation>/languagetool-server.jar \
     org.languagetool.server.HTTPServer --port <port>
~~~~

and asked over HTTP — `POST /v2/check` with an `application/x-www-form-urlencoded` body,
`GET /v2/languages` as the readiness probe. `LanguageToolClient` (a `QObject`, one per
`EditorWidget`, `objectName` `language-tool`) is its client and reaches the same
`GrammarCheckSession` surface as the harper backend, which is why the three hosting modes
know one `setChecker` and not two APIs. Numbers and shape facts, all verified against a
live 6.4 server (`/tmp/opencode/lt_probe/check.py` drives it by hand):

- **The process is booted by the first buffer which asks and ended by the loss of the
  last one**: `openDocument`/`closeDocument` refcount the users of a client, and a
  virtual machine with the rules of every installed language costs ~100 MB to serve
  nobody. Boot is ~1.1 s to the first answer of `GET /v2/languages` and its first check
  lands ~2.3 s after the start, which is what `SERVER_BOOT_TIMEOUT` and the waits of the
  test cases are scaled to. A process which does not answer is `terminate`d and reported
  by the port it was given.
- **Readiness is polled, every 200 ms, one probe at a time** (`ready_timer` is stopped
  while a probe is on its way). There is no handshake to wait for: the API has no
  notion of a session.
- **The API is stateless, so every change is a whole new request.** A document is sent
  500 ms after the last key (`TYPING_SETTLES_IN`) and a text longer than
  `maximumTextLength()` — 40 000 characters, which is a tenth of a second of server,
  where 60 000 is two seconds — is cut by `sliceStarts` in the last half of the piece,
  at an empty line, else at a line end; the offsets of an answer are then places in the
  piece and the place of the piece is what turns them into places of the buffer.
- **`port()` answers the setting, 0 included**: the free number a 0 stands for is taken
  in `startServer` by `listen`ing a throwaway `QTcpServer` on port 0 and reading
  `serverPort()` back, because only whoever binds the port learns it. Java is then
  started with that number. `--public` is deliberately not passed: the server stays on
  `127.0.0.1`.
- **A server which reads the settings the dialog no longer writes is ended** by
  `recheckDocuments` comparing `serverSettings()` with the `launch_settings` the process
  was started with (installation, runtime, port, Java options). The language and the two
  lists of categories are in every request body, so they need no restart and the
  documents are simply handed over again. `harper` has the same check with
  `harper_program` against `LspHarperSupport::serverExecutable()`, and neither has a
  public restart API: what makes a session stale decides inside the client.
- **The offsets of an answer are `QString` indices** (UTF-16 code units), so no
  conversion is needed; a request the server refuses is answered as **plain text** —
  `errorOf` reads its first line, which is what reaches the minibuffer.

Qt 6.2 traps of that HTTP surface, both paid for:

- **There is no `QUrl::FullyUrlEncoding`**; the encoding a form is read with is
  `QUrl::FullyEncoded`, and `requestBody` is then byte-identical to what
  `curl --data-urlencode` writes (the test asserts exactly that, with
  `QUrl::toPercentEncoding(text, {}, "/")`). The alternative reading of the content type
  turns a space into a `+`.
- **`QUrlQuery::queryItemValue` decodes with `PrettyDecoded` by default**, which leaves
  the `%0A` of an escaped text standing for the newline it came from: a test which reads
  a body back must ask for `QUrl::FullyDecoded`, or it compares a text against its own
  escaped form.

The trap of a mode which hides its markup, and the one this half of the feature really
costs: `LaTeXMajorMode::grammarCheckText` blanks `\section` and friends with **spaces of
the same length** — shortening the text would put every answer behind the cut on a word
the buffer does not hold there — and LanguageTool reads a run of blanks as a repeated
whitespace (`WHITESPACE_RULE`, category `TYPOGRAPHY`). So the editor would underline
where its own spaces are, in a place of the file of which the checker was handed nothing.
The filter is in `LanguageToolSupport::diagnosticsOf`: **a report whose named text holds
no letter and no digit is dropped**, since every report a reader wants names the word it
is about (verified: a missing space after a period is answered on the *following* word,
`an tests.It` at offset 17 and length 2, which is `It`). Filler characters which the
program treats as word characters (`_ - ~ # | *`) are silent; zero-width and soft hyphens
are worse than the blanks, they make reports on the words beside them.

And the coordinate trap of that filter: `diagnosticsOf(text, offset)` is handed the
**whole** text of the document and the place of the piece which was sent, so the named
text is `text.mid(start, length)` — `start` already carries `offset`. Writing
`text.mid(start - offset, length)` reads a report against the piece instead of the
whole text, which drops a report standing at the front of a piece (the blanks which lead
a sliced text are exactly `offset` characters long) and is a one-token change. Both
mutants are named by `aReportAboutTheBlanksOfAModeIsNotADetailOfTheBuffer`, which is the
only case of the suite needing no server for the LaTeX path: it feeds the two
`WHITESPACE_RULE` reports and one `A_NNS` report of a live server, once whole-text and
once as the answer of a piece at offset 18, and asserts one diagnostic survives on the
line the whole text draws it on.

Test wiring: `tests/CMakeLists.txt` has a `LANGUAGE_TOOL_JAR` cache filepath turned into
the `TFEL_EDITOR_LANGUAGE_TOOL_JAR` compile definition, which is what
`languageToolInstallation()` reads; without it the cases which ask the server anything
`QSKIP`, and `initTestCase` writes the address for the cases which only read the
settings. A run leaves its server running when a case is interrupted —
`pkill -f "[H]TTPServer"` between runs, and mind that a plain `pkill -f HTTPServer`
matches the `grep`/`pkill` command line itself.

Finally, the case which loads the `Markdown` plugin must stay **last** in the list of
slots: loading registers the mode for the rest of the run (`TestSettings` carries the
same rule for `castem`), and `underlinedWords` is what a text mode's lints look like to
a test — what the grammar checker leaves to hunspell is asserted on the `LaTeX` mode,
whose context menu asks the spell checker anything: the suggestions of a misspelled
word, counted before a session starts and after one answers about the same buffer. (The
`Markdown` mode answers that question too since the submenu existed, and
`tests/TestSpellCheckSuggestions.cxx` is what asks it there.)

### The suggestions of a misspelled word stand in a submenu of the context menu

A word the dictionary underlines is right-clicked into a `Suggestions` entry which leads
to one entry per correction the checker answers, a choice replacing the word. The entries
are built in **one place of the core library**, `MajorModeBase::addSpellCheckSuggestions
(m, checker, word, first, last)`, and `MajorModeBase::replaceTextOfBuffer` is the one way
that menu writes into a document. What a mode still decides for itself is only *which
part of a line is prose*:

- the `LaTeX` mode asks `LaTeXSyntaxHighlighter::parseLine(line).plainText` which
  portions of the line the highlighter reads as text, and `wordAt` (anonymous namespace
  of the mode) which run of letters of that portion the place stands in;
- the `Markdown` mode asks `MarkdownSyntaxHighlighter::spellCheckedWords(line)`, which is
  the one split its highlighter draws the underlines with, so an underlined word is never
  left unsuggested and a word which is not underlined is never suggested.

Three shape facts, and a rule about places:

- the submenu's `objectName` is `spell-suggestions`, the handle tests and whoever walks
  the menu single it out by, as `spelling-languages` and `clangd-references` are; the
  entry is inserted **in front of** the entries the menu already holds, and since
  `addSeparator` appends, the separator behind it comes back in front through
  `insertAction(entries.at(0), …)`;
- nothing is added when the word is empty, when `last <= first`, when the frame is read
  only (the freeze of a file modified on disk shows the underlines and none of the ways
  out of them), when the checker spells the word well — which is also the answer of a
  checker which was given no dictionary, so the off switch of the spell checker needs no
  test of its own — and when the dictionary answers no suggestion;
- the replacement keeps the positions it was built with: a `QMenu` is raised on a press
  and answered on a release, and nothing has moved the document in between, so the lambda
  of an entry captures `first` and `last` rather than asking the cursor where the word is;
- **a position of a document is a place between two characters**, so *both* bounds of the
  run of letters belong to the word: `p >= first && p <= last`. The click on the first
  letter of a word is answered by the place which precedes it and the click on its last
  letter by the place which follows it, and the only places which name no word are those
  between two words. The assertion which pins this is one which aims at the **last**
  character: an exclusive upper bound fails there and nowhere else.

And one conversion trap the `Markdown` mode had been living with: the TFEL `CxxTokenizer`
answers `offset` as a **byte offset into the UTF-8 encoding** of the line, while
`setFormat`, `QTextDocument` and `QString::mid` count characters, so on a note whose line
holds one accented word the underline of a misspelled word and the replacement of it were
both drawn one character behind the word. `spellCheckedWords` brings the two answers back
together through `characterAtByte`, in the one place a Markdown line is split for the
spell checker. The `LaTeX` mode never had the problem: its splitter is a
`QRegularExpression` run on the `QString` itself.

What is deliberately **not** on that menu: the lints of `harper` and LanguageTool have no
corrections to offer (`LspDiagnostic` holds the message, the rule and the range, and no
support code parses the `data` or `changes` a server could carry with them), and although
`hunspell` will add a word to a user dictionary (`SpellChecker::addToUserWordlist`, which
writes a file under the data location of the application), an entry which writes a file
behind the back of the person who owns that file is none of the editor's business.

Test traps, all paid for:

- **`QAction::trigger()` on the *menu* action emits no `triggered`**: it activates the
  submenu, which for a menu nobody showed does nothing at all, and the case reads as a
  replacement which silently failed to happen. A click reaches the leaves:
  `submenu->actions().at(0)->trigger()`.
- The menu is never raised: `QMenu::exec` at the end of `TextEditBase::handleMousePressEvent`
  is a nested event loop no offscreen test returns from, so the cases call
  `MajorMode::completeContextMenu(&menu, cursor)` directly, on a menu which already holds
  a stock entry to stand for the undo, copy and paste entries a right click puts there
  first — which is what makes "the suggestions stand in front of those" an assertion
  rather than an intention.
- What the submenu holds is compared to what a `SpellChecker` of the test's own answers
  for the same word on `en_US`: right whatever dictionary the machine happens to hold, and
  every case `QSKIP`s when that checker says nothing. `frame->setSpellCheckLanguage("en_US")`
  names the dictionary of a buffer; handing the same frame an empty language is how the
  case about holding no dictionary turns the answers off, and `setReadOnly(true)`, with the
  language left alone, is how the case about a frozen buffer freezes only the writing.
- This Qt has no `QtWidgets/QAction` header: include `<QtGui/QAction>`.
- Whole texts are compared after `replace(QChar(0x2029), QChar('\n'))`, as elsewhere in the
  suite (see the transcript of an interpreter session).

What each half of the feature is worth is named by mutating it: dropping the read-only
guard fires `aBufferWhichCanNotBeWrittenToIsOfferedNothing`, an exclusive upper bound on
the word fires the either-end case, and taking the byte offsets of the tokenizer literally
fires the `Markdown` case — the accented word in front of the misspelled one is what makes
the shift visible. `tests/TestSpellCheckSuggestions.cxx` is the seventeenth executable and
loads the `LaTeX` and `Markdown` plugins in `initTestCase`, which is harmless here because
`TestSettings`, the suite which asserts the set of configuration themes, is a separate
executable.

### Killing a buffer takes the pages of its tab widget with it (the `Ctrl-x b` segfaults)

`SecondaryTask::w` is a `QPointer<QWidget>` and `CommandLine::buffer` a
`QPointer<Buffer>`, because both outlive what they name in two ways which were
reached by hand and are pinned by `tests/TestKillBuffer.cxx`:

- **A shared secondary task is a page of one tab widget.** `QTabWidget::addTab`
  reparents the page, so when the compilation output of a `C++` buffer is shared
  with the other programming buffers, the buffer which added the last tab owns
  the widget — and that is not the one which created the task. Killing the owner
  frees the widget while the other buffers still name it, and the next
  `Buffer::refreshSecondaryTaskTabWidget` handed the freed address to `addTab`
  and died inside `QWidget::setParent`. Every reader of `w` tests it, and an
  entry whose widget is gone is inert: it matches no widget, because `taskMatches`,
  `wfind` and `operator<` all go through `.data()`. The registry is not pruned of
  such entries on purpose — pruning it from a `destroyed()` connection would
  re-enter `SecondaryTaskManager` in the middle of its own iterations.
- **A prompt which waits names the buffer it was raised in**, and the kill can be
  asked for while the user looks elsewhere, since `Ctrl-x b` does not block.
  `CommandLine::hideCompletions` reached that buffer to hide its `*completions*`
  page, which made the next typed character a use-after-free. `getBuffer()`
  answers null once the buffer is gone, and `hideCompletions`, `showCompletions`
  and the destructor then do nothing.

The guard against stranding a prompt sits in `EditorWidget::removeBuffer`, the
choke point of every kill — not in `closeCurrentBuffer`, which sees only the
current buffer and keeps its own stricter "any prompt at all" refusal and wording.
Its predicate is `CommandLine::isWaitingForAnAnswer()`, which is
`!isUserEditingFinished`: `userEditingFinished()` sets that flag **before** it
calls `treatUserInput()`, which is what lets the flows that kill a buffer from
their own answer (`SaveCurrentBufferAndKill`, `SaveBufferAndKill`,
`KillOtherBufferAndWriteFile`) go through while a prompt which merely waits is
refused; a predicate on the *presence* of an input would stop those flows dead.
`TextEditBase::KillOtherBufferAndWriteFile` holds its victim by pointer for the
same reason, and it is the only caller of `closeBuffer`.

The one behaviour change to expect: answering *another buffer is visiting this
file, proceed ?* with `y` no longer kills a buffer which holds a waiting prompt,
so the file ends up visited twice and the banner of the other buffer says so (see
"The buffer watches the file it visits").

Traps, all paid for:

- **The change is an ABI break**: `SecondaryTask` is in an exported header, so
  `cmake --build . --target TFELEditor` plus a packaged mode built against the
  old layout is a crash three frames from anything that looks like the cause, and
  a probe against it must be relinked (see the note on `SettingsGroup`).
- **How to test that a test catches the defect and not the shape of the fix.**
  Mutating the *guards* proves nothing: with the null test in
  `SecondaryTaskManager::removeBuffer` dropped, `deleteLater` on a null pointer is
  a `QWARN` and the suite stays green. What reproduces the old crash exactly, with
  no call site reverted, is a fifteen-line stand-in for `QPointer` which keeps the
  raw address and never nulls — `aKilledBufferTakesASharedOutputWithIt` then
  receives `signal 11`, which is the whole acceptance test of the fix.
- In a test of these two shapes: `removeBuffer` is **protected**, so expose it
  with `using` in a subclass as `ExposedEditorWidget` does; `getSecondaryTasks`
  takes a `Buffer *`; the buffer of the `Ctrl-x b` question is the one which is
  **current when the question is asked**, so the test makes its victim current
  first and switches away afterwards; and the wording of a refusal is seen by
  overriding `displayInformativeMessage`, which is virtual.

### The rectangle commands answer GNU Emacs, column for column

`Ctrl-X R` opens a blocking minibuffer input which waits for a single key —
`t` inserts a string on every line of the rectangle the selection covers, `k`
kills it, `y` pastes the last killed one — and the report which started this was
that a rectangle is sometimes not inserted under the current position of the
cursor. The defect was the loop which walks the lines: its guard was
`(!b.atEnd())`, so it stopped **before** the last line whenever that line was
the empty block a final newline opens, which is the normal shape of a file, and
the cursor was then placed by walking a `QTextCursor` across the separators the
rewrites had moved. What the three commands do now is driven by the **block
numbers of the two ends of the selection, both of them included** —
`getRectangle` takes those numbers and the two character columns from positions
and `QTextDocument::findBlock`, and each command walks
`findBlockByNumber(firstBlock..lastBlock)` and places its cursor at a position
built from `block.position() + column`.

The reference is GNU Emacs 27.1 driven through its own `rect.el`, and the
harness of it (`/tmp/opencode/rect2/gen.py`) is the **single source of both
sides**: `python3 gen.py el > measure.el` writes the Elisp which builds a shape,
selects the region, presses the command and prints the buffer and the point, and
`emacs -Q --batch -l measure.el > emacs.txt` runs it (`rect.el` itself is loaded
from `/tmp/opencode/rect/rect.el`); `python3 gen.py cxx > probe.cxx` writes the
C++ probe which does the same to a real `EditorWidget`, and
`./probe > editor.txt` runs it offscreen. `python3 gen.py check emacs.txt
editor.txt` prints the disagreements (76 shapes, 0 disagreements). Running it
with no argument prints the rules of the record; `python3 expect.py <name>`
prints the `QCOMPARE` lines of the shapes whose name it matches.
**Every expectation of `tests/TestRectangleMode.cxx` comes from that record,
never from reading the report by hand**: the test's `blocks()` lists *every*
block of the document, the empty last one included, while the Emacs report drops
it and says `--- final-newline: yes|no`, so a hand-copied expectation asserts a
line the document does not have. A shape is edited in `gen.py` and both reports
are regenerated; the shape lists of the harness and of the test are two
different things on purpose, the second being what a regression needs.

What Emacs says, measured rather than described (`t` = insert, `k` = kill,
`y` = paste):

- a line an end of the selection stands on **is** part of the rectangle, so a
  selection dragged down to the beginning of the next line reaches that line,
  and a buffer whose last block is empty has that block among the lines of a
  selection which reaches it;
- `t` pads a line which stops short of the left edge with blanks, keeps
  `line.mid(right)` behind the string (the line is *split*, not overwritten),
  and leaves the cursor behind the text written on the **last** line of the
  rectangle. An answer of several lines goes **whole on every line** of the
  rectangle — not one answer line per buffer line — which is why the rewrite
  runs from the bottom of the rectangle upwards: numbering of the lines above a
  rewritten line is nothing a lower line may depend on;
- `k` stores each line's piece **right-padded to the width of the rectangle**
  and never pads the line itself, so deleting what a line does not hold leaves
  that line alone. The cursor keeps **its own** line — which need not be a line
  of the rectangle at all, `k19` and `k20` — and goes back by the width of what
  was deleted in front of it, stopping at the left edge when it stood inside;
- `y` inserts at the cursor's own column, whatever the selection says; a line
  shorter than that column is padded and the piece goes **behind** the blanks;
  a line is opened only when a piece stands past a line which has no newline of
  its own, which is why pasting on the empty last line of a file which ends with
  a newline opens nothing;
- a rectangle of no width (both ends in the same column) is a command of its
  own: `t` inserts the string on every line it names; `k` takes nothing from the
  lines and stores a piece of no width per line, which `y` writes as nothing, so
  the buffer is left as it was and the paste only moves the cursor;
- **the killed rectangle does not go on the kill ring**: `kill-rectangle` of
  `rect.el` only sets `killed-rectangle`, so `Ctrl-Y` knows nothing about a
  killed rectangle and `Ctrl-X R Y` is its only way back. Whatever an earlier
  draft of this feature did with `EditorWidget::getKillRing`, it was a
  divergence from the reference.

Four Qt traps, all paid for:

- **`QTextCursor::blockNumber()` asks the layout** and is only brought up to
  date when the text is painted, so after a command which rewrote lines
  programmatically it can still name the line the cursor stood on the last time
  the buffer was drawn. The line of a cursor is `document->findBlock(position)`,
  in the mode and in the test's `cursorWhere` alike.
- `QTextDocument::findBlock(n)` reads its argument as a **position**, not a line
  number: `findBlock(1)` is the first line, `findBlockByNumber(1)` the second.
- a `QTextEdit` can not place a cursor past the end of a line at all, so the
  right edge of a rectangle is bounded by the length of the line it is drawn
  from. Emacs clamps the same way (`move-to-column`), which is why the 0
  disagreements of the harness do not hide this.
- writing on the empty last block leaves the buffer with text there and **no
  final newline**, which is what the record says of the same shape
  (`--- final-newline: no` for `t2` and `t11`): the field exists because the two
  programs had to be compared on it, and they agree. The case which names the
  defect itself is
  `theRectangleReachesTheLineTheEndOfTheSelectionStandsOn`.

Test traps:

- `RectangleMode::killedRectangle` is a **static shared by every buffer and
  nothing clears it**, so `pastingWithNoRectangleKilledSaysSo` must stay the
  first slot of `TestRectangleMode`; a case which needs an empty register
  anywhere else can not have one.
- `Ctrl-X R` is a blocking input: the `t`/`k`/`y` key must be pressed into the
  entry of that question (`liveInput` + `entryOf`, the `TestLspClient` shape —
  the entry is a `LineEdit`, i.e. a `QTextEdit` shim, not a `QLineEdit`), and
  `t` uncovers a **second** question, the string, whose answer is set with
  `setText` because a newline of a multi-line answer is the `Return` of the
  minibuffer.
- `RectangleModeLineEdit::keyPressEvent` must chain to
  `CommandLine::CustomLineEdit::keyPressEvent` for every key it does not claim
  and guard its `userEditingFinished()`: the version which did neither ended the
  command on any stray key — a space, a `z`, an arrow — with empty input.
- `QTest::keyClick(widget, int)` selects the **`char`** overload, which maps its
  argument through `QTest::asciiToKey`; key arguments are `Qt::Key` (the same
  trap `TestPythonMode` carries).

Two differences from Emacs are deliberate and are named in
`docs/web/rectangle-editing.md`, where the commands are documented for users:
the columns of a rectangle are **character** columns, not the screen columns
Emacs measures — a tab is one character here and moves to the next tab stop
there — and there is no `C-x SPC` (`rectangle-mark-mode`), so a rectangle is
always what a linear selection spans. Both show up only on lines which hold a
tab. A tab shape is deliberately **absent from the harness**: its Elisp side
places a corner with `move-to-column` and its probe side with a character
offset, and on a line which holds a tab those two answers are different
characters, so such a shape measures the convention of the harness rather than
what the command does. It is measured by hand instead
(`/tmp/opencode/rect2/tab1.el`, whose region is named by characters as the
editor names a selection), and the pair of answers the page quotes is that
measurement: on `a<TAB>Hello World` / `bbbbbbb World`, a region whose point
stands before the `o` of `Hello` and whose mark before the space of the second
line answers `a<TAB>HellXY World` / `bbbbbbXY World` here and
`a      XYo World` / `bbbbbbbXYd` in Emacs, which cuts at the screen columns 12
and 7 the same two ends name.

One pre-existing oddity was left alone: the history address of the insert
question is misspelled `/rectangle-mode/insert/histroy`, which is what a user's
settings file holds.

### Which TeX compiles a document (the `Interpreter` submenu of the `LaTeX` mode)

`latex/interpreter` names the program `Alt+M` starts: `latex`, `pdflatex` or
`xelatex`, in a theme named `latex` (the proxy fills `mode = "LaTeX"`, the
description and the icon, as it does for `clangd` and `castem`, since the entry
is about a program and not about the editor). `LaTeXMajorMode::declareSettings`
declares the one `SettingsKey::choice`; the `Interpreter` submenu of the mode
(`objectName` `latex-interpreter`, an exclusive `QActionGroup`) writes the same
address and reports `compiling with %1`. The address is read by
`chosenInterpreter()` **on every compile**, so a choice reaches the next compile
with no restart, and a value which is none of the three answers `latex` — the
program the mode ran before there was a choice to make, which is also the
declared default and the answer of a configuration file which says nothing.
`Alt+M` is spelled `Alt+M` in the strings of the tree and in the docs, not
`Alt-m`.

What the three engines are, measured on this machine (`/tmp/opencode/latexq`):

- **The quoted `"\input"` in front of the file name is a `pdftex` trick and it
  breaks `xelatex`**: handed the argument `"\input"`, XeTeX looks for a file
  named `"'` and stops (`! I can't find file `'"'.'`, exit 1, "No pages of
  output"). The file name alone reaches all three — absolute or relative, with
  or without the extension, with or without a space in it — so the arguments are
  now `-file-line-error -interaction=nonstopmode <file>`. The `-file-line-error`
  option is what keeps the report of the three in the one shape the
  `latex-output` mode of the frame reads; they are web2c programs.
- `latex` writes a DVI, the other two a PDF, which is the sentence each entry of
  the submenu and each tool tip of the combo box says.

Traps, all paid for:

- **A checkable entry of a `QMenu` is not ticked by the menu**: nothing sets
  `checked` unless the code does. Drawing the tick only from `aboutToShow` leaves
  a submenu whose entries are all unchecked until somebody opens it, and a test
  which asks the freshly built menu which engine it names finds none. `redrawTick`
  is called once when the submenu is built and again on every `aboutToShow` (the
  dialog, another buffer's menu, or a hand in the configuration file answer the
  question in between).
- **`QProcess::terminate()` of a child which never left `Starting` is handled by
  the test harness, not by the program it is aimed at.** `strace -f` names it
  (`/tmp/opencode/latexq/probe4.cxx`, the shape of `startLaTeX` reproduced: a
  `QProcess` with a parent, merged channels, a working directory, no write): the
  call is `kill(child, SIGTERM)` and the child answers `SI_USER, si_pid=parent` —
  but the child has not `exec`ed yet, so it still runs the code and the signal
  handlers of its parent. Qt Test's handler therefore wakes up *inside the child*
  and prints the whole confusion: `*** Received signal 15`, a stack trace,
  `QFATAL : TestLaTeXMode::aCompileRunsTheEngineChosenAndNoOther() Received signal
  15`, `FAIL! ... Received a fatal error` — while the parent finishes the case and
  reports `PASS`, which is why a run can show a failure and a success for one and
  the same case (`probe4` prints it and then keeps running: its second `start` on
  the same `QProcess` succeeds). What ends a process of a test is the compile
  itself — `waitForFinished` — and `terminate` is reached only by a state the
  process really is in.
- **The frame of a compile is reused, and a reused `QProcess` names the last
  engine it started**: `startLaTeX` keeps `po` as a member and calls `start` again
  on it, so a test which asserts the engine of the *second* compile must see the
  first process end first, or `program()` answers what the previous compile ran
  whatever the second one did.
- **A frame a compile attaches is found among the secondary tasks, not as the
  current one**: `startLaTeX` calls `attachSecondaryTask` and never shows it, so
  `Buffer::getCurrentSecondaryTask()` answers nothing. What works is
  `EditorWidget::getSecondaryTasks(&buffer)` and
  `qobject_cast<ProcessOutputFrame *>(t.w.data())`, keyed on
  `getProcess().program()`.
- **`ctest` runs the binary which is on disk**: after a compile which failed, the
  stale executable happily passes. Grep the build output for `error:` — and note
  that `cmake --build . | grep error` answers exit 0 whatever it found, so the
  build must be checked for what it printed, not for its status.
- `test-latex-mode` is the fifteenth executable and loads the built `LaTeX`
  plugin in `initTestCase` (`TFEL_EDITOR_LATEX_MODE`, `$<TARGET_FILE:...>` plus
  `add_dependencies`), which registers the `latex` theme for the rest of *that*
  executable's run only; `TestSettings`, which asserts the set of themes, is a
  separate executable and is untouched.
- A helper which answers something cannot use `QVERIFY` (it expands to a bare
  `return`): the failure of `makeBuffer` and `frameRunning` is a null pointer the
  caller asserts on.

### Finding and replacing: one search primitive, three surfaces

`Edit > Find/Replace...`, `Alt-%` and `Ctrl-Alt-%` all walk a buffer looking
for a text, and all three now ask the **same object** what a match is:
`SearchPattern` (`src/SearchPattern.cxx`) holds the pattern, the three options
(`Match case`, `Whole words`, `Regular expression`), the replacement and the
compiled expression, and answers `find(document, from, backward)` and
`startingAt(document, position)`. Its positions are places of the document
*and* of `document.toPlainText()` — the same numbers, because a block separator
is one character of each and no block holds a character `toPlainText` would
write as more than one, the invariant `normalizeLineSeparators` keeps. That is
what lets a `QRegularExpression` be matched against the whole text and its
answer applied to the document with no conversion.

The three surfaces are: the modeless `FindReplaceDialog` (`showFor(editor)`
reuses the one dialog of the window and prefills it from the buffer on screen,
`objectName` `find-replace`, its widgets `find-field`, `replace-field`,
`match-case`, `whole-words`, `regular-expression`, `find-next`,
`find-previous`, `replace`, `replace-all`, `close`, and `status` for the line
it reports on); and the two minibuffer walks of `TextEditQueryReplace`, which
differ in one flag, `regexp`. The dialog reports in **its own** `QLabel` and
never in the minibuffer: it is modeless, so a message there would land under a
hand on a button, and the minibuffer belongs to whoever is waiting for an answer.

Numbers and rules, all measured:

- **`QTextDocument::find` is case-INsensitive unless `FindCaseSensitively` is
  passed** — the option's absence is not "as written". Verified: `abc` is found
  in `ABC` by default and not found with the flag (`/tmp/opencode/frd/qtsemantics`).
- **The whole-word rule is `isLetterOrNumber`, the underscore included as a
  *boundary*, which is Qt's rule and is not `\b`.** The rule is `standsAlone`
  and it is used for the plain and the expression path alike, because
  `QRegularExpression`'s `\w` counts `_` as a word character and `\b` would
  therefore answer a different question from the one `FindWholeWords` asks.
  Measured on `abc_def abc` (`/tmp/opencode/frd/word.cxx`): Qt's
  `FindWholeWords` answers `0..3` — the `_` is a boundary, so the leading `abc`
  is a whole word — while `(?:\b)abc(?:\b)` answers `8..11`, the *second* one;
  and on a document which is `abc` alone, `\b…\b` finds **nothing** where Qt
  finds `0..3`. A `(?<![\w])…(?![\w])` lookaround is wrong the same way. The
  parity between the hand-rolled rule and Qt's is asserted by
  `TestSearchPattern`.
- **An empty match is never returned**: a pattern like `^[[:space:]]*$` matches
  at a place with no character, and a replacement of nothing is not what a
  reader means; `findNextMatch` retries one character further, which is also how
  a match the whole-words rule rejects is passed over.
- **`\0`–`\9` in a replacement, one digit only**: `\12` is group 1 then a `2`
  (two-digit group numbers are not what QRegularExpression's own `\12` would
  mean here); a backslash before anything else stands for that character, so
  `\\1` writes `\1` and a trailing backslash writes nothing; a group an
  alternative left unfilled writes nothing. The plain command expands **nothing**:
  its replacement is the text it was given, backslashes included.

Four traps, all paid for:

- **`QTest::keyClick` sends the *press* of the modifier before the key of the
  chord**, so `Ctrl-G` reaches the walk of the query replace as a bare
  `Qt::Key_Control` press first. `TextEditReplaceFilter::filterKeyPressEvent`
  consumed every key and answered the stray ones with `type either 'y', 'n' or
  '!'`, which is how the reader was told they were wrong at the moment they
  asked to stop. The filter now returns early on a bare Shift/Ctrl/Alt/Meta
  press, the rule `handleKeyPressEventWithEmacsShortCuts` already had.
- **Offscreen, `QTest::keyClick(…, Qt::Key_Percent, Qt::AltModifier)` adds no
  Shift**, nor any for `Qt::Key_Exclam`: the text of the event is `%` and `!`.
  The `Ctrl-Alt-%` branch therefore accepts the modifier pair with and without
  `Qt::ShiftModifier`, which is the US-layout spelling where `%` needs the
  shift; a test which passed only the shifted form would answer nothing at all.
- **`CommandLine::setLabel` stores `"<font color=blue> " + label + " </font>"`**,
  so a test which asks what a question reads must use `contains`, never equality
  on the text of the prompt.
- **A position on a line boundary belongs to the block behind it**: after a
  replacement, the walk's selection of `8..11` in `abc abc\nabc` is reported by
  `document->findBlock(selectionStart()).blockNumber()` as line **1**, not 0.
  A test which names where the walk stands asserts that reading.
- **A read-only text edit stops a key and nothing else**: a `QTextCursor` of a
  program writes straight through it (measured, `/tmp/opencode/frd/rocheck.cxx`:
  `insertText` on a `QPlainTextEdit` with `setReadOnly(true)` changes the
  document). The freeze of a file modified on disk is therefore only as wide as
  the `isReadOnly()` gate each writing command asks for itself — which the
  dialog's two buttons which write and the two query replaces now do, each
  reported `read only buffer`, and `aBufferWhichIsFrozenIsRefusedBothCommands`
  and `aBufferWhichCanNotBeWrittenToIsRefusedBothButtonsWhichWrite` are what
  say so. The two buttons which only search are answered: walking a frozen
  buffer writes nothing.
- **No replacement is the plural of English**: the report of a walk is built on
  `nb == 1`, not on `nb < 2`, which answered a walk ended before the first `y`
  with "replaced 0 occurrence" — a line a reader who skimmed it took for a
  success. The dialog already read `count == 1` the same way.

`RectangleEditing` (`src/RectangleEditing.cxx`) is the same story on the other
axis: the four rectangle operations moved out of `RectangleMode` — which is a
`CommandLine`, and a `Q_OBJECT` change of its layout would be an ABI break for
every packaged mode — into an exported non-`Q_OBJECT` struct of statics which
`RectangleMode` delegates to and the three entries of the `Rectangle Edit`
submenu of `MainWindow`'s `Edit` menu and the commands `cut-rectangle`,
`copy-rectangle` and `paste-rectangle` call directly, without the `Ctrl-X R`
question in the way. `kill-rectangle` — the name GNU Emacs gives the first of
them — is an **alias**, which `CommandFactory` has no notion of: an alias is a
second `StandardFunctionCommandProxy` of the same function under another name,
and since `Alt-X` completion lists `getAvailableCommandsNames()`, the alias name
shows up there, which is the point of registering it. `addCommand` keeps the
**first** of two clashing names and says so only through `qDebug`, so an alias
must be registered under a name nothing else claims.
`copyRectangleAsKill` is
`C-x r w`: the same register as the kill, the buffer and the cursor left alone,
and the kill ring untouched — `copy-rectangle-as-kill` of `rect.el` sets
`killed-rectangle` and nothing else, which `TestRectangleMode` asserts by
pasting the register a copy wrote and reading the document Emacs answers with it
(`/tmp/opencode/frd/copyrect.el`, GNU Emacs 27.1: the register of
`abcdefgh`/`ab`/`cdefgh` over columns 2..9 is `("cdef" "    " "efgh")`).
`yankRectangle` **inserts** and never replaces the selection: `yank-rectangle`
inserts, and the command which replaces a selection is `clear-rectangle`. The
three commands report nothing in the minibuffer, so the case which pins the
kill ring being untouched asserts the ring, not a message.

The three new suites are `test-search-pattern` (the primitive, no GUI),
`test-find-replace-dialog` and `test-query-replace`; the rectangle register and
`C-x r w` are cases of `test-rectangle-mode`, which must keep
`pastingWithNoRectangleKilledSaysSo` first because the register is one static
nothing clears.

### The file explorer panel and the `Display` menu

The panel beside the buffers is a `QTreeView` of a `QFileSystemModel`, in a
class of the core library (`FileExplorer`, exported, `Q_OBJECT`, so a test
reaches it with `findChild`). It is a **child of `EditorWidget`, not of the
window**: the editor's vertical layout holds **three rows** — a horizontal
`QSplitter` of `[panel, buffers]`, then the information line of the buffer on
the screen, then the minibuffer — so the panel is as tall as the text of the
buffer and ends **above the mode line, the informative message area and the line
the user types in**, all three of which keep the whole width of the window.
`centralWidget()` therefore answers the editor and the height walk of issue #121
runs the same chain it ran before the panel existed — neither the splitter nor
the row of lines is a parent of the minibuffer on the way up. `MainWindow` keeps
only the menu and delegates to `EditorWidget::setExplorerPanelVisible` /
`isExplorerPanelVisible`.

That shape is a correction, twice over: the first version split the *window*
between the panel and the editor, which cut the informative bar to two thirds of
the window and made a long report read as truncated; the second left the mode
line inside the buffer, so the line which names the file on the screen stopped
at the edge of the panel and named a buffer across only two thirds of the window
while the panel ran all the way down beside it. A `QDockWidget` was rejected
too: it brings a title bar, a float and a close button of its own, which is not
the shape of this window.

`request.md` names `QFileModelView`: **Qt has no such class**. The pair which
shows the file system through a model is `QFileSystemModel` — since Qt 6 in the
**`QtGui`** module, not in `QtWidgets` where its forwarding header used to live
(`#include <QtGui/QFileSystemModel>`) — and one of the views of `QtWidgets`.

What the panel is, and what the splitter keeps:

- Built by the constructor of the editor and **hidden**; the first show roots it
  at the directory of the file the current buffer visits (`QDir::current()` when
  it visits none, the choice `Ctrl-X Ctrl-F` makes) and gives it the width the
  address `explorer/panel-width` holds, a quarter of the editor when it names
  nothing. Later shows in the same window change nothing, because the splitter
  remembers the width a hidden child was last given: a drag to 400 px comes back
  at 398 px.
- **Both the state and the width survive the window** through two plain
  `QSettings` addresses — `explorer/panel-shown` (the absence is the default
  "closed") and `explorer/panel-width` — deliberately **not declared entries**
  of the configuration dialog (`TestSettings` asserts their absence): what the
  last session left open is state, not something one configures. The editor's
  constructor reads the first into `explorer_restore_pending`, and
  `EditorWidget::showEvent` — new — spends it on the **first show**: asked
  earlier, the splitter still answers the stale 640 px of an unshown window
  (`/tmp/opencode/explorer/restoretiming.cxx`), and a `QTimer::singleShot(0)`
  from the constructor fires before the window shows. A saved width is used only
  within `40 <= w <= total - 100`, the quarter otherwise.
- **`QSplitter::splitterMoved` is drag-only**: it fires 0 times for a
  programmatic `setSizes`, a window resize, and a child's hide or show
  (`/tmp/opencode/explorer/splittermoved.cxx`), so it cannot keep the width
  address current while the hand drags. The width is written at the three
  moments the panel provably has one: the first show (the applied width), a hide
  (what the handle was left at), and `~EditorWidget` when the panel is open.
  The predicate at the last two is **`isHidden()`, not `isVisible()`**: by the
  time the destructor runs, the window which made the panel visible is being
  destroyed, `isVisible` answers false whatever the user left open, and the
  close silently saves nothing (measured: the next window restored the first
  show's quarter instead of the dragged 350 px).
- A probe which triggers the menu entry **before the first `show()`** computes
  the quarter — or restores the saved width — against the stale splitter, and
  `QSplitter`'s slack redistribution then draws a wide panel: no hand can do
  that, so `shot.cxx` shows after `show()` and clears the two addresses first.
- **`QSplitter::setSizes` resizes the splitter itself** to the sum it is handed,
  so the two sizes given add up to `explorerSplitter->width()`; a `QMainWindow`
  layout wins over an oversized call (measured: `setSizes({100, 2000})` left the
  window at 901 px and the panel squeezed to its 43 px minimum).
- **The members `EditorWidget` and `Buffer` gained are an ABI break**: a
  packaged mode built against the old layout reads the minibuffer pointers out
  of the wrong places, so the whole tree is rebuilt, not `--target TFELEditor`
  (the rule the note on `SettingsGroup` carries).
- The panel wears `font/textedit` from its constructor and is handed the new font
  by `EditorWidget::changeFont`, like every other widget which read the setting
  itself; `MainWindow::selectFont` has no special case for it any more.

Measured on both platforms (panel shown, `/tmp/opencode/explorer/geom.cxx`): the
row of the panel and the buffers is 649 px tall offscreen and 643 px on Xvfb +
metacity, the panel 249 px offscreen and 248 px on the live display of the
editor's 1000 px (a quarter, ratio ~0.249), the handle 4 px, the buffer then
747 px wide at x=253; below that row the
information line is 1000x14 offscreen and 1000x16 on the live display — exactly
what its labels ask, which is what a top-level layout with zeroed contents
margins gives (see the section on `Buffer::infoLine`) — and the minibuffer is
1000x18 and 1000x20, each beginning exactly where the row above it ends. The
total is what the buffer and the minibuffer were before the line was hoisted
out of the buffer (663+18 offscreen, 659+20 on the live display): the hoist
moved the line, it did not grow it. The ratio is taken on the width of the
**editor**, which is the same number on both platforms; the window is not
(metacity's own decoration moves the client area, and `w.width()` is the client
width).

Four facts of `QFileSystemModel`, all measured in `/tmp/opencode/explorer/`
(`sort.cxx`, `filter.cxx`, `dclick.cxx`, `tasks.cxx`):

- **The model sorts by itself**: directories before files, both in alphabetical
  order, whether the view asks for sorting or not. **No
  `QSortFilterProxyModel` is needed.**
- **`setRootPath` is mandatory**: an index asked for a directory the model was
  never rooted at is *valid* and has **no row**, which reads as an empty
  directory rather than as a model which looked at nothing.
  `setRootDirectory` is the one place the path of the model and the root index of
  the view are both set, and it refuses an empty name and a name which is no
  directory: a panel which went blank on a typo is worse than one which stayed
  where it was.
- **The filter decides what a hidden file is**: `QDir::AllEntries` names no
  hidden entries, so with `AllEntries | NoDotAndDotDot` a directory of
  `visible.txt`, `.hidden` and `Beta` answers `Beta visible.txt`; adding
  `QDir::Hidden` answers `Beta .hidden visible.txt`. The two dot entries are what
  `NoDotAndDotDot` is for — the panel has a button of its own for the parent.
- **The listing of a directory arrives through a socket notifier**, which
  `QCoreApplication::processEvents()` never expires: a test waits with
  `QTest::qWait` (the same trap a pipe carries). `isDirectoryLoaded` **does not
  exist in this Qt 6.2.4**, so the wait is on `rowCount(root) > 0`.

The double click is the requested gesture, and the shape of it is:
`FileExplorer::openFileAt` refuses an invalid index and a directory, and the view's
own `expandsOnDoubleClick` (left at its default) does the expanding of a directory.
**A double click on the *name* of a directory expands it just as the branch
decoration does** — measured both ways, label centre and decoration.

Two test traps, both paid for:

- **`QTest::mouseDClick` alone reaches nothing in an inactive window**, which is
  what every window of an offscreen run is: the file stays unopened and the
  directory unexpanded, with no warning of any kind. A `QTest::mouseClick` at the
  same place first — the press a real double click always begins with — makes both
  the open and the expand happen, offscreen and on `DISPLAY=:99`. `setFocus` and
  `QApplication::setActiveWindow` do **not** substitute for it.
- `QTreeView::visualRect` of a row the view has not laid out is an **empty**
  rectangle, and a click at its centre is a click on the blank part of the
  viewport; the wait is on `height() > 0`.
- And outside the panel: **`Buffer::attachSecondaryTask(QWidget *)` does nothing
  at all, silently, for a widget the registry does not already hold** — that is the
  overload a mode re-shows a registered task with. A test which wants a page on a
  buffer uses the title form, `attachSecondaryTask("*output*", frame)`, which
  registers it, adds the tab and shows it.

The `Display` menu stands between `Edit` and `Buffers` and holds two checkable
entries, neither a submenu (an entry which carries a submenu can not be clicked —
the trap issue #131 names for the spelling menu). Two conventions of this window
apply to it: **the titles of the menu bar carry no `&` mnemonic** (`tr("File")`,
`tr("Edit")`, `tr("Display")`, …), and **nothing ticks a checkable entry but the
code**, so `updateDisplayMenu` runs when the menu is built and on `aboutToShow`;
`Ctrl-X 1` hiding the tasks is a transition the entry must survive telling the
truth about. There is **no `Alt-X` command** for either entry, which the panel
being a child of the editor no longer prevents — a `StandardFunctionCommandProxy`
of a free `void showExplorerPanel(EditorWidget &)` would do it — but the request
named the menu and every command is one more name in the `Alt-X` list.

`tests/TestFileExplorer.cxx` is the twentieth executable: the menu between
`Edit` and `Buffers` and its two entries, the panel hidden until the entry is
triggered and the buffers keeping the whole width until then, the quarter of the
editor a shown panel takes, the mode line and the message line it ends above
keeping that whole width, a dragged width kept across a hide and a show, the
width and the open state coming back to a second window — through a hide for the
first, through the close for both —, the ticks of both entries, the root of the
panel and `goUp`, the buffer change which
leaves the root and the width where they are, a double click which opens a file
(and reuses the buffer of a file already visited), a double click on a directory
which opens nothing, and the auxiliary tasks entry hiding and showing a frame.
The two addresses are removed in `initTestCase` and after **every** case: the
conf file outlives the run, and a session left open would open every later
window with the panel — which is exactly the state half the cases are about.
A case which needs the tick of an entry whose panel the *editor* restored on its
own asks `aboutToShow` first: the restoration is not a trigger, and the tick of
the entry follows the screen when the menu opens.
Geometry assertions of it hold on both platforms (offscreen and Xvfb +
metacity). Four traps of measuring that geometry:

- The splitter of the panel is `ex->parentWidget()`, **not**
  `w.findChild<QSplitter *>()`: a `Buffer` holds a splitter of its own between its
  main frame and the pages of its secondary tasks, and whatever `findChild`
  happens to reach first is an accident of depth.
- The three rows of the editor are reached by index: the splitter of the panel
  is `ed->layout()->itemAt(0)->widget()`, the row of information lines
  `itemAt(1)`, the minibuffer `itemAt(2)`. A subclass exposing the protected
  members of `EditorWidget` — the `ExposedEditorWidget` idiom — is no help
  here: `MainWindow` builds the editor itself, so the object on the screen is
  not of that subclass. What distinguishes the two rows under the splitter
  without a member is what they hold: the line of labels holds a `QLabel` and
  no `LineEdit`, the minibuffer is the row which holds the read-only
  `LineEdit` of the messages.
- Two widgets whose parents differ have nothing to compare: the geometry of the
  panel is in the coordinates of the splitter and the geometry of the line
  below it in those of that line. What "the panel ends above the line" means
  is then `ex->mapTo(ed, QPoint(0, ex->height())).y() == line->mapTo(ed,
  QPoint(0, 0)).y()`, both edges in the coordinates of the editor.
- The width the panel takes is measured on the buffer
  (`ed->getCurrentBuffer().getMainFrame().width()`), never on the editor: the
  editor's own width is what showing the panel must *not* change, so it is the
  control of the assertion, not its subject.

`docs/web/file-explorer.md` is the user-facing page, with a screenshot of a
shown panel, the mode line of the buffer below it and a message in the line
under that — the one evidence that **only the name column is drawn**, since
`hideColumn` of the four others is invisible to every selection API, and the
look at what the panel leaves both lines alone with.

### The mode line of a buffer is a page of the editor (`Buffer::infoLine`)

The row of labels which names the buffer, says where the cursor stands and shows
the icon and name of the major mode and the time (`bni`, `rpi`, `api`, `ii`,
`mi`, `ti`) is a widget of its own, `Buffer::infoLine`, built in the **member
initializer list and declared above the main frame** — the same trap issue #121
names for the banner pointers, since a pointer declared lower is not initialized
yet while the `PlainTextEdit` is being built. `EditorWidget::insertBuffer` puts
the buffer into `buffers` and its line into `infos`, one `QStackedWidget` per
row, in the one call and in the same order, which is what lets
`setCurrentBuffer` show the line of the buffer it makes current by index:
`infos->setCurrentIndex(buffers->indexOf(b))`. `removeBuffer` takes the page out
of `infos` and `deleteLater`s it, because `addWidget` made the stack the parent
of a widget the buffer used to own — a line left behind keeps naming a killed
buffer under whatever comes next (`aKilledBufferTakesItsInformationLineWithIt`,
and `/tmp/opencode/explorer/lines.cxx` outside the suite).

Why one line per buffer rather than the single shared line `GNU Emacs` draws:
the drawing is `Buffer::updatePosition`, `updateDate` and
`updateBufferInformations`, each of which writes the labels of *its* buffer on a
timer and on a signal, and a shared line would have to ask every one of them
whether its buffer is the main frame, the trap the file-on-disk banner had to
avoid with `isMainFrame()`. As it is, the labels are children of the line and
nothing about the drawing changed but their parent — `Buffer::changeFont`
reaches them by the same explicit `setFont` it always used, and the four
functions which draw them (`updatePosition`, `updateDate`,
`updateBufferInformations`, `changeFont`) are otherwise untouched, as is the
rest of the tree: outside `Buffer.cxx` and the header which declares them, no
file names one of the six. `getInformationLine()` is **not virtual**: the
vtable of an exported class is not ours to shuffle.

Two shapes of the row are load-bearing:

- `infos` wears the **vertical policy of the minibuffer** (`Maximum`, as tall as
  its tallest label, never taller): with a `Preferred` policy the row and the
  minibuffer would share with the text the height the window gave to that
  text, and `installingAUserInputLeavesTheEditorHeightToTheWindow` accounts
  for the row for the same reason — a growing input takes its second line out
  of the buffers, as it did before the row existed.
- the labels have a `Maximum` **horizontal** policy, which inside the 747 px of
  a buffer next to a panel could not be relieved and clamped the row; at the
  whole width of the window there is always room for what they name.
- the layout of the line has **zeroed contents margins**, because installed on
  a widget it is a *top-level* layout and takes the style's defaults — 9 px top
  and bottom, so a 14 px line of labels would stand in a 32 px row and the
  text would lose a line of height to nothing. The `setContentsMargins` of the
  **widget** does not help: a layout with unset margins reads the style, not
  the widget. Nested in the buffer with `addLayout`, the same layout had no
  margins at all — this is the whole of why the hoist first came out taller.
  `informationLineStandsUnderTheBuffersAndNamesThem` asserts the row's height
  equals the tallest label's `sizeHint` (±2 px), which a dropped
  `setContentsMargins` fires on both platforms.

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
- **`clang-format` (the installed 14) joins a Doxygen block into one paragraph**:
  the `\brief` line and the `\param`/`\note` lines under it come back as a single
  run of prose, because `ReflowComments` is on by default and the tree's
  `.clang-format` does not set it. The committed files of the recent issues keep
  those blocks on their own lines and are not clean under the tool either —
  `src/GrammarCheckSession.cxx` differs by 148 lines, `tests/TestGrammarCheck.cxx`
  by 328 — so running it on a new file gives a file whose comments have a shape the
  rest of the branch does not use. Hand-wrap the comments, keep the code lines
  within 80 columns, and read a wrap back: wrapping the first argument of a
  `connection->request(…)` once gave the call a JSON object where its method name
  belongs, which the compiler caught and the eye had not.
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
