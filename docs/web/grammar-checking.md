This page describes the grammar checking of `tfel-editor`, which is
based on the [`harper`](https://writewithharper.com) language server,
[`harper-ls`](https://github.com/Automattic/harper).

`harper` is a grammar checker of English prose: it reads a text and says
what is wrong with the way it is written — an article which does not fit
the word it introduces, a verb which has to be the base form after `to`,
a pronoun which does not agree with its verb, a spelling which belongs to
another variety of English. It is not a spell checker: it knows what
words fit a sentence, not whether a name is in a dictionary. The editor
itself embeds no checker: everything the underlines of this page say comes
from the server.

The editor acts as a
[`Language Server Protocol`](https://microsoft.github.io/language-server-protocol/)
client, the same way it does with [`clangd`](clangd-support.html): it
starts `harper-ls`, keeps the text of the buffer in sync with it, and
turns what the server publishes into underlines and tooltips. A buffer of
`C++` can be read by both servers at once — they say different things —
and a buffer which was never saved has no name for the checker to be asked
under: its text is read as soon as it is a file.

# Main features

- **Lints**: what the checker finds is underlined in the text, in a plain
  red underline. Every lint of `harper` comes with the same severity, a
  hint, so the whole answer is drawn with one face rather than being
  sorted into colors by a scale the server does not use. That red is not
  the red of an error of `clangd`, which is darker: the two underlines can
  be told apart where they sit beside each other.
- **The message of a lint**: resting the mouse on an underlined word shows
  what the checker said of it, with the name of the rule it applied —
  `Did you mean to spell 'colour' this way? [SpellCheck]`, for instance.
  The name of the rule — `AnA`, `InflectedVerbAfterTo`,
  `PronounVerbAgreement`, `SpellCheck` — is what a person who thinks the rule
  is wrong looks up in the documentation of the checker, so it goes with the
  sentence and is not left out. Several lints of one run are said together,
  one per line.
- **The prose of a program**: a source file is read whole, so the comments
  of a `C` or `C++` file are checked like any other text, and the line of
  code around them is left alone. In a `LaTeX` or a `Markdown` buffer the
  whole text is prose.
- **A variety of English**: which spellings are held to be their own and
  which inflections are accepted depends on the variety the text is read
  as. The `English variant` submenu of a menu chooses between `American`,
  `Canadian`, `Australian`, `British` and `Indian` English; a session
  which runs reads its buffers again under the new answer, so `colour`
  stops being a lint as soon as the buffer is read as British English.
- **One switch, one address**: the `Grammar checking` entry of a menu is
  checked while a session runs, and triggering it both ends that session
  and writes the setting which tells a new buffer whether to take a
  session with it. The switch is the editor's, not the buffer's: a buffer
  opened afterwards follows it, and so does the menu of every other buffer.

The lints are drawn as a contribution of their own to the extra selections
of the text edit, the same mechanism the diagnostics, the navigation hints
and the colors of the identifiers of `clangd` use. A session which is
stopped, or a mode which is replaced, takes its own underlines away and
leaves the rest of the buffer untouched.

# Where it is offered

| Mode | Menu | Entry |
|------|------|-------|
| `C`/`C++` | `C/C++` | `Grammar checking` |
| `LaTeX` | `LaTeX` | `Grammar checking (English only)` |
| `Markdown` | `Markdown` | `Grammar checking (English only)` |

The `C`/`C++` mode has other entries of its own, and a checker which is
one entry among several does not need to explain itself. The two text modes
have the checker as their only specific menu, and their entry carries a
note: `harper` knows no word of French, and a document of French is
underlined by the spell checker and by nothing else. A person who turns it
on in a buffer of another language is left with an underlined text they did
not ask to be corrected.

Every one of those menus also holds the `English variant` submenu, and all
of them write the same address: the variety is one choice for the editor,
not one per buffer, which is what keeps the submenus of the buffers on
screen from disagreeing with each other.

A machine which has no `harper-ls` to start offers no entry at all: an
entry which could only report that the program is missing is no entry.

# One checker of a text at a time

The two checkers of the editor say what is wrong with a word in the same
place, one in the red of a spelling mistake, the other in the red of a
grammar mistake, and a buffer which shows both says nothing. So while a
grammar session reads a `LaTeX` or a `Markdown` buffer, the hunspell spell
checker is taken off that text: the underlines of the lints are the only
red on the screen, and the context menu stops proposing the replacements of
a word hunspell does not know. Turning the session off hands the text back
to hunspell.

A lint is not offered in the context menu, incidentally: the checker says
what is wrong with a word and leaves the pen to the person who wrote the
sentence, so the message of a lint is read where it is written, as a
tooltip.

What is taken away is the mode's use of the language, not the language of
the buffer: the frame keeps the dictionary it was given, which the
`Dictionaries` menu of `Options` writes and the frame reaches for again as
soon as the checker is off. The `C`/`C++` mode needs no such rule, since it
runs no spell checker at all.

# The program, and the settings

| Entry | Widget | Takes effect |
|-------|--------|--------------|
| `harper/launch-by-default` | a check box | a new buffer |
| `harper/executable` | a one line edit | the next session |
| `harper/dialect` | a choice | the buffers as they are read again |

Those three make up the `harper` theme of the
[configuration dialog](configuration.html), and the menus write two of
them. The program is `harper-ls`, started with `--stdio`; left empty, the
editor looks for that name in the directories of the session, and a release
which is not on the search path is named by its full path. The buffers
already open keep the session they were built with, which is why turning
the checker on is a thing one does to a buffer and turning it on by default
is a thing one does to the editor.

A session of a text mode is the mode's own: the editor does not send the
text of a `Markdown` buffer to the `clangd` session of some other buffer,
and a grammar session is asked nothing about `C++` files. Starting `harper`
does not start `clangd` and stopping one ends neither of the other; the two
sessions of a `C++` buffer are two programs and two handshakes.

## What the editor does not ask

The session is asked about the text of its buffer and nothing else. There
is no completion, no hover, no rename and no code action behind it: the
answers of `harper-ls` to those questions are either none of this mode's
business or a second round trip the editor would have to answer on the
server's behalf. Nor is the file watched from the outside: the checker sees
what the buffer holds as it changes, so saving is none of its business.

There is no command to launch or stop a session, either: the entry of the
menu is the door for a buffer and the setting is the door for the editor.
The [`clangd` integration](clangd-support.html) does have commands, because
a session which needs a compilation database to be chosen is a decision
worth making from the minibuffer; the grammar checker has no such choice to
make.

# Building, and testing

Nothing special: the grammar checker is part of the core library, and the
`LaTeX` and `Markdown` modes use it as they were already built. The tests
of the checker are `test-grammar-check`, which drives a real `harper-ls`
when one is installed and skips the cases which ask it anything when none
is; see [the unit tests page](unit-tests.html) for how to build and run
them. A machine which has no `hunspell`, or no `en_US` dictionary, skips
the one case which looks at what the two checkers do to each other.
