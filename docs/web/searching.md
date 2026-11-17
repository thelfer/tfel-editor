# Searching and replacing

Three commands of `tfel-editor` look for a text in a buffer and write something
in its place. One is a dialog which stays on screen, two are conversations held
in the minibuffer, one of a text, one of a regular expression:

| | Reached by | Asks | Answers |
|---|---|---|---|
| the Find/Replace dialog | `Edit > Find/Replace...`, `Alt-X find-replace` | the text and its replacement, in its own fields | the matches walked by the buttons, one press per match |
| the query replace | `Alt-%` | the same two answers, in the minibuffer | the matches walked by the keys of the buffer |
| the query replace of a regular expression | `Ctrl-Alt-%` | an expression instead of a text, and a replacement which may name its groups | the same walk |

All three read a buffer, and all three can write one. A buffer which can not be
written to — one frozen by a file modified on disk, see [that
page](file-modified-on-disk.html) — is searched and walked by the dialog all the
same, and the two commands which replace are refused there with `read only
buffer`, since the freeze exists to keep a key from writing into a buffer whose
file holds something else.

The dialog, its three options, the reports it makes and the groups of a
replacement are described on [the find and replace
page](find-replace.html), which describes the two query replaces too: the two
are asked the same questions, and what the two have apart is named there.

# What the query replace answers

The shortcut opens the question of the text to replace, and its answer is named
back in the question of the replacement: `query replace abc with :`, or
`query replace regexp a.c with :`. The third stage hands the buffer's own keys
to the walk of the matches, and the minibuffer says what the walk did:

- `y` replaces the match on the screen and brings the next one;
- `n` passes the match over and brings the next one;
- `!` replaces the match on the screen and every match behind it;
- `Ctrl-G` ends the command with nothing more written and says how many matches
  had been replaced up to that point — the keys of the walk only ever moved a
  selection, so nothing has to be taken back;
- any other key is reported as what it is, `type either 'y', 'n' or '!'`, and
  writes nothing.

The report at the end names what was written: `replaced 3 occurrences`, in the
singular for one (`replaced 1 occurrence`), and `replaced 0 occurrences` for a
walk which was ended before the first `y` — so the keys spent moving a selection
through the buffer are not read as keys which wrote it. The other lines the two
commands write there are the refusals: `'abc' not found` for a pattern the buffer
holds nowhere, which ends the command where it began; `empty entry` for an empty
first answer with nothing remembered to reuse; `type either 'y', 'n' or '!'` for
a key which is no answer of the walk; and `read only buffer` for a buffer which
can not be written to.

An empty first answer of a command which was run before reuses the pair of
answers of that run, which is what makes the shortcut a way of doing the same
replacement again in another buffer.

# Which of the two commands is the other

`Alt-%` reads its first answer as the characters it holds; `Ctrl-Alt-%` reads it
as a `POSIX` extended regular expression, the syntax of `QRegularExpression`,
where `[a-z]+`, `^#` and `a{2,4}` name what they name in most programs and an
expression which does not compile is refused with the message the engine makes of
it, the question coming back holding what was typed. That is the whole of what
the two have apart, and it is enough to make the same characters two different
questions: `a.c` is a text of five characters to the plain command, and an
expression which an `abc` satisfies to the other.

The two commands keep two histories, since an answer with `\1` in the middle is
no answer to a search of plain text. The two lists live at
`user input/query replace/history` and `user input/query replace regexp/history`
of the settings file, and they are deliberately not entries of the configuration
dialog: the commands write them as they are used, and what the dialog shows is
what one chooses, not what the editor remembers.
