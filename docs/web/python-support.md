This page describes the `Python` support of `tfel-editor`: the `Python`
major mode and what a [`pylsp`](https://github.com/python-lsp/python-lsp-server)
language server adds to it.

A `.py`, `.pyi` or `.pyx` file is opened in that mode, which is one of the
modes which edit the source of a program: `Ctrl-;` comments the line with
`#`, `Alt+M` runs the file, and the output of that run is shared with the
buffers of the other programming languages, as
[the compilation output page](compilation-output.html) explains.

When a file of this language is opened, `tfel-editor` acts as a
[`Language Server Protocol`](https://microsoft.github.io/language-server-protocol/)
client: it keeps the buffer in sync with a `pylsp` session and turns what
the server sends back into editor behaviour. The editor itself embeds no
`Python` parser: everything smart about a `Python` file — what a name is,
where it is defined, what a call takes — comes from the server. The
machinery is the one [the `clangd` support page](clangd-support.html)
describes for `C` and `C++`, and the shortcuts are the same; what differs
is what the server is able to answer, and when a session is worth
starting.

A session starts on its own, with the file: `pylsp` needs nothing beyond
the text it is handed to say what a name is, so there is no project to
find first and no database to choose — the choice which is why a `C++`
session is only started on its own when a `compile_commands.json` is
there to be read. The minibuffer names the directory that session takes
as the root of the program. `pylsp/start-by-default` turns those sessions
off; see the [Options](#options) section.

# Main features

- **Diagnostics**: the errors and warnings of the checkers the server was
  installed with — `pyflakes` and `pycodestyle` are what the usual
  installation brings — are underlined in the text, with a color set by
  their severity: red for an error, orange for a warning, blue for a note
  and gray for a hint. The message goes with the place: the minibuffer
  echoes it once the cursor rests on an underlined range, and resting the
  mouse on one shows the same text as a tooltip. What stands under
  `nobody` of `value = greet(nobody)` is `undefined name 'nobody'`, and
  nothing stands under `greet`, which the file does define.
- **Completion**: completion of names, attributes, keywords and builtins,
  computed by the server on the text as it is being edited. `Alt+Tab`
  asks for it; see [Shortcuts](#shortcuts).
- **Go to definition**: jump to the line where the symbol under the
  cursor is defined, opening the file which holds it, or the source of
  the standard library, if needed.
- **Find references**: the uses of the symbol under the cursor, its
  definition counted in, listed as `file:line` entries in a menu which
  jumps to the chosen one.
- **Rename**: the symbol under the cursor is renamed wherever the server
  knows the name is used. The new name is asked in the minibuffer and the
  edits are applied to every buffer visiting a file they name, which is
  opened if it is not visited yet; the buffers come back to the one the
  rename was asked in. The buffers are rewritten, not saved: saving stays
  `Ctrl-X Ctrl-S`.
- **Signature help**: the parameter list of the call the cursor stands
  in, echoed in the minibuffer with the argument being typed picked out
  of it between `<` and `>`. `Alt-S` asks for it, and so does the editor
  itself when a character the server lists as a place a call is typed
  around reaches the buffer.
- **Document symbols**: the overview of the file as the server reads it,
  and a jump to whichever entry of it is chosen. The entries are
  qualified by the scope they live in the way this language spells such
  a name — `Widget.paint`, not `Widget::paint` — each with what the
  server says beside it.
- **Hover**: a tooltip showing the signature of the symbol under the
  cursor and its documentation, which is where the docstring of a
  function is read without leaving the call.
- **Format the buffer**: `Alt+Q` hands the text to the server and writes
  back what it answers, which is the program `pylsp/formatter` names — `Ruff`
  by default, `Black`, `Autopep8`, `Yapf` or none, from the `Format with`
  submenu of the `Python` menu. A buffer which is already formatted is
  answered with nothing and stays silent, since that is the common answer of
  the key.
- **Run the file**: `Alt+M` proposes `python3 <name of the file>`, which
  is the shortest honest answer to "run this", and the output goes to the
  read-only area beside the buffer.
- **Indentation which follows the blocks**: `Tab` puts a line at the
  indentation the file above it allows and walks it through the places it is
  allowed to stand at, one place per press, the way `python.el` does;
  `Shift-Tab` takes a line out of its block and the same two keys shift a
  region. See [the indentation section](#indentation).
- **An interpreter beside the buffer**: `Ctrl-C Ctrl-L`, `Ctrl-C Ctrl-R` and
  `Ctrl-C Ctrl-B` hand the line, the region or the whole buffer to a
  `Python` session which shows what it answers beside the buffer. See
  [the interpreter section](#talking-to-an-interpreter).
- **The tools of the language are chosen, not hardcoded**: which programs
  read the text to draw the underlines of a buffer, which one formats it and
  which one puts its imports in order are entries of the configuration and
  entries of the `Python` menu, and the buffers on screen follow the choice.
  `Ruff`, which does the work of the two usual checkers in one program which
  is fast, is the default of all three questions.
- **Sort the imports**: the `Sort imports` entry of the `Python` menu, and
  the `python-sort-import` command, ask the chosen program and write back
  what it answers.

Completion, navigation and hover wait at most three seconds for the
server: a server which does not answer is reported in the minibuffer
instead of freezing the editor.

The text is sent to the server as it is modified, so the diagnostics, the
candidates and the hover of a buffer reflect what is on the screen, not
what was last saved on disk.

Three things the `C`/`C++` mode asks of `clangd` are asked of nobody
here, because the server has nothing to answer:

- the *declaration* of a name, which `pylsp` refuses outright; a jump is
  decided by the definition question alone, and asking a second question
  which is refused would make every word the cursor rests on wait on
  nothing;
- *semantic colors*: `pylsp` announces no semantic tokens at all, so the
  faces of the identifiers are those of the highlighter of the mode, and
  no color of a name comes from the server;
- *quick fixes*: the context menu asks what the server can repair and is
  answered nothing, so no `Fix:` entry ever appears. A checker which
  names a mistake without a fix has that consequence.

# The faces of the text

What is colored without a session, and what is colored whether a session
runs or not, is decided by a highlighter of the mode itself:

| text                                      | face                        |
|-------------------------------------------|-----------------------------|
| a word of the language (`def`, `class`, `if`, `await`, …) | bold dark magenta |
| `True`, `False`, `None`                   | dark red                    |
| a builtin (`len`, `print`, `isinstance`, …) | dark cyan                 |
| the name a `def` introduces                | bold dark cyan              |
| the name a `class` introduces              | bold dark blue              |
| a decorator (`@staticmethod`)             | italic dark cyan            |
| a number                                   | dark red                    |
| a string, quoted or triple-quoted          | dark green                  |
| a docstring                                | italic dark green           |
| a comment                                  | a lightened dark red        |

Two of those rows are a decision rather than a pattern match, and they are
the reason the highlighter is not a list of regular expressions:

- **A docstring is read as prose.** Three quotes standing on the line
  which heads a `def`, a `class` or the module itself open a docstring,
  and every line that follows is prose to its end, where the same three
  quotes standing in an assignment open a string of the program. The two
  are drawn with different faces — italic and plain green — and the state
  a line ends with says which of the two the next line is inside of.
- **A decorator is a statement of its own.** An `@` at the start of a
  line is a decorator; the same character standing in the middle of a
  line belongs to an expression and is left alone.

The *soft keywords* of the language — `match`, `case` and `type`, which
are words of the language only where the grammar allows them and ordinary
names everywhere else — are deliberately not colored: highlighting them
by name would ink over a variable called `case`, which a great many
`Python` programs have.

# Indentation

`Tab` indents the line the cursor stands on the way `python.el` of `GNU Emacs`
indents it, which is not the shape an indent key usually has: the presses of the
key walk the line through the *places it is allowed to stand at*, from the
highest to the lowest, and the press which sits at the lowest place brings it
back to the highest. The last line of

~~~~{.python}
print("first")
if True:
    if False:
print("last")
~~~~

has three such places — the body of the inner `if`, the body of the outer one,
and no block at all — so four presses on that line write it at 8, then at 4,
then at 0, then at 8 again, and they do so whatever indentation the line was
written with. A line which belongs one level deeper is one press away; a line
written at 8 which belongs at the margin is three.

The places are decided by the lines *above* the line, not by the place the line
holds, and they are made of:

- the indentation the file above allows — the body of the block the line stands
  in; the place right behind the bracket which holds it when the line continues
  a call; the indentation of the line a triple quoted string was opened on when
  the line stands inside that string;
- the multiples of the level of the language, four columns, which stand under
  that place: the continuation line of `def f(a,` is allowed at 6, right behind
  the opening parenthesis, and the presses take it from there to 4 and then to
  nothing;
- for a line which *closes* a block — `else`, `elif`, `except`, `finally` — the
  indentations of the blocks it can close, which are no regular ladder: the
  `else` of an `if` which is itself the body of an `if` whose body already holds
  a line has one place to stand and no press moves it, while the `else` of three
  nested `if`s walks 8, 4, 0, 8.

`Shift-Tab` walks the same list in one direction only: one place down per press,
and it *stops* at the lowest place. It is the key which means *out of this
block*, and a key which wrapped would put the line back into the block it has
just been taken out of. `python.el` binds no such key — it shifts a region with
`C-c <` and `C-c >` — so this one is the mode's own.

A selection which spans more than one line is a region, and the two keys move a
region rather than reindenting it line by line: `Tab` takes every non-blank line
of it one level to the right, `Shift-Tab` takes them one level back, and the
inner levels of a block keep the difference between the lines they are made of.
The blank lines of the region are left empty — an empty line is how this
language separates two functions, and a line which holds nothing is nothing to
move. A selection which stays inside a single line is no region: the key indents
the line it stands on.

What the presses answer with is a model of `python-indent-line-function` of
`python.el`, `GNU Emacs` 27.1, measured press by press against the real thing,
and the cycles it gives are what `tests/TestPythonMode.cxx` asserts on. Where
the two disagree, the mode is wrong.

# Getting `pylsp`

The server is **not** part of `tfel-editor` and is not a program `apt`
delivers under that name; it is a `Python` package:

~~~~{.bash}
pip install 'python-lsp-server[all]' python-lsp-ruff
~~~~

The `[all]` extra is what makes the conversation worth having: it brings
`jedi`, which answers what a name is and where it is defined, `pyflakes` and
`pycodestyle`, whose findings stand under the text, and the programs the
format question of the server can be answered with — `autopep8`, `yapf` and
the rest of the checkers, `flake8`, `pylint` and `pydocstyle` included.

What `[all]` does **not** bring is `ruff`, which is the program this mode
asks to lint, to format and to sort the imports by default: its plugin is a
package of its own, `python-lsp-ruff`, and that is the line above. The other
tools of the same kind are named by [the Options](#options) and are separate
packages too — `python-lsp-black` for `Black`, `pyls-isort` for `Isort`,
`pylsp-mypy` for `Mypy`. A plugin which is not installed is simply never
asked: the server starts, the buffers are read by what is there, and the
entries of the `Linters` submenu tick nothing.

`pylsp` is then in the directories of the session and nothing has to be
configured. Where it is not — a server installed in a virtual environment,
or a `pipx` installation, which keeps its own environment and needs
`pipx inject python-lsp-server python-lsp-ruff` for a plugin to reach it —
the `pylsp/executable` option names it, and naming the one of that
environment is what makes the answers of the buffers read that
environment's installed packages. The search, in this order:

- the `pylsp/executable` option, if it is set;
- `pylsp` in the `PATH` of the session.

Where neither answers, nothing is started: the buffers keep the faces of
the highlighter and no underlines, the minibuffer says once that the
program was not found, and the `Start pylsp` entry repeats that as often
as it is asked.

# Starting and stopping

The mode starts no server while building itself; a session comes about in
one of three ways:

- **a file of this language is opened**, which is the automatic case and
  the useful one. The session reads the directory of that file as the
  root of the program and the minibuffer says so: `pylsp started with
  project root <directory>`. Nothing is started for a buffer which visits
  no file — `*scratch*`, the area of a process output —, and an automatic
  start never replaces a session which is running: the root a session
  reads is chosen when it starts, and a file of another project is none to
  change what somebody else is working with.
- **the `Start pylsp` entry of the `Python` menu**, or the `start-pylsp`
  command, which is the same thing: the program and the arguments the
  settings name, the project of the file the current buffer visits. Where
  a session is already running, this *replaces* it, which the minibuffer
  reports as `pylsp restarted`, and the buffers are opened again on the
  new session as soon as its handshake is over.
- **the `Stop pylsp` entry**, or `stop-pylsp`: the server is asked to shut
  down and loses the argument if it lingers. The underlines it published
  go away with it, since they say something a program which is gone no
  longer knows. The minibuffer reports `pylsp stopped`, or `pylsp is not
  started` when no session was running.

Requests never start a session: `Alt+Tab`, `Alt+.`, `Alt+,`, `Alt+R`,
`Alt+S`, `Alt+O`, `Alt+?`, `Alt+Q`, `Ctrl-Left click` and the context menu
report that `pylsp` is not started and where to start it instead of
starting one they were not asked for. The documents of the buffers are
registered on the client as their files are opened, whether a session runs
or not, and are sent to the next one.

One session serves all the `Python` buffers of a window, and a new window
uses a new session. Closing the window shuts the server down with the
`shutdown` request and the `exit` notification of the protocol; closing a
buffer only closes its document.

# Shortcuts

The following shortcuts are active in `Python` buffers, in addition to
the [usual `emacs` shortcuts](emacs-shortcuts.html) of the editor. They
are those of the `C`/`C++` mode, with the same bindings and the same
waiting rules; [its page](clangd-support.html) describes each of them in
full and only what differs is said here:

- `Alt+Tab`: complete the name at the cursor. `Tab` alone indents the line,
  and belongs to the popup only while it is open.
- `Alt+.`: go to the definition of the symbol under the cursor. Only the
  definition is asked, since this server knows no separate declaration
  question.
- `Ctrl-Left click`: the mouse's `Alt+.`, on the word it was pressed on.
- `Alt+,`: list the references to the symbol under the cursor.
- `Alt+R`: rename the symbol under the cursor. The server of this language
  answers a rename as one rewrite of the whole file, which is what the
  completion library behind it gives back; the editor cuts that answer
  back to the lines which actually change, which is what leaves the cursor
  on the line the rename was asked on instead of at the end of the file.
- `Alt+S`: the parameter list of the call the cursor stands in.
- `Alt+O`: go to one of the symbols of the file. The names are qualified
  the way this language spells them: `Widget.paint`, and typing `paint`
  narrows the list to it.
- `Alt+?`: the signature and the documentation of the symbol under the
  cursor.
- `Alt+Q`: format the buffer with the program `pylsp/formatter` names, which
  the `Format with` submenu of the `Python` menu chooses and `Ruff` is by
  default. A buffer which is already formatted is answered with nothing at all
  and is left silent, since it is the common answer of this key; where no
  session runs, the report says so and names where a session is started. Which
  changes a format makes are those of the program which was chosen, and the
  default rules of `autopep8`, for one, ignore the missing spaces around an
  arithmetic operator (`E226`), among others.
- `Alt+M`: run the file. The command proposed is `python3` followed by the
  name of the file the buffer visits, in the directory of that file; an
  empty answer runs nothing. The output is a secondary task, shared with
  the other programming buffers.
- `Tab`: indent the line the cursor stands on, or move a region one level to
  the right. Repeated presses walk the line through the places it is allowed
  to stand at. See [the indentation section](#indentation).
- `Shift-Tab`: take the line one place out of its block, or move a region one
  level back to the left.
- `Ctrl-C Ctrl-L`: send the line the cursor stands on to the interpreter.
- `Ctrl-C Ctrl-R`: send the selected region to the interpreter.
- `Ctrl-C Ctrl-B`: send the whole buffer to the interpreter. See
  [the interpreter section](#talking-to-an-interpreter).

The navigation hints are those of the other language served by a server:
the word the cursor sits on is underlined when the server knows where the
symbol is defined, the mouse pointer turns into the icon of that jump
while it rests on such a word, and what was answered about a word is
remembered until the text changes or the session is stopped.

The `Python` menu holds all of that beside the keys, since a key one has
never heard of is a key never pressed: `Start pylsp`, `Go to symbol...`,
`Start interpreter` with the three sends below it, `Sort imports`, the
`Linters`, `Format with` and `Sort imports with` submenus, and `Stop pylsp`.

# Talking to an interpreter

The line, the region or the whole buffer are handed to an interactive
interpreter, and what it answers — the banner, the prompts, the values, the
tracebacks — is shown in a secondary task of the buffer named `Python Output`,
which is what the body of a function is tried in before the rest of the program
is asked to run it.

| Keys              | What goes to the interpreter                        |
|-------------------|-----------------------------------------------------|
| `Ctrl-C Ctrl-L`   | the line the cursor stands on                       |
| `Ctrl-C Ctrl-R`   | the selected region                                 |
| `Ctrl-C Ctrl-B`   | the whole buffer                                    |

Those three keys open a session when none is running, so the first line sent is
what starts it; `Start interpreter`, or the command `start-python`, opens one
without sending anything. The commands of the three keys are `python-send-line`,
`python-send-region` and `python-send-buffer`. The page belongs to the buffer
which asked for it, and ends the interpreter when it is closed: what a session
has defined is gone with it, and `Start interpreter` opens a new one. Typing
`exit()` in the page has the same end, and the next key which sends something
opens the session again.

`Ctrl-C` is the prefix of the second keys of a mode, and these three are those
of the `Cast3M` mode for its own interpreter, so the two languages are talked to
with the same hand. `Ctrl-C Ctrl-C`, which saves the buffer, belongs to the
editor and to no mode.

How the text reaches the interpreter is decided by what the interactive parser
does with it, and not by taste:

- **a line** is written to it as it stands, with an empty line added when the
  line opens a block: a `def f():` alone leaves the parser waiting for a body,
  and the empty line is what tells it the block is over. Without it every line
  sent afterwards is swallowed as that body, which reads as the keys doing
  nothing;
- **a region of several lines, and the whole buffer**, are written to a
  temporary file the interpreter is asked to read and execute. The interactive
  parser ends an indented block at an empty line, so a function whose body holds
  a blank line — which is what a buffer of this language is full of — comes back
  as `IndentationError: unexpected indent` with the function left undefined when
  it is written as text, and defines the function when it is sent through a file;
- **a region of a single line** is sent as the line it holds.

The page reads as a conversation typed at a terminal, with each send standing
behind a prompt above its answer:

~~~~{.text}
>>> print('toto')
toto
>>> print('toto')
toto
>>>
~~~~

The interpreter runs behind a pipe, which echoes nothing, so the editor writes
the sent text itself: a send goes out at a prompt, together with its echo, one
send per prompt, in the order of the presses. A key pressed while the line
before it is still running therefore waits its turn rather than be written in
the middle of an answer, and a region or a buffer shows as the text it holds,
not as the line which carries it through the file route. An interpreter which
prompts with something other than `>>> ` and `... ` reads nothing from the
queue, exactly as `comint` in Emacs hands nothing to a program whose prompt it
cannot read.

The interpreter is started with `-i`, which keeps it up after what it was handed
instead of treating it as a script and exiting, and with `-u`, which is what
lets its answers reach the page as they are made rather than when the
interpreter decides to flush them. It runs in the directory of the file the
buffer visits, which is what makes the imports of a project and the module
standing beside it visible to it.

The program is the one `python/executable` names, `python3` by default, and that
address is read on every launch: the release chosen in the configuration is run
by the next session with no restart of the editor. A program which is not where
the entry says it is is named by the minibuffer — `/no/such/python could not be
started` — and the keys which were pressed are not reported as unknown shortcuts
on top of that.

# Options

The options live in the `QSettings` of `tfel-editor`, which is
`~/.config/CEA/tfel-editor.conf` on `GNU/Linux`. None of them asks for a restart
of the editor: the entries which name the server and its arguments are read each
time a session is started, so `Start pylsp` is enough for a change to be taken
into account; the entries of the tools of the language are read at the same
moments, and the `Python` menu which writes them hands what they now say to the
session which is running, which is what makes the underlines of the buffers
follow a click; the program of the interpreter is read on every launch. What
[the configuration dialog](configuration.html) writes about the tools reaches
the next session rather than the running one, since that dialog tells no server
anything — the `Python` menu is the one which does.

| Key                         | Default   | Description                                               |
|-----------------------------|-----------|-----------------------------------------------------------|
| `pylsp/executable`          | empty     | program of the server; empty means: search                 |
| `pylsp/arguments`           | empty     | extra command line arguments, separated by commas          |
| `pylsp/start-by-default`    | `true`    | start a session when a file of this language is opened     |
| `pylsp/linters/ruff`        | `true`    | whether that program reads the text of a buffer            |
| `pylsp/linters/pyflakes`    | `false`   | the same question for that program                         |
| `pylsp/linters/pycodestyle` | `false`   | the same question for that program                         |
| `pylsp/linters/flake8`      | `false`   | the same question for that program                         |
| `pylsp/linters/pylint`      | `false`   | the same question for that program                         |
| `pylsp/linters/mccabe`      | `false`   | the same question for that program                         |
| `pylsp/linters/pydocstyle`  | `false`   | the same question for that program                         |
| `pylsp/linters/pylsp_mypy`  | `false`   | the same question for that program                         |
| `pylsp/formatter`           | `ruff`    | which program a request to format a buffer answers with    |
| `pylsp/import-sorter`       | `ruff`    | which program the `Sort imports` entry asks                |
| `python/executable`         | `python3` | program an interpreter session runs                        |

For example, pointing the editor at the server of a virtual environment:

~~~~{.ini}
[General]
pylsp/executable=/home/me/work/.venv/bin/pylsp
~~~~

and turning the automatic sessions off:

~~~~{.ini}
[General]
pylsp/start-by-default=false
~~~~

`pylsp/start-by-default` is read on every file which is opened, so
turning it off is answered by the next file opened and by no session which
is running: a buffer keeps the session it was opened with, whatever the
setting says afterwards. The two entries which name the program are read when a
session starts, so they reach the next `Start pylsp`.

The eight linters are one question each, and several of them at once is the
norm, since each sees a mistake the others do not. `ruff` is the one enabled by
default, as the plugin of that name does by its own account, and it reports
what `pyflakes` and `pycodestyle` report in one program which is faster than
either — which is why those two stand off: a buffer would else read the same
mistake under three underlines. `pylint` is off because it reads the file *on
disk* and so has nothing to say about a buffer which was not saved; `mypy` is
off because it needs the types of the names it reads to say anything and
answers more warnings than mistakes on a file which types nothing. The `I`
rules of `ruff` are not selected by the linter alone, see the sorter below.

The formatter and the import sorter are each *one* program, not a list: every
formatter of `pylsp` answers the same question and the server keeps the first
answer which holds something, so two of these on at once is one answering and
the other silent. `none` is a choice of both questions, and it leaves a request
to format with nothing to write back, which is what lets a person format with a
program outside the editor. Choosing `ruff` as the sorter selects the `I` rules
of that program with it, since a Ruff which sees no problem in the order of the
imports offers no change sorting them; `Isort` is a wrapper of the formatting
question, so it puts in order what the formatter above it wrote.

The `Linters`, `Format with` and `Sort imports with` submenus of the `Python`
menu write those same addresses and hand what they now say to the session which
is running, which is what makes the underlines of the buffers follow a click.
The commands which write them are `python-enable-<tool>` and
`python-disable-<tool>`, one pair per linter, then `python-set-formatter` and
`python-set-import-sorter`. A tool is named as a person names it in a command
and in the menu — `Mypy` and `python-enable-mypy` — while the address keeps the
name the plugin of the server goes by, `pylsp/linters/pylsp_mypy`.

The entries are gathered in two themes of
[the configuration dialog](configuration.html), among the themes a major mode
exports: `pylsp`, for what the mode asks the server and what it asks it with,
and `python`, for the program which runs the text.

# Remarks

- A session reads the program, the arguments and the project it was
  *started* with. The directory it takes as the root of the program is the
  one of the file which asked for it, so a project whose packages are
  spread over several directories is read from the directory which holds
  them, and a file opened elsewhere does not move that root: restarting
  the session from a buffer of the project in question is.
- A buffer which visits no file yet cannot be analysed: the protocol
  identifies documents by their file name, so the server is only fed with
  buffers that have been saved at least once.
- Diagnostics arrive asynchronously: after a file is opened or modified,
  the underlines appear once the server has read it again, which takes a
  fraction of a second for a buffer and longer for a file which imports a
  great many packages.
- While the completion popup is open, `Tab` no longer indents the line: it
  inserts the selected candidate. `Tab` indents again as soon as the popup
  is closed.
- The underlines are single underlines, not wavy lines: `Qt 6.2`, the
  version `tfel-editor` is built against, does not ink wave styles in its
  text widgets.
- The prose of a `Python` buffer — its comments and docstrings — is read
  by no proofreader: the spell and grammar checkers of the editor are
  wired to the `LaTeX` and `Markdown` modes and to the three modes which
  hold a buffer of text, and this mode is not one of them yet. See
  [the grammar checking page](grammar-checking.html).

# References

- [issue #135](https://github.com/thelfer/tfel-editor/issues/135), which
  is the work this page describes: the support of the language server was
  extracted from the `C++` mode so that both modes drive one client, and
  the mode was rewritten around it.
- [python-lsp-server documentation](https://python-lsp-server.readthedocs.io),
  in particular the page of its
  [plugins](https://python-lsp-server.readthedocs.io/en/latest/server/plugins.html),
  which names the program which answers each question and how to turn one
  of them off.
- [the `clangd` support page](clangd-support.html), for the machinery both
  modes drive, and for the questions only a `C++` server answers.
