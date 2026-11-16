# Configuring the editor

The editor keeps its configuration in the standard configuration file of
the platform — `~/.config/CEA/tfel-editor.conf` under GNU/Linux —, which
`Qt` reads and writes through `QSettings`. What can be edited from inside
the editor is described here.

# Opening the dialog

The dialog is the `Configuration` entry of the `Options` menu, and the
command `configure-tfel-editor` (`Alt+X`) opens the same window.

The themes the configuration is made of are listed on the left, the
entries of the chosen theme on the right. The list keeps two things
apart which have nothing in common: the themes of the editor, under the
title `Editor`, and the themes a major mode exports, under the title
`Major modes`. The first are always there; the second come with the
modes which are installed and are about a program a mode drives, not
about the editor. A title is not a theme: it can not be chosen and shows
nothing. The page of a theme of a mode says whose it is above the entries
it holds, since `clangd`, `castem`, `latex`, `pylsp` and `python` are names
of programs and say nothing of the mode which launches them.

A theme which is a corner of another one is a submenu of it: the entries
of a proofreader — which program it is, which language it reads — are
opened under the theme which chooses that proofreader, since the two are
one decision. Those submenus are opened, never folded: an entry behind a
click nobody makes is an entry which is not found.

The name of an entry stands
above the field which edits it, which has the width of the page to
itself — a path of a program is not something to read in a narrow box —
and the sentence which says what the entry is for is the tool tip of both.
A question answered by yes or no is the exception, because a page of them
is read as a list: its name and its check box stand on one line and the box
is *flush with the right edge* of the page, which is the one column the boxes
of a theme stand in. A box under the name it answers reads as an orphan, and
a box at the end of each name of its own makes a ragged edge instead of a
column; a column behind the longest name of the theme would leave the right
half of every line empty. The name of such an entry is the buddy of its box,
which is what makes a click on the name toggle it.
Each entry is edited with the widget which suits the kind of value it
holds: a check box for a boolean, a spin box for a number, a one line edit
for a text, a multi line edit for a list of texts, a combo box for a
choice, a button which names the font and opens a font dialog for a font.

Nothing is written while one edits: the dialog writes when it is
accepted, since most entries are read once, when the widgets which use
them are built. The entries whose effect one has to see under one's hands
are applied at once: the font of the text edits, the style of the key
bindings, the language the misspelled words are underlined with, and which
proofreader reads them. What is left waits for a new buffer, or for a
restart of the editor, and the minibuffer says how many entries were
written.

`Restore Defaults` sets every field back to the value the editor uses
when the configuration file holds nothing; it writes nothing by itself, so
the dialog can be left after that without any effect. An entry whose
value is the default one is *removed* from the configuration file rather
than written: the file holds what was changed and nothing else, and an
entry which was never shown keeps the reading its default has in the code
which reads it.

# What the dialog holds, and what it leaves out

The dialog shows what one *chooses*: the look of the editor, the key
bindings it uses, the language of the underlines, the program the `Cast3M`
mode launches, what the `C`/`C++` mode asks its language server. It does not
show what the editor *remembers* by itself:

- the histories of the prompts, which are the answers the minibuffer
  proposes again — the commands of `Alt+X`, the shell commands of `Alt+!`,
  the expressions of the `grep` command, the strings of `Alt+%`, and the
  compilation command `Alt+M` proposes in a buffer of a mode which
  compiles;
- the list of recent files `File > Open Recent` offers;
- the interface the dialog of `Run MFront` proposes for the next file of
  each kind.

Those addresses are written by the very act of working: an address a
`CommandLine` is given and written back when its answer is taken. A field
of a configuration dialog would show what was there when the dialog was
built and be overwritten the next time a prompt is used, and editing a
history by hand is not what a person reaches a configuration dialog for.
An address which nothing declares has no field, which is why the dialog
shows what was declared and not what the configuration file happens to
hold; clearing a history is a matter of removing that address from the
file, as [editing the file by hand](#editing-the-file-by-hand) does.

# The themes

The themes are gathered from what the libraries of the editor and the
major modes which are loaded said they read: a theme of a mode whose
plugin is not loaded is not there, and an address which was written by
hand in the configuration file, and which nothing declares, is left out on
purpose.

## General

| Entry                         | Widgets                        | Takes effect  |
|-------------------------------|--------------------------------|---------------|
| `font/textedit`               | a font button                  | nothing       |
| `shortcut style`              | `Emacs` or `Qt`                | nothing       |
| `secondary tasks orientation` | `Horizontally` or `Vertically` | a new buffer  |

The font of the main frames is also the font of the mode line and of the
minibuffer; it is what `Options > Main Frames' Font` chooses, and the
editor follows the font of the application as long as none is stored. The
key bindings style decides whether the line edits and the text areas
handle the usual `Qt` short cuts or keep the `GNU Emacs` bindings; the
menus are rebuilt as soon as it changes.

## Spelling and grammar

| Entry                         | Widgets                | Takes effect        |
|-------------------------------|------------------------|---------------------|
| `spelling/checker`            | `None`, `Hunspell`, `Hunspell and Harper`, `LanguageTool` | every buffer, at once |
| `hunspell default dictionary` | a one line edit        | nothing             |

What reads the words of the buffers: nothing, the dictionary of `hunspell`,
that dictionary and the sentences as read by `harper-ls`, or LanguageTool,
which says both and needs no dictionary. This is the address the
`Orthograph/Grammar` submenu of `Options` writes, and the menus of the
`C`/`C++`, `LaTeX` and `Markdown` modes write the same one. `Hunspell` is
the default and is kept by the *absence* of the address, which is what the
editor did before any grammar checker was added to it. See
[the grammar checking page](grammar-checking.html) for what each choice does
to a buffer, and for the one case where a buffer is not read with the choice
it was offered.

The language of the dictionary is an entry of this theme and not a theme of
its own: which program reads the words and which words that reading holds to
be spelled right is one decision. It is named as the installed dictionaries
name it, such as `en_US`; leaving it empty takes the language of the session,
and it is the same address the `Language` submenu of the `Orthograph/Grammar`
menu writes when `Hunspell` is the choice in use. A buffer which LanguageTool reads is
handed no dictionary at all, whatever is written here: that program checks
the spellings of the language it was given, and two sets of underlines of one
word say nothing one does not. The entry is there in a build which found
`hunspell`; where it did not, only the choice is shown.

`Harper` and `LanguageTool` are the submenus of this theme: what the program
which was chosen is, and what it is asked with, follows from the choice
itself.

## castem

| Entry               | Widgets                    | Takes effect    |
|---------------------|----------------------------|-----------------|
| `castem/executable` | a one line edit            | the next session |

The program the `Cast3M` mode starts, both for the session the sending
commands of `Ctrl-C` run and for the notice `Ctrl-X H` asks: the same
program answers both questions, so the notices come from the release the
buffer is worked on with. A bare name is the one a terminal would find in
the places the session searches; a release which is not there is named by
its full path. The address is read every time a program is started, so what
is chosen here reaches the next command, no restart is asked for. And a
program which will not start is named by the message the minibuffer shows,
which is the whole report of a wrong path: the mode launches what is
written and nothing else.

## clangd

| Entry                      | Widgets             | Takes effect             |
|----------------------------|---------------------|--------------------------|
| `clangd/enable`            | a check box         | a new buffer             |
| `clangd/start-by-default`  | a check box         | the next file opened     |
| `clangd/executable`        | a one line edit     | the next session         |
| `clangd/arguments`         | a list of strings   | the next session         |
| `clangd/semantic-tokens`   | a check box         | the buffer as it settles |

What the `C`/`C++` mode asks the language server, and what it asks it
with. See [the `clangd` support page](clangd-support.html) for what each
of them does: a mode which is already attached to a buffer was built with
the answer of `clangd/enable`, and `Start clangd` refuses a session while
it is off; the program and its arguments are read when a session is
started; `clangd/start-by-default` decides whether a file of a project
which has a `compile_commands.json` starts a session which reads it, and
is read again on every file opened. The `Color identifiers` entry of the
`C/C++` menu writes the same address as `clangd/semantic-tokens`.

## latex

| Entry               | Widgets  | Takes effect     |
|---------------------|----------|------------------|
| `latex/interpreter` | a choice | the next compile |

Which program the `LaTeX` mode starts when a document is compiled
(`Alt+M`): `latex`, which writes a DVI file, `pdflatex`, which
writes a PDF file of the file it is given, or `xelatex`, the engine of
Unicode and of the fonts of the system. The entry is named after the family
of programs, as the theme of the `Cast3M` mode is named after `castem`: the
three are programs a TeX distribution installs under those names, and the
combo box offers them under those names and no other. `latex` is the default
and is what a configuration file which says nothing answers with: it is the
program the mode ran before there was a choice to make.

The `Interpreter` submenu of the `LaTeX` menu writes the same address, and
the address is read when a process is started, so the next compile uses what
was chosen — no restart and no new buffer is asked for. A document whose
class is unknown, or one which loads `fontspec`, is compiled with the engine
it needs, and that is a decision of the document, not of the editor: the
menu names the choice for the whole session.

## pylsp

| Entry                      | Widgets           | Takes effect              |
|----------------------------|-------------------|---------------------------|
| `pylsp/executable`         | a one line edit   | the next session          |
| `pylsp/arguments`          | a list of strings | the next session          |
| `pylsp/start-by-default`   | a check box       | the next file opened      |
| `pylsp/linters/ruff`       | a check box       | the next session           |
| `pylsp/linters/pyflakes`   | a check box       | the next session           |
| `pylsp/linters/pycodestyle` | a check box      | the next session           |
| `pylsp/linters/flake8`     | a check box       | the next session           |
| `pylsp/linters/pylint`     | a check box       | the next session           |
| `pylsp/linters/mccabe`     | a check box       | the next session           |
| `pylsp/linters/pydocstyle` | a check box       | the next session           |
| `pylsp/linters/pylsp_mypy` | a check box       | the next session           |
| `pylsp/formatter`          | a choice          | the next session           |
| `pylsp/import-sorter`      | a choice          | the next session           |

What the `Python` mode asks the language server, what it asks it with, and
which programs that server runs over the text of a buffer. See
[the `Python` support page](python-support.html) for what each of them does.
The program is `pylsp`, the `python-lsp-server` package; the default of the
entry is empty, which is that name on the search path of the session, and a
server installed in a virtual environment is named by the full path of its
`pylsp` — which is what makes the answers of the buffers read that
environment's packages. The two entries which name the program and its
arguments are read when a session starts, so what is chosen here reaches the
next `Start pylsp` and no session which runs. `pylsp/start-by-default` decides
whether opening a file of this language starts a session of its own, and is
read again on every file opened: turning it off is answered by the next file
opened, not by the session a buffer is already attached to, since the project a
session reads is chosen when it starts. Unlike `clangd`, the default of that
entry is *on*: this server needs no compilation database to say what a name is.

The eight check boxes are one question per program the server can run over a
buffer, and several of them on at once is the norm, since each sees a mistake
the others do not; only `Ruff` is on by default, as the plugin of that name is
by its own account, and it reports what `Pyflakes` and `Pycodestyle` report in
one program which is faster than either. The formatter and the sorter are one
exclusive question each: every formatter of the server answers the same
request and the server keeps the first answer which holds something, so two on
at once is one answering and the other silent, and `None` is the choice of a
person who formats outside the editor.

The `Linters`, `Format with` and `Sort imports with` submenus of the `Python`
menu write these same addresses and hand what the settings now say to the
running session, which is what makes the underlines of the buffers follow a
click at once. What is chosen in this dialog reaches the *next* session: a
server which is already up reads the plugins it was started with until some-
body tells it otherwise, and the menu is the one which tells it.

## python

| Entry               | Widgets         | Takes effect  |
|---------------------|-----------------|---------------|
| `python/executable` | a one line edit | the next session |

The program an interactive session of the `Python` mode runs: the line, the
region and the buffer are handed to it by the three keys of `Ctrl-C` and what
it answers stands in a secondary task of the buffer, as
[the `Python` support page](python-support.html) tells. A bare name is the one
a terminal would find in the places the session searches; the full path of the
interpreter of a virtual environment is what makes the session see the modules
installed there, where the buffer's imports reach. The default of the entry is
the program the mode would run with the address left empty, which is why the
field shows `python3` and an answer equal to it is removed from the file rather
than stored. The address is read on every launch, so what is chosen here is the
program the next `Ctrl-C Ctrl-L` runs, with no restart asked for; a program
which is not there is named by the message the minibuffer shows.

## Harper

| Entry                        | Widgets           | Takes effect                        |
|------------------------------|-------------------|-------------------------------------|
| `harper/executable`          | a one line edit   | the next session                    |
| `harper/dialect`             | a choice          | the buffers as they are read again  |

What the checker of sentences is, and which English it reads a text as: see
[the grammar checking page](grammar-checking.html). A variety chosen in the
`Language` submenu while `Hunspell and Harper` is the choice writes the same
address as `harper/dialect`, so the dialog and the menu are two hands on one
address. The program is
`harper-ls`, started with `--stdio`; left empty, the editor looks for that
name in the directories of the session, and a release which is not on the
search path is named by its full path. A session which runs under a program
the dialog no longer names is ended by the first request which follows the
change.

The theme is named after the program the editor starts, as the `clangd` and
`castem` themes are, but it is a theme of the *editor*: `harper-ls` checks
prose, and the prose of this editor is read in the core library, in the three
modes which hold a buffer of text. It is shown whether or not a `harper-ls`
is installed, since an entry which is read by the editor is an entry whatever
the machine holds — only the menus disappear when there is no program to
launch. It is a submenu of `Spelling and grammar`, and it is spelled as the
program is spelled; the addresses of its entries keep the lower case name of
the program, which is what a configuration file written before the theme had
a name of its own holds, and what `harper-ls` itself is called by.

## LanguageTool

| Entry                                | Widgets           | Takes effect               |
|--------------------------------------|-------------------|----------------------------|
| `language-tool/installation`         | a one line edit   | the next server            |
| `language-tool/java-runtime`         | a one line edit   | the next server            |
| `language-tool/port`                 | a spin box        | the next server            |
| `language-tool/java-options`         | a list of texts   | the next server            |
| `language-tool/language`             | a one line edit   | the next request           |
| `language-tool/enabled-categories`   | a list of texts   | the next request           |
| `language-tool/disabled-categories`  | a list of texts   | the next request           |

Where the program is, what runs it, and what of its rules reach a text. The
server is a Java archive, `languagetool-server.jar`, and the two entries
which name a path have to be filled on a machine where the program is a
download: the installation is looked for in the directories named above, the
runtime in the directories of the session and in `JAVA_HOME`, and neither of
those guesses is worth trusting for a program which is not packaged.

What is stored under `language-tool/port` is the port the person asked to
start the server on, and zero is the entry which says *take a free one*: the
free number is picked when the process is started, which is what lets two
editors of one session run two servers and collide with nothing. The
`language` entry is the same address the `Language` submenu writes while
LanguageTool is the choice in use, and any code the program knows can be
written here, not only the thirty the submenu offers. Like `Harper`, this
theme is a submenu of `Spelling and grammar`.

Those themes — `Spelling and grammar`, with `Harper` and `LanguageTool`
under it — are themes of the editor, and are there whatever plugins
are loaded. Only `clangd`, `castem`, `pylsp` and `python` are themes a major
mode exports: the mode `C++` asks the language server those five entries and
nothing else, the `Cast3M` mode launches one program, the `Python` mode asks
its server what reads the text and what rewrites it and launches one program
of its own, and every other mode of the editor reads nothing from the
configuration file but the history of the command its prompt proposes, which
is not an entry of the configuration.

An address is shown once, in the first theme which declared it: the modes
which derive from the `C++` mode, such as `MTest` and `PipeTest`, use the
entries of the `clangd` theme as well, and those are not shown a second
time under their own name.

# Editing the file by hand

Every entry the dialog shows can be edited in the configuration file
itself, which is a plain text file. Two details of that format, which is
INI under GNU/Linux, are worth knowing:

- a list of texts is stored as texts separated by a comma, and a list of
  a single entry is stored as a plain text: the editor answers both
  readings with the same list;
- a font is stored as the `Qt` variant of that name spells it, which is
  not something worth typing by hand.

An entry which is stored with the value the editor would use anyway is
harmless but pointless: the dialog removes those addresses as soon as it
is asked to write.

# Declaring the entries a mode uses

The entries a configuration dialog shows are *declared*, never
discovered: `QSettings` answers the addresses which were written to, which
says nothing about the addresses the editor reads, and a dialog built on
it would show a typo in the configuration file as an entry of the editor.

An entry is described by a `SettingsKey` (`TFEL/GUI/SettingsKey.hxx`): the
address it lives at, the name it is shown under, a sentence which says
what it is for, the kind of value it holds, and what it answers when
nothing was stored. The declaration sits with the code which reads the
entry, which is what keeps the two from drifting apart.

A theme of the editor itself is registered by a `StandardSettingsGroupProxy`
at the bottom of the source file which holds that code, in the same spirit
and by the same mechanism as the commands of `CommandFactory` and the modes
of `MajorModeFactory`; see `src/CoreSettings.cxx`.

A theme which is a corner of another one says so with the fourth argument of
that declaration, and a theme a mode exports through its own
`SettingsGroup::parent`. `SettingsRegistry::getGroups` answers such a theme
directly after the theme it names, whatever file declared it and whatever
static initialiser ran first, and the dialog opens it as a submenu of that
theme. A theme whose parent is not declared keeps its own place: the relation
is a promise about the list, never a way to hide a theme.

A major mode exports its entries through a static `declareSettings` method
returning a `QVector<SettingsGroup>`. Its proxy reports them, filling the
name, the description and the icon of any theme which names none of its own
from the mode itself — which is how a mode whose entries are about a
program it drives names its theme after that program:

~~~~{.cxx}
  QVector<SettingsGroup> CxxMajorMode::declareSettings() {
    auto clangd = SettingsGroup{};
    clangd.name = QStringLiteral("clangd");
    clangd.description = QObject::tr(
        "What the C++ mode asks the language server, and what it "
        "asks it with. A session reads the program, its arguments "
        "and the compilation database when it is started: what "
        "changes here reaches the next `Start clangd`");
    clangd.keys =
        QVector<SettingsKey>()
        << SettingsKey::boolean("clangd/enable",
                                QObject::tr("Ask clangd anything"),
                                QObject::tr("When this is off, the C++ "
                                            "mode highlights and indents "
                                            "its files and asks nothing to "
                                            "anyone"),
                                true)
        // ... the four other entries the clangd support reads
        ;
    return QVector<SettingsGroup>() << clangd;
  }
~~~~

A mode which uses no entry declares nothing and is left untouched: no
theme of its own appears in the dialog. An entry which names no theme
belongs to the theme it is declared in, which is what the mode above makes
the four `clangd` addresses do.

The dialog of the configuration exposes its fields as `value` and
`setValue` and its writing as `apply`, which answers the addresses it
wrote, so that it can be driven from a test without a window manager: see
`tests/TestSettings.cxx`.
