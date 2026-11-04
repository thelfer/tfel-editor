This page describes the `C`/`C++` support of `tfel-editor`, which is
based on a [`clangd`](https://clangd.llvm.org) language server.

When a `C` or `C++` file is opened, `tfel-editor` acts as a
[`Language Server Protocol`](https://microsoft.github.io/language-server-protocol/)
client: it keeps the buffer in sync with a `clangd` session and turns what
the server sends back into editor behaviour. The editor itself embeds no
`C++` parser: everything smart about `C` and `C++` files comes from
`clangd`.

The session is not started on its own account, but when it is asked for,
either from the `C/C++` menu of the mode or with one of the
`launch-clangd` commands, and with the compilation database that was
chosen, if any. See [Launching `clangd`](#launching-clangd).

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
finds it when a session is launched and keeps it alive until the window
is closed or another session is launched. The search, in this order:

- the `clangd/executable` option, if it is set;
- `clangd` in the `PATH`;
- the highest versioned `clangd-NN` of `/usr/bin`, `/usr/local/bin` and
  `/bin`;
- the highest versioned `/usr/lib/llvm-NN/bin/clangd` of the `LLVM`
  installations, which is where the `Debian` packages put the binary.

If `clangd` cannot be found, the support stays silent about it until a
session is asked for: the `C`/`C++` buffers work as before, without the
diagnostics and the completion, and the minibuffer says that the binary
could not be found when the session fails to start.

The mode covers the extensions the `C++` major mode is registered for:
`.cxx`, `.hxx`, `.ixx`, `.cpp`, `.hpp`, `.ipp`, `.CC`, `.hh`, `.c`, `.h`
and `.cu`.

# Launching `clangd`

Nothing talks to a server until one is launched, because what a session
is launched *with* — the compilation database above all — is not
something the editor can guess and cannot be changed afterwards. Two
entries of the `C/C++` menu of the mode, shown in the menu bar while a
`C`/`C++` buffer is current, do it:

- `Launch clangd` starts a session with the options of the settings only,
  which leaves `clangd` to look for a compilation database by itself, as
  it does when it is run in a terminal. The command is `launch-clangd`.
- `Launch clangd with a compile_commands.json` asks for a compilation
  database in the minibuffer first: the input completes paths, starting
  in the directory of the file which is open, and the answer is either
  the database file itself or the directory holding it. The session is
  then started with `--compile-commands-dir`, which is what `clangd`
  reads: the directory, not the file. The command is
  `launch-clangd-with-compile-commands`.

An empty answer to the question, and the first entry, are the same thing:
a session which searches a database by itself. An answer which points
nowhere is refused, and the running session is left alone.

Launching a session while one is running *replaces* it: a server cannot
change the options it was started with, so the old session is shut down
and a new one answers. The buffers are not lost on the way: each of them
is opened again on the new session as soon as its handshake is over, and
the underlines the old session had published are forgotten, as they
describe files a server read with other options, or not at all.

Requests never launch a session: `Alt+Tab`, `Alt+.`, `Alt+?` and the
context menu report that `clangd` is not launched and where to launch it
instead of starting one they were not asked for. The documents of the
buffers are registered on the client as their files are opened, whether a
session runs or not, and are sent to the next one.

One session serves all the `C`/`C++` buffers of a window, and a new
window uses a new session.

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

The context menu, reached with a right click, holds a `Go to definition`
entry whenever `clangd` has a definition for the symbol the menu was
opened on, which is where the cursor is placed. The question is asked as
the menu is built, since that is the only way to know whether the entry
has something to offer, and the answer is waited for a second only: a
server which is not up yet, or which is busy parsing the file, leaves the
plain menu instead of delaying it. When a symbol has several definitions, as
a virtual override has, one entry each is offered, labelled with the file
and the line they point to.

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
`~/.config/CEA/tfel-editor.conf` on `GNU/Linux`, and are read each time a
session is launched: launching a new session from the `C/C++` menu is
enough for a change to be taken into account, the editor does not have to
be restarted.

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

Rather than linking the database into the project, it can be handed to
the session that is launched for it: the `Launch clangd with a
compile_commands.json` entry asks for its path, and the session starts
with `--compile-commands-dir` pointing at the directory of that file.
That option makes the search above moot: `clangd` only looks in the
directory it is told about, so a session launched that way is bound to
the build directory it was given, whatever the buffers it is fed. The
answers to the question are kept in the `clangd/compile-commands/history`
setting, so a project whose database lives in a directory of its own is
answered with `Ctrl-P`, or the up arrow, from the next session on.

# Remarks

- The session is launched by the user, from the `C/C++` menu or one of the
  `launch-clangd` commands, not at the startup of `tfel-editor` and not
  when the first `C`/`C++` file is opened: the options a session runs
  with can not be changed while it runs, so they are asked for when it is
  started. One `clangd` instance serves all the `C`/`C++` buffers of a
  window, and a new window uses a new instance.
- On the first opening, `clangd` parses the whole translation unit,
  which can take a moment on a large project: the minibuffer says
  `contacting clangd...` while the editor waits for the handshake, and
  says `clangd launched` or `clangd restarted` when a session is started.
- Closing the window shuts the server down with the `shutdown` request
  and the `exit` notification of the protocol; the same ritual shuts down
  a session which is replaced by a newly launched one. Closing a
  `C`/`C++` buffer only closes the document, so the remaining buffers keep
  the server they are served by.
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
