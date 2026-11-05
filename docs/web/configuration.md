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
it holds, since `clangd` and `castem` are names of programs and say
nothing of the mode which launches them.

The name of an entry stands
above the field which edits it, which has the width of the page to
itself — a path of a program is not something to read in a narrow box —
and the sentence which says what the entry is for is the tool tip of both.
Each entry is edited with the widget which suits the kind of value it
holds: a check box for a boolean, a spin box for a number, a one line edit
for a text, a multi line edit for a list of texts, a combo box for a
choice, a button which names the font and opens a font dialog for a font.

Nothing is written while one edits: the dialog writes when it is
accepted, since most entries are read once, when the widgets which use
them are built. The entries whose effect one has to see under one's hands
are applied at once: the font of the text edits, the style of the key
bindings, and the language the misspelled words are underlined with. What
is left waits for a new buffer, or for a restart of the editor, and the
minibuffer says how many entries were written.

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

## Spell checking

| Entry                         | Widgets         | Takes effect |
|-------------------------------|-----------------|--------------|
| `hunspell default dictionary` | a one line edit | nothing      |

The language the misspelled words are underlined with, named as the
installed dictionaries name it, such as `en_US`. Leaving it empty takes
the language of the session. This is the theme of a build which found
`hunspell`; it is the same address the `Dictionaries` menu of `Options`
writes, and the buffers follow it at once.

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
| `clangd/executable`        | a one line edit     | the next session         |
| `clangd/arguments`         | a list of strings   | the next session         |
| `clangd/semantic-tokens`   | a check box         | the buffer as it settles |

What the `C`/`C++` mode asks the language server, and what it asks it
with. See [the `clangd` support page](clangd-support.html) for what each
of them does: a mode which is already attached to a buffer was built with
the answer of `clangd/enable`, and `Launch clangd` refuses a session while
it is off; the program and its arguments are read when a session is
launched. The `Color identifiers` entry of the `C/C++` menu writes the same
address as `clangd/semantic-tokens`.

Those two are the themes the major modes export. A mode which uses no entry
of the configuration is not part of the dialog, which is the case of every
other mode of the editor: the only thing they read from the configuration
file is the history of the command their prompt proposes, and a history is
not an entry of the configuration.

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
        "and the compilation database when it is launched: what "
        "changes here reaches the next `Launch clangd`");
    clangd.keys =
        QVector<SettingsKey>()
        << SettingsKey::boolean("clangd/enable",
                                QObject::tr("Ask clangd anything"),
                                QObject::tr("When this is off, the C++ "
                                            "mode highlights and indents "
                                            "its files and asks nothing to "
                                            "anyone"),
                                true)
        // ... the three other entries the clangd support reads
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
