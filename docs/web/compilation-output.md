This page describes what the output of a compilation is, where it goes,
and which buffers get to see it.

# Running a compilation

`Alt+M` asks in the minibuffer for the command which compiles the current
buffer — the guess it proposes is what the major mode of that buffer
says, the `C++` mode proposing a compiler, the `CMake` and `Makefile`
modes proposing a build tool — and starts it when the answer is
validated. The same command exists as
`launch-command > run-compilation`. The chosen command is remembered per
language, so the next `Alt+M` proposes the command last used for that
language.

The output does not replace the buffer: it is written in a *secondary
task*, a read-only text area shown beside the text — below it by
default, to its right when `Ctrl-X+3` is used — with a tab of its own
naming the program which ran. `Ctrl-X+O` cycles between the text and
that area, `Ctrl-X+1` hides it again.

The lines of the output which name a file and a place in it — what a
compiler prints when it fails — are underlined. Following one of them,
by clicking it or by pressing `Return` with the cursor on it, opens that
file and puts the cursor on the line the message is about. The icon of
the tab shows whether the last run succeeded.

# Which buffers see the output

A compilation is about a *project*, not about the single file that
happened to be current when `Alt+M` was pressed: an error in one
translation unit usually means work in a header, and the header is open
in another buffer. The editor therefore shares the output of a
compilation with every buffer editing a source file of the kind that
output is about.

Concretely, the output of a compilation appears in the secondary tasks
of:

- the buffers editing `C` and `C++` — the modes which know how to compile
  what they edit: `C++`, `MFront`, `MTest` and `PipeTest`;

and it does not appear in the buffers which edit anything else: a
`LaTeX` report, a `Markdown` note, a `Makefile`, a shell script, a
directory listing, a buffer visiting a file no major mode claims or no
file at all. What decides is the *category* of the major mode of the
buffer, which each mode states for itself: the modes which edit the
source of a compiled program, the modes which edit prose, and the others.

The sharing is decided twice, so its outcome does not depend on the
order in which things happened:

- when the output is created, it reaches all the programming buffers
  which exist at that moment;
- a buffer opened afterwards, or a buffer whose major mode becomes one of
  those modes (`M-x c++-mode`, or `Options > Major mode`), is given the
  outputs which are already displayed.

It is the very same area in all those buffers, not a copy: closing the
tab of one buffer does not destroy a process another buffer still
displays, and the output keeps running while a compilation is being read
in whichever file one happens to be editing.

Two things follow from this and are deliberate:

- The other secondary tasks stay where they were asked for. The output of
  a `grep`, of an `MTest` study or of a `Cast3M` session belongs to the
  buffer which launched it: those are not compilations, and sharing them
  would only move unrelated text from buffer to buffer.
- A buffer which *leaves* the programming category, by changing its major
  mode, keeps the outputs it already holds. Removing a secondary task is
  what closing its tab is for (`Ctrl-X+k` on it, or the `x` of the tab):
  an output which no buffer holds anymore is destroyed, running process
  included.
