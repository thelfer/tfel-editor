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
  gray for a hint. The message goes with the place: the minibuffer
  echoes it once the cursor rests on an underlined range, and resting
  the mouse on one shows the same text as a tooltip.
- **Completion**: completion of names, members, keywords and macros,
  computed by `clangd` on the text as it is being edited.
- **Go to definition**: jump to the line where the symbol under the
  cursor is defined, opening the file which holds it if needed.
- **Find references**: the uses of the symbol under the cursor, its
  definition included, are listed as `file:line` entries in a menu
  which jumps to the chosen one.
- **Rename**: the symbol under the cursor is renamed everywhere
  `clangd` knows it uses the name: the new name is asked in the
  minibuffer, and the workspace edit the server answers is applied to
  every buffer visiting the files it names, opening the files which are
  not visited yet and coming back to the buffer one started from. The
  buffers are rewritten, not saved: saving stays `Ctrl-X Ctrl-S`.
- **Signature help**: the parameter list of the call the cursor stands
  in, echoed in the minibuffer with the argument being typed picked out
  of it between `<` and `>`, and the number of overloads the call could
  still be when it could be more than one. `Alt-S` asks for it, and so
  does the editor itself as the characters `clangd` says a call is
  typed around reach the buffer: `(`, `,`, `<` and the like. Nothing is
  said where there is no call, so typing braces stays quiet.
- **Quick fixes**: the context menu offers what `clangd` can repair on
  the line it was opened on — insert a missing `;`, add a dereference,
  fix a spelling — and triggering an entry writes the text the server
  sent. The buffers are rewritten, not saved, as for a rename.
- **Document symbols**: the overview of the file as `clangd` reads it,
  and a jump to whichever entry of it is chosen. The names are the
  qualified ones — `Widget::paint`, `tools::Helper::twice` —, each with
  what the server says beside it: the signature of a function, the kind
  of a class. A definition written out of the class it belongs to is an
  entry of its own, under the name it is spelled with.
- **Hover**: a tooltip showing the declaration of the symbol under the
  cursor and its documentation, if any.
- **Navigation hints**: the word the cursor sits on is underlined when
  `clangd` knows where the symbol is defined or declared, and the mouse
  pointer turns into the icon of that jump — `go-jump-definition` or
  `go-jump-declaration` of the Freedesktop Icon Naming Specification,
  the generic `go-jump` icon or a pointing hand where the theme has
  neither — while it rests on such a word. The jump itself is `Alt+.`,
  as before.
- **Semantic colors**: while a session runs, the identifiers of the file
  are colored with what `clangd` says they are, not with what a regular
  expression guesses from their spelling: the names of types — a class,
  a struct, an enumeration, an alias, a template parameter — in a dark
  blue, the names of functions and methods in a dark cyan, the names of
  macros in a dark magenta, and a name the server says is deprecated
  struck through. Everything else keeps the face it has without a
  session: the keywords, strings, numbers and comments belong to the
  built-in highlighter, which `clangd` is asked nothing about, and the
  locals, parameters and members it does name are deliberately left
  alone, so a file reads as it did and only what was unknowable stands
  out. `Color identifiers` in the `C/C++` menu turns the colors off and
  on again. The question is asked of the whole file — `clangd` answers no
  smaller part of it — when a session starts, when the file has been
  parsed, and when the text settles after an edit; a file which is not
  visited by a session keeps the highlighter's faces.

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
something the editor can guess, and the directory that database is read
from is fixed by the launch. What happens when the file itself is
regenerated under a running session is a different matter, and is what
[When the database changes](#when-the-database-changes) is about. Two
entries of the `C/C++` menu of the mode, shown in the menu bar while a
`C`/`C++` buffer is current, do it, and a third ends the session again:

- `Launch clangd` starts a session with the options of the settings only:
  the editor looks for a compilation database itself, as described below,
  and starts the session with the one it found, or without one. The
  command is `launch-clangd`.
- `Launch clangd with a compile_commands.json` asks for a compilation
  database in the minibuffer first: the input completes paths, starting
  in the directory of the file which is open, and the answer is either
  the database file itself or the directory holding it. The session is
  then started with `--compile-commands-dir`, which is what `clangd`
  reads: the directory, not the file. The command is
  `launch-clangd-with-compile-commands`.
- `Stop clangd` ends the session which is running: the server is asked
  to shut down and loses the argument if it lingers. The diagnostics it
  published go away with it, and the text of the buffers is kept, marked
  as never sent, so a session launched afterwards receives it anew. The
  minibuffer reports `clangd stopped`, or `clangd is not launched` when
  no session was running. The command is `stop-clangd`.

The database of the plain entry is looked for in this order:

1. the file named by the `--compile-commands-file=<path>` option of
   `tfel-editor`, when the editor was started with it and the file
   exists;
2. a `compile_commands.json` of the project of the file which is open:
   its directory, then the parent directories for as long as they are
   still part of the project — a parent holding a `CMakeLists.txt`
   is —, and, once the boundary of the project is reached, a
   sub-directory whose name starts with `build` of the top directory
   of the project or of the directory holding the project, which is
   where the directory of an out-of-source `cmake` build usually is;
   when several such directories coexist, the plain `build` is looked
   into first, and the others follow in name order;
3. nothing else: the session is launched without any database option,
   and `clangd` guesses the compilation options the way it does when
   it is run in a terminal.

What a plain launch ends up doing is what its minibuffer message says,
once, rather than one report per step:

| database used                     | message                                        |
|-----------------------------------|------------------------------------------------|
| found, no default given           | `using <path> to start clangd`                 |
| none found, no default given      | `no compile_commands.json found, clangd is restricted to current C++ file` |
| found, default given but missing  | `using <path> to start clangd (default file <path> not found)` |
| none found, default given but missing | `no compile_commands.json found and default file <path> is not found, clangd is restricted to current C++ file` |

A `--compile-commands-file` naming a file which does not exist is
therefore never a failed launch: the search of the project takes over
and the final message mentions the default that was left out. The
editor searches rather than leaving it to `clangd` because the option
which hands a database to the server, `--compile-commands-dir`, is set
when the session starts: knowing the database the session runs with
means choosing it before the launch, and saying so.

An empty answer to the question of the second entry is an answer too:
it launches the plain session, the one which looks for nothing and
guesses everything, even where a default or a project database was
waiting to be found. An answer which points nowhere is refused, and the
running session is left alone.

Launching a session while one is running *replaces* it: a server cannot
change the options it was started with, so the old session is shut down
and a new one answers. The buffers are not lost on the way: each of them
is opened again on the new session as soon as its handshake is over, and
the underlines the old session had published are forgotten, as they
describe files a server read with other options, or not at all.

Requests never launch a session: `Alt+Tab`, `Alt+.`, `Alt+,`, `Alt+R`,
`Alt+S`, `Alt+O`, `Alt+?` and the context menu report that `clangd` is not
launched and where to launch it instead of starting one they were not asked
for. The documents of the buffers are registered on the client as their
files are opened, whether a session runs or not, and are sent to the next
one.

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
- `Alt+,`: list the references to the symbol under the cursor, the
  definition counted in. The answer of the server becomes a menu of
  entries at the cursor, each naming its file and line — and the line's
  text for the references in this buffer — and jumps where chosen.
  `Escape` dismisses the menu.
- `Alt+R`: rename the symbol under the cursor. The new name is asked in
  the minibuffer, an empty answer cancels, and the edits the server
  returns are applied to the buffers visiting the files they name. The
  minibuffer then reports how many places changed in how many files; the
  buffers are modified but not saved.
- `Alt+S`: display the parameter list of the call the cursor stands in,
  the argument being typed marked between `<` and `>`. The same happens
  on its own whenever a character `clangd` lists as a place a call is
  typed around is written, which covers typing an argument list
  character by character. Where the cursor is not inside a call, the key
  says so and a typed character says nothing.
- `Alt+O`: go to one of the symbols of the file. The overview the server
  gives is displayed as a list of completions — each entry the qualified
  name of a symbol and, beside it, the signature of a function or the
  kind of a class — and typing narrows that list to the names it visits:
  `paint` is what one types for `Widget::paint`. `Tab` completes the part
  the remaining names share. The answer puts the cursor on the name of the
  symbol and the minibuffer says which one and on what line; a name the
  file does not hold is refused and the cursor stays where it was. An
  empty answer asks for nothing. The same question is the `Go to symbol...`
  entry of the `C/C++` menu.
- `Alt+?`: display the declaration of the symbol under the cursor and
  its documentation as a tooltip.

The context menu, reached with a right click, holds a `Go to definition`
entry whenever `clangd` has a definition for the symbol the menu was
opened on, which is where the cursor is placed. The question is asked as
the menu is built, since that is the only way to know whether the entry
has something to offer, and the answer is waited for a second only: a
server which is not up yet, or which is busy parsing the file, leaves the
plain menu instead of delaying it. A symbol without a body, a function
declared and never defined for instance, is no special case: `clangd`
answers the definition question of such a symbol with its declaration, and
the entry jumps to the prototype. When a symbol has several definitions, as
a virtual override has, one entry each is offered, labelled with the file
and the line they point to.

The same question round brings back the `Fix:` entries: what the server can
repair among the diagnostics of that line. An entry is named after the
change it writes — `Fix: insert ';'` — and its status tip is the message of
the diagnostic it answers to, since that is the whole reason the entry is
there. Triggering it applies the text the server sent to every file that
text names, opening the files which are not visited yet exactly as a rename
does, and the minibuffer reports what was applied; the buffers are
rewritten, not saved. Actions which carry no text are not offered: the AST
tweaks of `clangd`, generating a definition or extracting a variable, come
as a command the server would have to be asked a second question to apply,
and an entry which does nothing is worse than no entry.

The navigation hints ask the server neither on every keystroke nor on
every pixel of a mouse move: one settling timer of the mode is shared by
the cursor and the mouse, and what was answered about a word is
remembered until the text changes or the session is stopped, at which
point the underlines and the mouse cursor go away with it.

`Alt+Tab` may be captured by the window manager, which often uses it to
switch windows: if it does nothing in the editor, look for that binding
in the settings of the window manager. The shortcuts of the editor are
not configurable: they are defined in `src/TextEditBase.cxx` and, for
the mode specific ones, in the major mode itself. They belong to
the `emacs` style, the default one: the `Options` menu can switch to the
`Qt` style, in which a major mode sees no key event at all, so the
shortcuts above become inactive, like the other mode specific ones.

# Options

The options live in the `QSettings` of `tfel-editor`, which is
`~/.config/CEA/tfel-editor.conf` on `GNU/Linux`, and are read each time a
session is launched: launching a new session from the `C/C++` menu is
enough for a change to be taken into account, the editor does not have to
be restarted.

| Key                        | Default    | Description                                        |
|----------------------------|------------|----------------------------------------------------|
| `clangd/enable`            | `true`     | turn the `clangd` support off (`false`) or on      |
| `clangd/executable`        | empty      | path of the `clangd` binary; empty means: search   |
| `clangd/arguments`         | empty      | extra command line arguments, separated by commas  |
| `clangd/semantic-tokens`   | `true`     | color the identifiers with what the server says    |

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

The colors of the identifiers are the one option read again on every
question rather than once at a launch, so `clangd/semantic-tokens=false`
shows on the buffer of a running session without restarting it — as does
the `Color identifiers` entry of the `C/C++` menu, which writes that same
setting and repaints the file on screen. The buffers of the other files
follow as they settle.

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
the session that is launched for it: the plain entry finds a
`build` directory of the project by itself, as described above, and
launches the session with it; the `Launch clangd with a
compile_commands.json` entry asks for its path when the database lives
where no search looks, and the session starts with
`--compile-commands-dir` pointing at the directory of that file. That
option makes the search above moot: `clangd` only looks in the
directory it is told about, so a session launched that way is bound to
the build directory it was given, whatever the buffers it is fed. The
answers to the question are kept in the `clangd/compile-commands/history`
setting, so a project whose database lives in a directory of its own is
answered with `Ctrl-P`, or the up arrow, from the next session on.

A whole editor session can be started with a database too, which spares
the question for the projects whose database is always the same one:

~~~~{.bash}
tfel-editor --compile-commands-file=build/compile_commands.json
~~~~

The plain entries then start every session with that database, unless
one launches the other entry and answers the question differently.

# When the database changes

A build regenerates `compile_commands.json` all the time: a source is
added to the project, an include directory moves, a definition joins the
options. What a running session makes of it depends on which of its two
kinds of knowledge is asked about, and the two answer differently.

The **file being parsed** follows the database on its own. `clangd` says
so in its handshake, and re-reads the file when it next needs a command
line: a source which compiles only with a `-D` the regenerated database
has just gained loses its error on the next edit — and gets it back when
the definition is taken away again. Two details of that are worth
knowing:

- the reload goes with a parse, not with the clock: a session left idle
  reads nothing, and a parse within about two seconds of the
  regeneration still uses the old command line;
- an open buffer is never reparsed on its own account, so the underlines
  one is looking at say what the *old* options made of the text until
  something is typed in that buffer.

What the server **knows about the rest of the project** — the body of a
function defined in another file, the uses `Alt+,` lists, a file nobody
has opened — does not follow. On a reload `clangd` queues the files of
the new database again and then rebuilds only the ones it has not read
yet in that session: a file whose new option unveils a symbol stays as
it was indexed, whether or not it changed on disk in the meantime. A
file the regenerated database *adds* is the exception: it is indexed as
soon as the reload happens, and a definition question reaches it seconds
later.

Relaunching the session reads the new database, and indexes the files
whose contents differ from what it wrote on disk the last time it ran.
*Wrote on disk* is the catch: what `clangd` keeps of a file is filed
under the contents of that file, not under the command line which
compiled it, so changing the options of a file which reads the same
leaves what the server knows about it — and about the names it defines —
as it was. What it keeps lives in a `.cache/clangd` directory next to the
database the session was pointed at, which for a session reading it from
a build directory means:

~~~~{.bash}
rm -rf build/.cache
~~~~

Deleting that and launching the session again is what makes `clangd` read
the project as it is really compiled again. A change which only *adds
files* needs no such precaution, and neither does the buffer one is
editing: it uses the new options as soon as it is parsed again.

So a database which moved, or a session launched without one, stays what
the two entries of the menu are for: launching again, which replaces the
session. A database regenerated where the session already reads it is
the easier case — the file on screen follows it, and only the cross-file
answers above need the relaunch.

# Remarks

- The session is launched by the user, from the `C/C++` menu or one of the
  `launch-clangd` commands, not at the startup of `tfel-editor` and not
  when the first `C`/`C++` file is opened: the directory the compilation
  database is read from and the arguments the server runs with can not be
  changed while it runs, so they are asked for when it is started. One
  `clangd` instance serves all the `C`/`C++` buffers of a
  window, and a new window uses a new instance.
- On the first opening, `clangd` parses the whole translation unit,
  which can take a moment on a large project: the minibuffer says
  `contacting clangd...` while the editor waits for the handshake, and
  says `clangd launched` or `clangd restarted` when a session is
  started — the plain entry reports `using <path> to start clangd` or
  `no compile_commands.json found...` instead, as described above.
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
- What `clangd` offers and the editor does not use yet is listed, with
  the order in which the gaps are worth closing, on the
  [integration roadmap](clangd-integration.html) page.
- [clangd documentation](https://clangd.llvm.org/guides), in particular
  the [design](https://clangd.llvm.org/design) page, which explains
  where the compilation options come from.
