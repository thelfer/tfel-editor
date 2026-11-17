The `emacs` shortcuts mimics the shortcuts used by the `GNU Emacs`
editor.

Those shortcuts are defined in `src/TextEditBase.cxx`. They apply
to the main frame of a buffer or to secondary tasks derived from
`TextEditBase`, i.e. most secondary tasks displaying pure text.

# Editing

- `Ctrl-X+P`: Print the main frame or the current secondary task.
- `Ctrl-X+R`: Enter rectangle editing mode, which waits for `t` to insert a
  string on every line the selection covers, `k` to kill that rectangle, `w` to
  keep it without deleting it, and `y` to insert the last kept one. See [the
  rectangle editing page](rectangle-editing.html) for details.
- `Ctrl-X+K`: From the main frame, kill the current buffer. Kill the
  current secondary task otherwise.
- `Ctrl-X+H`: Select the whole text.
- `Ctrl-X+Ctrl-S`: save the current frame
- `Ctrl-X+Ctrl-F`: open a new file
- `Ctrl-X+Ctrl-C`: close `tfel-editor`
- `Ctrl-X+Ctrl-W`: save the current frame as new file. TODO: if a
  secondary task, turn it into a new frame in a new buffer.
- `Ctrl-C+Ctrl-C`: save the current frame. `Ctrl-C` is the prefix a
  major mode uses for its own second keys and no mode asks for this one.

# Searching and replacing

- `Alt+%`: Query replace: asks a text and its replacement in the minibuffer,
  then walks the matches of the buffer and asks `y`, `n` or `!` for each one.
- `Ctrl-Alt+%`: The same three questions, the first answer read as a regular
  expression, whose groups the replacement may name as `\1` to `\9`.

Both are described on [the searching page](searching.html), together with the
dialog of `Edit > Find/Replace...` which asks no key of the buffer and stands
modeless while the buffer is read. The histories of the two questions are
written as the commands are used and are not entries of the configuration
dialog.

# The `Ctrl-C` prefix

The keys which follow `Ctrl-C` belong to the major mode of the buffer, and
a mode which claims one of them says what went wrong itself when it can
not do what the key asks. The modes which bind that prefix are the two
which talk to an interpreter of their own, and they use the same three
keys:

- `Ctrl-C+Ctrl-L`: send the line the cursor stands on to the interpreter,
  of the `Python` mode or of the `Cast3M` mode.
- `Ctrl-C+Ctrl-R`: send the selected region to that interpreter.
- `Ctrl-C+Ctrl-B`: send the whole buffer to that interpreter.

What the interpreter answers is shown in a secondary task of the buffer.
See [the `Python` support page](python-support.html) and
[the configuration page](configuration.html), which tells what the `Cast3M`
mode launches.

# Display

- `Ctrl-X+1`: Dide secondary tasks.
- `Ctrl-X+2`: Display secondary tasks under the main frame.
- `Ctrl-X+3`: Display secondary tasks on the right of the main frame.

# Navigation

- `Ctrl-X+B`: Change buffer.
- `Ctrl-X+O`: Cycle through the main frame and the secondary tasks.
