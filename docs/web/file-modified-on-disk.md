This page describes how `tfel-editor` watches the file a buffer visits, so
that a file written by another program while you edit it never silently
loses either party's work.

A file you edit in the editor can be written over at any time by something
else: a `git` checkout, a build rule, another editor still open on it, a
script. The editor compares the state of the file on disk — its
modification time and its size — with the state it had when the buffer read
or wrote it, once per second, for every buffer it holds.

# The warning banner

As soon as the file on disk differs from the file the buffer knows, a
banner appears above the buffer:

> **file `/path/to/file.cxx` has been modified on disk**  `[Reload] [✕]`

It says what happened and offers the two ways out:

- **Reload** replaces the text of the buffer by the file on disk. The
  modified flag is reset and the cursor stays on the line it was on; the
  text the reload discarded is still in the undo history (`Ctrl-/`), so a
  mistaken reload is not a loss.
- **✕** dismisses the warning. Nothing else changes: the buffer keeps its
  text, and the banner stays hidden until the file is touched again. The
  dismissal does not permit anything either — a save still confirms before
  overwriting the changes on disk.

A buffer whose file nobody touched shows no banner; a file which was
deleted is left alone rather than reported as modified.

While the banner is up, the buffer is **read-only**: the text can not be
changed until the disagreement is settled one way or the other, so that
overwriting what the other program wrote stays a decision and never a slip
of the fingers. Reading is not affected — the cursor moves with the arrow
keys, `Home`/`End`, `PageUp`/`PageDown`, text can be selected with the
keyboard or the mouse and copied, and the buffer can still be saved: the
freeze stops keys that would mutate the text, not the decision the banner
is waiting for. Dismissing the warning with **✕** gives the buffer back
for editing, exactly as reloading does.

# Saving over a modification

Whatever the banner was told, saving a modified buffer whose file changed
on disk never writes without a confirmation, because the write would
discard what the other program put there:

- A save from the menus — *Save current buffer*, *Save all buffers* — is
  a click, and a click can wait on a dialog: a message box asks whether
  the file shall be saved anyway, with a **Cancel** that leaves both the
  file and the buffer untouched.
- A save from the keyboard — `Ctrl-X Ctrl-S` or `Ctrl-C Ctrl-C` — asks in
  the minibuffer instead, in the style of every other question of the
  editor:

  > file '/path/to/file.cxx' has been modified on disk, overwrite ? **(y or n)**

  Answering `y` writes the buffer to the file, changes of the other
  program included; answering `n`, or cancelling with `Ctrl-G`, saves
  nothing and leaves the banner up as the reminder that the disk and the
  buffer disagree. Re-adopting the file instead of overwriting it is what
  the **Reload** button of the banner is for.

A buffer which visits no file, or a file the disk does not know yet, is
not a conflict: the first save of a new file asks nothing.

# Whose state is the reference

The reference is the file as this buffer last saw it: reading a file
when the buffer is created, or writing one on a successful save. A buffer
which is told to write under a new name takes the new file's state as its
reference at that moment, so the difference with the file it leaves behind
is never reported as an external modification.

Two buffers visiting the same file are not protected from *each other* by
this mechanism: the check sees that the file changed, and that is exactly
the situation where the confirmation of a save earns its keep, but the
banner is the tool for the outside world — the write of a sibling buffer
is the editor's own doing, and saving through it hides that buffer's
banner.
