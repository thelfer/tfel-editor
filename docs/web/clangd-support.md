This page describes the `C`/`C++` support of `tfel-editor`, which is
based on a [`clangd`](https://clangd.llvm.org) language server.

When a `C` or `C++` file is opened, `tfel-editor` acts as a
[`Language Server Protocol`](https://microsoft.github.io/language-server-protocol/)
client: it starts `clangd`, keeps it in sync with the buffers that are
edited, and turns what the server sends back into editor behaviour.
The editor itself embeds no `C++` parser: everything smart about `C`
and `C++` files comes from `clangd`.

# Main features

- **Diagnostics**: errors, warnings, notes and hints reported by
  `clangd` are underlined in the text, with a color set by their
  severity: red for an error, orange for a warning, blue for a note and
  gray for a hint. The text of the message is not displayed yet: an
  underline says where a problem is and its color how serious it is.
- **Completion**: completion of names, members, keywords and macros,
  computed by `clangd` on the text as it is being edited.
- **Go to definition**: jump to the line where the symbol under the
  cursor is defined, opening the file which holds it if needed.
- **Hover**: a tooltip showing the declaration of the symbol under the
  cursor and its documentation, if any.

Completion, navigation and hover wait at most three seconds for the
server: a server which does not answer is reported in the minibuffer
instead of freezing the editor.

The text is sent to the server as it is modified, so the diagnostics,
the candidates and the hover of a buffer reflect what is on the screen,
not what was last saved on disk.

# Getting `clangd`

The `clangd` binary is **not** part of `tfel-editor`: it is delivered
by the `LLVM` project and must be installed separately.

- On `Debian`/`Ubuntu`, the `clangd` package installs the plain name:

~~~~{.bash}
sudo apt install clangd
~~~~

  The versioned `clang-tools-NN` packages install `clangd-NN` only, with
  no plain `clangd` in the `PATH`, which the search of `tfel-editor`
  handles too.

- On `Fedora`, it is provided by `clang-tools-extra`:

~~~~{.bash}
sudo dnf install clang-tools-extra
~~~~

`clangd` is also shipped by the official `LLVM` binary distributions
and by most `conda` toolchains.

If `clangd` is in the `PATH`, nothing has to be configured: `tfel-editor`
starts it when the first `C`/`C++` file of a window is opened and keeps
it alive for the rest of the session. The search, in this order:

- the `clangd/executable` option, if it is set;
- `clangd` in the `PATH`;
- the highest versioned `clangd-NN` of `/usr/bin`, `/usr/local/bin` and
  `/bin`;
- the highest versioned `/usr/lib/llvm-NN/bin/clangd` of the `LLVM`
  installations, which is where the `Debian` packages put the binary.

If `clangd` cannot be found, the support stays silent about it: the
`C`/`C++` buffers work as before, without the diagnostics and the
completion, and the reason is displayed in the minibuffer the first
time a request is made.

The mode covers the extensions the `C++` major mode is registered for:
`.cxx`, `.hxx`, `.ixx`, `.cpp`, `.hpp`, `.ipp`, `.CC`, `.hh`, `.c`, `.h`
and `.cu`.

# Shortcuts

The following shortcuts are active in `C`/`C++` buffers, in addition to
the [usual `emacs` shortcuts](emacs-shortcuts.html) of the editor:

- `Alt+Tab`: complete the name at the cursor. The candidates returned
  by `clangd` are displayed in a completion popup: keep typing to
  narrow the list and insert the selected candidate with `Tab`, which
  replaces the whole word under the cursor, as `clangd` completes
  fuzzily. `Ctrl-G`, the `quit` shortcut of the editor, closes the
  popup.
- `Alt+.`: go to the definition of the symbol under the cursor. If the
  definition is in the current buffer, the cursor jumps to it,
  otherwise the file which holds it is opened, reused if it is visited
  already, and the cursor is placed on the defining line. There is no
  jump back: use `Ctrl-X+B` (change buffer) to go back to the buffer one
  left, as always in the editor.
- `Alt+?`: display the declaration of the symbol under the cursor and
  its documentation as a tooltip.

`Alt+Tab` may be captured by the window manager, which often uses it to
switch windows: if it does nothing in the editor, look for that binding
in the settings of the window manager. The shortcuts of the editor are
not configurable: they are defined in `src/TextEditBase.cxx` and, for
the mode specific ones, in the major mode itself. They belong to
the `emacs` style, the default one: the `Options` menu can switch to the
`Qt` style, in which a major mode sees no key event at all, so the three
shortcuts above become inactive, like the other mode specific ones.

# Options

The options live in the `QSettings` of `tfel-editor`, which is
`~/.config/CEA/tfel-editor.conf` on `GNU/Linux`, and are read when the
server is started: restarting the editor is enough for a change to be
taken into account.

| Key                 | Default    | Description                                        |
|---------------------|------------|----------------------------------------------------|
| `clangd/enable`     | `true`     | turn the `clangd` support off (`false`) or on      |
| `clangd/executable` | empty      | path of the `clangd` binary; empty means: search   |
| `clangd/arguments`  | empty      | extra command line arguments, separated by commas  |

For example, adding these lines to the configuration file:

~~~~{.ini}
[General]
clangd/arguments=--clang-tidy,--header-insertion=never
~~~~

turns on the `clang-tidy` checks. The other options of
[`clangd`](https://clangd.llvm.org/man_page.html), like
`--header-insertion=never` to stop it from completing an unexpected
`#include`, are set the same way. Likewise, `clangd/executable` points
the editor at a specific installation, and `clangd/enable` turns
everything off:

~~~~{.ini}
[General]
clangd/executable=/usr/lib/llvm-15/bin/clangd
clangd/enable=false
~~~~

# Projects

For a single file, or a small project whose includes are all on the
default search path, `clangd` needs no help: it guesses the compilation
options. Anything bigger requires a
[`compile_commands.json`](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
file, the compilation database that records how each translation unit
is really compiled. `CMake` generates it with:

~~~~{.bash}
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
~~~~

and `clangd` looks for it in a `build` directory of the project or next
to the opened file, so a symbolic link is often enough:

~~~~{.bash}
ln -s build/compile_commands.json .
~~~~

`Makefile` based projects, like `TFEL` itself, can generate the same
file as a by product of the build with
[`bear`](https://github.com/rizsotto/Bear):

~~~~{.bash}
bear -- make
~~~~

Without a compilation database, `clangd` still works — completion,
navigation and hover are available — but the diagnostics are limited to
what can be compiled with guessed options: files of a `TFEL` based
project report spurious "file not found" errors for headers that are
only reachable through the include directories of the real build.

# Remarks

- The server is started when the first `C`/`C++` file of a window is
  opened, not at the startup of `tfel-editor`. One `clangd` instance
  serves all the `C`/`C++` buffers of a window, and a new window uses a
  new instance.
- On the first opening, `clangd` parses the whole translation unit,
  which can take a moment on a large project: the minibuffer says
  `contacting clangd...` while the editor waits for the handshake.
- Closing the window shuts the server down with the `shutdown` request
  and the `exit` notification of the protocol; closing a `C`/`C++`
  buffer only closes the document, so the remaining buffers keep the
  server they are served by.
- Diagnostics arrive asynchronously: after the file is opened or
  modified, the underlines appear once the server has reparsed it,
  which takes a fraction of a second for a buffer and can take longer
  for a large project on the first opening.
- Buffers which have no file yet (`*scratch*`, a new file) cannot be
  analysed: `clangd` identifies documents by their file name, so the
  server is only fed with buffers that have been saved at least once.
- While the completion popup is open, `Tab` no longer indents the line:
  it inserts the selected candidate. `Tab` indents again as soon as the
  popup is closed.
- The underlines are single underlines, not wavy lines: `Qt 6.2`, the
  version `tfel-editor` is built against, does not ink wave styles in
  its text widgets.

# References

- The implementation of the client is described in
  [issue #125](https://github.com/thelfer/tfel-editor/issues/125).
- [clangd documentation](https://clangd.llvm.org/guides), in particular
  the [design](https://clangd.llvm.org/design) page, which explains
  where the compilation options come from.
