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
| `test-lsp-protocol`       | the shapes of the Language Server Protocol the client reads: framing, positions as UTF-16 code units, file uris, the answers of semantic tokens, document symbols and workspace edits |
| `test-lsp-client`         | a real `clangd` session: the start and the handshake, the menu of the `C++` mode, the session a file of a project starts on its own and the setting which turns it off, the jumps — `Alt+.` and `Ctrl-Left click` —, the diagnostics, the rename, the signature help, the quick fixes, the compilation database the session is started with |
| `test-grammar-check`      | the proofreader a buffer is read with: the four choices and the menu which offers them, the languages of each of them, a real `harper-ls` session and a real LanguageTool server, the lints and the face they are drawn in, what a mode which is gone leaves behind, the spell checker going off a text LanguageTool reads, and the reports the blanked markup of a mode must not produce |
| `test-file-on-disk`       | the buffer which watches the file it visits: what a change on the disk is, the banner which says it, dismissing it, re-adopting the file, the two answers of the overwrite question, and the buffer frozen while the banner stands |
| `test-settings`           | the entries of the configuration: what is declared, what the dialog shows, how it keeps the themes of the major modes apart, what writing it stores and what it applies live, what a packaged mode loaded as a plugin exports |
| `test-castem-mode`        | the `Cast3M` mode: the program the configuration names, which is the one the session starts and the notice asks about, and the line which starts the solver reaching it |
| `test-shared-compilation-output` | the output page the programming buffers share, and the line of a prompt the output of a command continues |
| `test-kill-buffer`        | killing a buffer: the shared output page it takes with it, and the minibuffer question which named the buffer that is gone |
| `test-python-mode`        | the `Python` mode: the faces of its scanner, the indentation levels `python.el` walks a line through, the region keys, the interpreter session and the prompts of its transcript, the tools of the language the menu offers |
| `test-latex-mode`         | the program a document of `LaTeX` is compiled with: the entry of the configuration which names it, the `Interpreter` submenu which writes the same address and the tick which names the engine in use, and the process a compile of the buffer starts |
| `test-rectangle-mode`     | the rectangle commands of `Ctrl-X R`, every expectation of which comes from a record GNU Emacs itself wrote, and the register `C-x r w` keeps without taking the rectangle from the buffer |
| `test-search-pattern`     | the search the three ways of replacing share: what the three options answer, the whole-word rule held against `QTextDocument::FindWholeWords`, the two directions, and what `\0`, `\1` and a backslash which means nothing are written as |
| `test-find-replace-dialog` | the dialog of `Edit > Find/Replace...`: the one dialog a window holds, what it starts from, the two directions and the turn at an end of the buffer, the counts and the reports, the three options, the buffer it follows, and the menu entry and the command which reach it |
| `test-query-replace`      | the two walks of the minibuffer, `Alt-%` and `Ctrl-Alt-%`: the questions they ask, the same characters read as a text by one and as an expression by the other, the groups of a replacement, an expression which does not compile, the keys of the walk and the two histories |
| `test-file-explorer`      | the panel of the file system and the `Display` menu: the menu between `Edit` and `Buffers` and its two entries, the panel hidden until it is asked for, its third of the window and the width a drag gave it, the ticks which name what is on the screen, the directory it is rooted at, and the double click which opens a file or a directory |
| `test-spell-check-suggestions` | the `Suggestions` entry of the context menu of a misspelled word: the submenu of the corrections the dictionary answers, which of two words of a line the place of the click names, and the replacement a choice makes, asked of the `LaTeX` and `Markdown` modes as the editor loads them |

These encode the invariants fixed by
[issue #121](https://github.com/thelfer/tfel-editor/issues/121);
the infrastructure itself comes from
[issue #123](https://github.com/thelfer/tfel-editor/issues/123), and the
test of the configuration is that of
[issue #127](https://github.com/thelfer/tfel-editor/issues/127).
Adding a test is a matter of adding a `.cxx` to `tests/CMakeLists.txt`.

A test which writes entries of the configuration points `QSettings` at a
file of its own before the first case and forgets the addresses it knows
about at the end of each of them: the dialog of the configuration reads
and writes what the editor itself does, and a test must leave the
configuration of the user alone.
