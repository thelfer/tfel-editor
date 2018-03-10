The `emacs` shortcuts mimics the shortcuts used by the `GNU Emacs`
editor.

Those shortcuts are defined in `src/QEmacsTextEditBase.cxx`. They apply
to the main frame of a buffer or to secondary tasks derived from
`QEmacsTextEditBase`, i.e. most secondary tasks displaying pure text.

# Editing

- `Ctrl-X+P`: Print the main frame or the current secondary task.
- `Ctrl-X+R`: Enter rectangle editing mode.
- `Ctrl-X+K`: From the main frame, kill the current buffer. Kill the
  current secondary task otherwise.
- `Ctrl-X+H`: Select the whole text.
- `Ctrl-X+Ctrl-S`: save the current frame
- `Ctrl-X+Ctrl-F`: open a new file
- `Ctrl-X+Ctrl-C`: close `qemacs`
- `Ctrl-X+Ctrl-W`: save the current frame as new file. TODO: if a
  secondary task, turn it into a new frame in a new buffer.


# Display

- `Ctrl-X+1`: Dide secondary tasks.
- `Ctrl-X+2`: Display secondary tasks under the main frame.
- `Ctrl-X+3`: Display secondary tasks on the right of the main frame.

# Navigation

- `Ctrl-X+B`: Change buffer.
- `Ctrl-X+O`: Cycle through the main frame and the secondary tasks.
