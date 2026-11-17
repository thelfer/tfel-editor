Searching a buffer, and replacing what is found, is done from a dialog
which stays on screen while the buffer is read: `Edit >
Find/Replace...`, or `Alt-X find-replace`. The dialog is one per window of
the editor, so calling it again brings back the same window with what it
holds, and the buffer answers the keys as usual while the dialog stands —
nothing is waited for.

The dialog searches the buffer on the screen, and follows the buffer: the
second buffer brought on the screen is what the next press of **Find next**
walks, with the text and the options the fields hold. When it is raised and the
**Find** field is empty, a text selected in the buffer is written in that field
— a selection which holds no newline, since a text of several lines is a
question about columns, not a text to look for.

# The fields and the three options

| | |
|---|---|
| **Find** | the text, or the expression, looked for |
| **Replace with** | what a match is written to |
| **Match case** | off by default: a search answers both spellings of a letter |
| **Whole words** | off by default: a match with a letter or a digit on either side is part of a longer word and is passed over |
| **Regular expression** | off by default: the **Find** field is read as an expression instead of as the characters it holds |

The whole-words rule counts a letter or a digit, so an underscore, a dash, a
space and the two ends of the buffer are all boundaries: `abc` of `abc_def` is a
whole word to this option, which is what the `\b` of an expression does *not* say,
since `\b` reads the underscore as a word character. The two options answer their
own question and are read on every press, so a search whose options were changed
under the hands is answered by the new ones, and a report which is not what the
text said names the option as surely as it names the text.

The line at the bottom of the dialog is where everything the commands answer
is written — the count of a replacement, a text which is not in the buffer,
an expression which does not compile. The minibuffer is left to the commands
of the editor: a dialog which answered in the minibuffer would answer under
a hand which is on the dialog.

# Walking the matches

**Find next** and **Find previous** put the next match of the walk on the
screen, and the walk goes on from where it stands: repeated presses walk the
buffer one match at a time, in either direction, and bring the cursor and the
selection to the match, so a click or a key in the buffer is a hand on the
text which was found. `Return` in either field is **Find next** and
`Shift+Return` is **Find previous**, which is what keeps a hand which is
writing an expression from having to leave the keyboard to walk a buffer.

A walk reaches the end of the buffer and turns around: when the matches
before the place it started from are what is left, the line says so — `'abc'
searched from the beginning of the buffer` — and the walk continues there.
One turn is allowed, so a buffer which holds the text nowhere at all gets
`'abc' not found` and a cursor left where it stood.

**Replace** writes the text of the **Replace with** field over the match the
walk stands on and brings the next match, which makes replacing a matter of
pressing the button as often as the matches to replace are; a match which is
left alone is passed over by **Find next**. **Replace all** replaces every
match of the buffer at once and says how many were replaced, which is the
number worth reading before a replacement of an expression is believed.

**Close** takes the dialog away, and everything it holds — the text, the
options, the place of the walk — is there when it is brought back, as is the
dialog itself: the editor holds one dialog per window, and the **Close** button
and `Escape` hide it rather than taking it away.

# What an expression is, and what its answer is

With **Regular expression** ticked, the **Find** field is read as a
`POSIX` extended regular expression, the syntax of `QRegularExpression`:
`[a-z]+`, `^#`, `(foo)(bar)?` and `a{2,4}` name what they name in most
programs. An expression which does not compile is reported with the message
the engine makes of it, and the search is not asked of the buffer.

The characters of a group of that expression are written in the **Replace
with** field as `\1` to `\9`, and a replacement is computed for each match, so
one answer rewrites the two matches of a buffer with the two answers of the
two matches:

| Buffer | Find | Replace with | Buffer after **Replace all** |
|--------|------|--------------|------------------------------|
| `a-1 b-2` | `([a-z])-([0-9]+)` | `\2:\1` | `1:a 2:b` |
| `foo foo` | `(f)(o+)` | `<\0>` | `<foo> <foo>` |

`\0` is the whole match. A backslash before anything else — before a letter, or
before another backslash — stands for the character it precedes, which is how a
replacement of a literal `\1` is written: `\\1`; and an answer which ends on a
backslash writes that backslash. The number of a group is read with at most one
digit, so `\12` is the first group followed by a `2`, which is what the two-digit
numbers of a group are not. A group which matched nothing is written as nothing.

Without **Regular expression** ticked, the **Find** field is the characters
it holds, backslashes included, and so is the **Replace with** field: a
replacement of `a\1b` writes those four characters, because a search of a
text has no groups to answer with.

# The query replace of the minibuffer

The dialog is not the only way to replace. Two commands of the editor ask the
same two questions in the **minibuffer** and then walk the buffer with the
keys of the buffer answered one match at a time, as `GNU Emacs` does:

| Shortcut | What the first answer is |
|----------|--------------------------|
| `Alt-%` | a text |
| `Ctrl-Alt-%` | a regular expression |

The shortcut opens the question of the text to replace, whose answer is then
named back in the question of the replacement: `query replace abc with :`, or
`query replace regexp a.c with :`. The answer of the second question may name
the groups of the expression, `\0` to `\9`, with the rules above.

The third stage is the walk, and it is a conversation with the buffer, not
with the minibuffer: the match is put on the screen and the keys are typed
into the buffer.

- `y` replaces the match on the screen and brings the next one;
- `n` passes the match over and brings the next one;
- `!` replaces the match on the screen **and every match behind it**, which is
  the answer of a reader who meant the whole buffer when the last match before
  it was reached;
- `Ctrl-G` ends the command with nothing more written, and the report says how
  many matches had been replaced up to that point — the keys of the walk only
  ever moved a selection, so nothing has to be taken back.

Any other key is reported as what it is — `type either 'y', 'n' or '!'` — and
writes nothing, which is the difference between a query replace and a typo.

An empty first answer reuses the pair of answers of the previous run of the
same command, and with no previous run says `empty entry` and leaves the
minibuffer back to the buffer. The two commands keep two histories, since an
answer with `\1` in the middle is no answer to a search of plain text, and both
histories are written under `user input/query replace/history` and
`user input/query replace regexp/history` of the settings file. Those two
addresses are deliberately not entries of the configuration dialog: the
commands write them as they are used, and what the dialog shows is what one
chooses, not what the editor remembers.

An expression which does not compile is the one answer the query replace of a
regular expression does not spend: the report is the message the engine makes
of it, and the question of the expression comes back holding what was typed, so
a missing parenthesis is put back rather than gone through again.

# Reading the reports

| Report | What happened |
|--------|---------------|
| `'abc' not found` | the buffer holds no match, and the cursor was left alone |
| `'abc' searched from the beginning of the buffer` | the walk turned around at an end of the buffer and is looking at the other half |
| `replaced 5 occurrences of 'abc'` | **Replace all** wrote five matches; one is `replaced 1 occurrence of 'abc'`, and none is `replaced 0 occurrences of 'abc'` |
| `replaced 'abc' with 'x'` | **Replace** wrote the match the walk stood on — of a regular expression, the second text is what the groups were written as, so an answer of `\2:\1` reports the `1:a` it made |
| `empty search` | nothing was typed in the **Find** field, so nothing was asked of the buffer |
| `read only buffer` | the two buttons which write were pressed on a buffer which can not be written to — a frozen buffer (see [the file modified on disk page](file-modified-on-disk.html)) is searched and walked all the same and takes no replacement |
| `unable to construct regular expression` | the expression of the **Find** field is what the engine refused, and nothing was asked of the buffer: the message is the engine's own, so `(a` is answered `missing closing parenthesis` |

The two keys of the query replace answer the same questions, and the reports they
make of the walk are `type either 'y', 'n' or '!'` for a key which is no answer
of it, and `replaced 3 occurrences` — the singular of one — of what was written.
