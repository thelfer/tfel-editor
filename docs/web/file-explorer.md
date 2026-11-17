The left hand side of the buffers can hold a tree of the file system: the
directories around the file you are working on, and the files of those
directories. It is brought on screen and taken away with
`Display > Hide/Show Explorer Panel`, the entry of a menu which stands between
`Edit` and `Buffers` — and a new window shows the panel only if the window
before it was closed with the panel open.

![The explorer panel on the left of the buffers, with the file its double click
opened on the screen](img/explorer-panel.png "The explorer panel, about a
quarter of the width given to the buffers, with notes.txt opened by a double
click on its name. The line of the buffer — name, position in the file, major
mode, time — and the line which reports what the editor did stand below it,
across the whole width of the window")

# What the panel holds

The tree shows the entries of one directory, named in the line above it. That
line holds the two ways out of the directory it shows:

| | |
|---|---|
| **Up** | shows the directory which holds this one |
| **Choose a directory** | asks for another directory through a file dialog, and roots the panel at the answer |

Below that line stand the sub-directories of the directory, in alphabetical
order, then its files, in the same order. Only the name of an entry is drawn:
the panel is a fraction of a window wide, and the size, the type and the date of
a file are none of its business. A directory names its own children once it is
opened — a double click on its name, or on the arrow beside it — and an entry
created, renamed or removed on disk reaches the tree by itself, without asking.
Files whose name begins with a dot are not among the entries.

# Opening a file

Double clicking on a file opens it in a buffer, exactly as `Ctrl-X Ctrl-F`
would, with its major mode chosen from its name. When the file is already
visited, the buffer which visits it is what comes to the screen: no second
buffer reads the file, and the edits it holds are what you see.

A double click on a directory opens nothing but itself; a double click on the
blank space below the entries opens nothing at all.

# How wide it is

The first time the panel is shown it takes about a quarter of the width and the
buffers get the rest. The handle between the two halves drags like any other: a
panel pulled to half the width stays there, and when it is hidden and brought
back it comes back **the width you left it**, not the quarter it was given the
first time. It can be shrunk to the two buttons of its header and widened as far
as the buffers allow.

And what you leave is what the next window finds. The panel is shown closed or
open as the window before it was closed — open in the last one, open in the new
one; and the width the handle was last left at is the width the new window
restores, not the default quarter. This remembering is the state of your last
session, not a setting: neither the open nor the width appears in the
configuration dialog, and there is nothing to configure — drag the handle, hide
or show the panel, and the next window obeys.

The panel is as tall as the buffers and ends where the text of the buffer on the
screen ends. **Below** it stand the two lines of the window, each across its
whole width: the line of the buffer, which names that buffer and says where the
cursor stands in it and what its major mode and the time are, and the line which
reports what the editor did and asks the questions of the minibuffer in it. The
width of the panel is therefore taken out of the row of the text alone, and a
long message is never cut by a panel which has nothing to say about it — nor is
the name of the buffer you are reading, which stays as wide as the window even
when half of that window shows the file system. The height of the buffers is
untouched by showing or hiding the panel.

The directory the panel is rooted at is the one which holds the file the buffer
on the screen visits — the working directory when that buffer visits no file —
and it is chosen when the panel is shown for the first time. It does not follow
the buffer afterwards: a root which jumps every time the buffer changes is no
help walking a tree you chose yourself. **Up** and **Choose a directory** are
the ways to walk elsewhere.

# The `Display` menu

| Entry | What it does |
|-------|--------------|
| Hide/Show Explorer Panel | shows the tree of the file system on the left of the buffers, or hides it |
| Hide/Show Auxiliary Tasks | shows the pages which stand beside the current buffer — the output of a compilation, a session of an interpreter, the list of the symbols of a file — or hides them |

Both entries are ticked when what they name is on the screen, and the answer is
asked again every time the menu is opened: a page hidden by its own key
(`Ctrl-X 1`) is not named as still open, and a page a compilation brought back
is not named as hidden. Hiding a panel changes no buffer, no mode, no setting:
what a compilation wrote to a page stays there when the page is brought back,
and the width a hidden panel had is what the editor holds for the next time.

An entry which is asked to show what does not exist says so — a buffer with no
auxiliary task answers `no SecondaryTask to be shown` and the entry stays
unticked, rather than ticking itself over an empty page.

# What it is, and what it is not

The tree is Qt's own view of the file system — a `QFileSystemModel` shown by a
`QTreeView` — so it is that model which watches the directories it has looked
into, answers the icons the desktop gives the entries, and decides their order.
There is no filter to edit, no ignore list, no setting: the file system is shown
as it is, minus the hidden files.

The panel is a widget of the editor, not a buffer of the editor. It is not in
the list of buffers, it can not be killed, it holds no major mode, and it is
none of what the `Buffers` menu is about. Its font is the font of the text
edits, which the font dialog changes for it as it does for the rest of the
window.
