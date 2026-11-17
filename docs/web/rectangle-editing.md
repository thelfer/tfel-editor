Rectangle editing works on a block of columns rather than on a run of
characters: the lines between the two ends of the selection, and the columns
between the columns of those two ends. Three commands of `tfel-editor` act on
such a block, all under `Ctrl-X R`:

| Shortcut | Command | What it does |
|----------|---------|--------------|
| `Ctrl-X R T` | | asks a string and writes it on every line of the rectangle |
| `Ctrl-X R K` | `cut-rectangle` | removes the rectangle from the lines it covers and keeps it |
| `Ctrl-X R W` | `copy-rectangle` | keeps the rectangle the same way, without taking it from the buffer |
| `Ctrl-X R Y` | `paste-rectangle` | writes back what was kept, at the cursor |

The three commands which take or keep a rectangle are also the three entries
`Cut Rectangle`, `Copy Rectangle` and `Paste Rectangle` of the `Rectangle Edit`
submenu of the `Edit` menu, and the commands `cut-rectangle`, `copy-rectangle`
and `paste-rectangle` of `Alt-X`, which run on the buffer on the screen.
`kill-rectangle`, the name `GNU Emacs` gives to the first of them, is registered
as an alias of `cut-rectangle` and answers the same command. A rectangle is what
a selection says it is, so a menu entry needs no argument: the selection of the
buffer is the rectangle.

`Ctrl-X R` opens a minibuffer input of its own, which shows `Ctrl-X r ` and
waits for a single key: `t`, `k`, `w` or `y` runs its command at once, without
`Return`, and `Ctrl-G` cancels. The buffers do not answer any other key while
that input stands, so a mistyped key costs nothing. `t` then asks the string in
the minibuffer.

The three commands, which lines and columns a rectangle covers, and where the
cursor stands when they are done are described here, and checked against
`GNU Emacs` 27.1 command by command. The two places where the editor answers
differently from `Emacs` are named at the end of the page.

# What a rectangle covers

- the **lines** are all the lines the selection stands on, the line of either
  end of the selection included: a selection which stops at the beginning of a
  line takes that line with it;
- the **columns** are the columns of the two ends of the selection, from the
  leftmost to the rightmost, so the order the ends were made in does not
  matter;
- the last line of a buffer is one of the lines the selection can reach, even
  when it is empty, which is what the last line of a file ending with a
  newline is. That line is treated as any other: a string inserted there is
  written on the line which is already there rather than opening a new one, and
  so is a pasted rectangle. An answer of several lines opens as many lines as it
  needs, on that line as on any other.

A selection of a few characters inside one line is therefore a rectangle of one
line, and a selection which spans five lines takes those five lines. Setting
the mark with `Ctrl-Space` and moving the cursor further down or up is how one
draws a rectangle over several lines, and `Ctrl-X H`, which selects the whole
buffer, is the rectangle of every line of the file.

# Inserting a string

`Ctrl-X R T` asks a string in the minibuffer and writes it on every line of the
rectangle, at the left column of the rectangle:

- a line which does not reach that column is padded with blanks up to it;
- what the line holds from the right column of the rectangle onwards is pushed
  behind the string, so inserting inside a line *splits* it rather than
  overwriting what stands there;
- when the answer itself spans several lines, one whole answer is written on
  every line of the rectangle, not one answer line per line of the buffer.

When the command is done, the cursor stands just behind the text which was
written on the last line of the rectangle.

# Killing a rectangle, and writing it back

`Ctrl-X R K` removes the rectangle from the lines it covers and keeps it. The
killed rectangle is **not** put on the kill ring: `Ctrl-Y` knows nothing about
it, `Ctrl-X R Y` is the only way to put it back, and what that puts back is the
rectangle, whatever was killed or copied since.

`Ctrl-X R W` keeps the rectangle **without taking it from the buffer**: the
lines are read and kept and nothing is written, so the buffer holds what it held
and the cursor stays where it stood, which is the one thing that tells this
command apart from a kill the reader undid themselves. What it keeps is the same
rectangle the kill keeps, padded the same way, so a paste answers the two
alike — `copy-rectangle-as-kill` of `rect.el` writes `killed-rectangle` and
nothing else, and the command does the same.

Two things the command does on its own account are worth knowing, because both
are what `Emacs` does and neither is what a first guess gives:

- a line shorter than the right column of the rectangle keeps what it holds and
  loses only what stands inside the rectangle; the rectangle which is kept is
  right-padded with blanks, so a paste of it brings the width of the block back;
- the cursor stays on its **own** line — which need not be a line of the
  rectangle at all — and moves back by the width of the rectangle in front of
  it.

`Ctrl-X R Y` writes the kept rectangle back at the cursor's **own line and
column**: the top line of the rectangle is written at the column the cursor
stands on, the next line of the rectangle one line below, and so on. Lines
which do not exist yet are opened; a line which does not reach that column is
padded with blanks and the text is written behind those blanks. The cursor ends
up below and to the right of what was written.

A rectangle of no width — both ends of the selection in the same column — is
inserted like any other, which makes `Ctrl-X R T` a way to write a string on
several lines at once at the same column. Killing such a rectangle takes
nothing from the lines and keeps a piece of no width per line, which a paste
writes as nothing.

# Two differences from GNU Emacs

The behaviour of these commands is checked against `GNU Emacs` 27.1, on shapes
built by pressing the keys and reading the buffer and the cursor back — the
behaviour of a rectangle command is not something a description pins down, and
where the two disagree the editor is wrong. Two differences of shape remain:

- **The columns of a rectangle are plain character columns, not columns of the
  display.** `Emacs` measures a rectangle on the screen, where a tab moves to
  the next tab stop; the editor counts the characters of the line, where a tab
  is one of them. The two readings agree on every line which holds no tab, and
  on a line which does they cut in different places. For a buffer of two lines,
  `a`-tab-`Hello World` and `bbbbbbb World`, with the point just before the `o`
  of `Hello` and the mark just before the space which follows the seven `b`s,
  inserting `XY` gives:

  ~~~~{.text}
  this editor                  GNU Emacs
  a<TAB>HellXY World           a      XYo World
  bbbbbbXY World               bbbbbbbXYd
  ~~~~

  The ends name the character columns 6 and 7, which is where the editor cuts
  both lines. `Emacs` reads the same two ends as the screen columns 12 and 7 —
  the point before the `o` has been moved to the twelfth place by the tab — and
  cuts both lines at those, which is why its first line keeps nothing of the tab
  and stands padded with the blanks it is cut between, and why its second line
  loses ` Wor`.
- **There is no rectangle marking** (`Ctrl-X SPC`), so a rectangle is always
  what the selection says it is, and the only way to get the end of a line as
  the right edge of a rectangle is to put the cursor there.
