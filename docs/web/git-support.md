When the file a buffer visits stands in the working tree of a
[`git`](https://git-scm.com) repository, the line under the buffer says so, in a
short word right beside the name of the buffer:

~~~~{.text}
main.c (git)          50%       (1,0)        C++        19:51
~~~~

That word says one thing: the file is a working file of a repository, because
the editor found the `.git` entry of one above the directory the file stands in.
The answer comes from the file system alone, so it costs nothing and is there
from the first moment the buffer is: no `git` is started and no subprocess is
waited for to draw it.

![The line of a buffer visiting `main.c` of a repository: the name of the buffer,
`(git)` beside it, the place of the cursor in the file, the major mode and the
time](img/git-indicator.png "A buffer of `main.c`, which stands under the
directory whose `.git` entry makes it a repository: the line names the buffer and
says what that file is")

What the file *is* to that repository — new, or different from what the
repository holds — is a second word, and it does need the program:

![The line of three buffers of one repository: `main.c`, which the last commit
holds unchanged, `matrix.c`, whose text has been edited, and `newton.c`, which no
commit holds](img/git-indicator-states.png "The three states of the line: `(git)` for a file the repository holds as it is, `(git:modified)` for a file it holds differently, `(git:untracked)` for a file nobody has added to it")

The same answer colors the [file explorer panel](file-explorer.html): the
entries of the directory it shows are drawn with what the repository says of
them.

![The window of the edited file: the panel of the repository on the left, with
`matrix.c` dark red and `newton.c` blue, and the line of the buffer under the
text saying `(git:modified)`](img/git-explorer-colors.png "One answer, drawn in two places: the panel colors the entries of the directory by what the repository says of them — `matrix.c` dark red because the working tree differs from the index and the commit, `newton.c` blue because neither a commit nor the index holds it — while `main.c`, `matrix.h`, `README.md` and the ignored directory `build` keep the color of the desktop")

# What the three texts say

| The line says | What it means |
|---|---|
| `(git)` | the file stands in the working tree of a repository, and the repository holds it as it stands on disk. It is also what the line says when no answer about the repository is held: no answer yet, or none asked for, which is what [the entry of the configuration](#the-two-entries-of-the-configuration) decides |
| `(git:untracked)` | neither the last commit nor the index holds the file. That is also true of every file of a directory nobody has added, and of a name `Ctrl-X Ctrl-W` has just written and nobody has added |
| `(git:modified)` | the repository holds the file differently from what it is: its text is not what the last commit has of it, or the index holds it differently, or the file is staged as an addition or a deletion, or it was renamed, or two versions of it stand there unresolved |
| nothing | the file belongs to no repository, or the buffer visits no file at all: the buffer of a new file, the output of a compilation, a session of an interpreter |

So the word answers one question about one file — what is it to its repository —
and says nothing else: no branch, no count of differences, nothing about the
rest of the tree. The `nothing` of the first table row matters as much as the
texts: a file of no repository is named plainly, with nothing beside it, so the
two are told apart at a glance.

The texts are a label of their own, right behind the label of the name they
qualify, so the pair reads as one item and the name stays the name. The name of
a buffer is what identifies it — its tab, the list `Ctrl-X b` walks, the signal
the editor emits when it is renamed — and what the visited file happens to be
has no business in it. `assignSuffixes`, which gives buffers a suffix when two
of them visit files of the same name, would otherwise move a `(git)` into a name
nobody typed, and the same file would give a buffer two names, one before
`git init` and one after.

# The colors of the panel

The entries of the [file explorer panel](file-explorer.html) wear the same
answer:

| The entry is | Because |
|---|---|
| blue | neither a commit nor the index holds the file, nor the directory it stands in |
| dark red | the repository holds the file differently — edited text, staged addition or deletion, rename, conflict |
| the color of the desktop | everything else: an entry the repository holds unchanged, an entry of no repository, an ignored file, and every entry of a repository the editor was told to ask nothing about |

What is said of a directory is said by the program, and only there: a tree of
new files is answered as **its top directory**, so that entry is blue as the
files inside it are, and a file of such a directory is answered untracked too,
because the directory which holds it is. A directory which holds an edited file
is answered nothing — `git status` names the file and not the directory it
stands in — and the panel leaves it the color of the desktop, since a red
invented for it would say the directory itself differs, which is a thing the
program never says. Ignored files are left the color of the desktop on purpose:
`git status` names no file which is not its business, which is the whole of what
makes ignoring a file different from deleting it, and a gray invented for the
case would say what the program does not say.

# What the editor runs, and when

One program, one question, asked on a timer and at the moments the editor
knows the tree changed:

~~~~{.bash}
git status --porcelain -z --untracked-files=normal
~~~~

started in the root of a repository the window looks at, with
`GIT_OPTIONAL_LOCKS=0` in its environment so that the question does not refresh
the index of a tree somebody else may be working in. That is the format which
does not change between versions of the program; `-z` is what keeps a name with
a space, or with a byte above 127, out of the quotes and octal escapes the
default spelling of `git` writes; and `--untracked-files=normal` is asked
explicitly so that a `status.showUntrackedFiles=no` in somebody's configuration
does not silence the new files this is about.

- **Every five seconds**, once per repository the window looks at — the
  repositories of the files the buffers visit, and that of the directory the
  panel is rooted at. One question at a time per repository: a tree which is
  slow to answer costs one process, not a new one on every tick.
- **And at the moment a buffer writes one of its files**: saving a file, writing
  it under a new name, and reloading a file another program wrote all ask the
  repository of that file again at once. A save is the moment you look at the
  line to see what the change made, and five seconds is long enough for that
  wait to read as a save which went unnoticed. A question asked while a run is
  going is kept and spent when that run ends, so a save which lands in the
  middle of a poll is not a save which was not noticed either.
- **About whole repositories, never one file at a time**: one run says what every
  file of the tree is, which is what lets the buffers of the same repository
  share an answer, and the panel color its entries from.
- **Nothing is redrawn when nothing changed**: the answer is compared with the one
  before, and a repository whose files stand where they were gets no redraw. A
  repository nobody looks at any more — its buffers killed, the panel rooted
  elsewhere — is no longer asked and its answer is dropped.

Typing in a buffer asks nothing: the state is what the file *on the disk* is to
the repository, which is what `git` reads and not the text the buffer holds. The
state of a file you are editing therefore follows your edits when you save them,
and not one keystroke before. A save which does not reach the disk — the buffer
frozen behind the banner of a file modified on disk, an answer of `n` to the
overwrite question — asks nothing either, and changes no state.

# The two entries of the configuration

`Options > Configuration` holds a theme named after the program, under a heading
of its own named `External tools` which says that the programs gathered there
are run by the editor beside itself. There:

| Entry | What it decides |
|---|---|
| `git program` | which program is asked. The default is `git`, the name on the paths of the session; a name which is no program on them takes the states away, both from the line and from the panel, and says so once in the minibuffer |
| Ask git what the files of a working tree are | whether the editor runs it at all. Off, the line says `(git)` and the panel wears the colors of the desktop — what they were before the states existed. The answer is read at every question, so it reaches the next poll with no restart, and the answers already on the screen go with it |

Both entries are read where they are used, on every question; nothing about this
needs a restart of the editor.

# What counts as a repository

The search starts at the directory the name of the file names and ascends to the
root of the file system. The first directory holding a `.git` entry is the root
of the repository, and that entry is asked only whether it is there: it is a
directory of a regular checkout and a *file* of a linked working tree or of a
submodule, and both make the directory above them a working tree.

| | |
|---|---|
| A file of a checkout, however deep under its root | `(git)`, plus its state |
| A file of a linked working tree or of a submodule, whose `.git` entry is a file naming the repository elsewhere | `(git)`, plus its state |
| The root itself, when the buffer visits a directory | `(git)`, since a directory of a repository is as much a working directory of it as one of its files is |
| A file which is not written yet, in a directory of a repository — the new name `Ctrl-X Ctrl-W` asks for | `(git)`, and `(git:untracked)` once the program answers, because the ascent asks the directories the name names, which are there, and not the file, which is not |
| `HEAD`, `config`, or any file under the `.git` entry | nothing: those are the bookkeeping of the program, not a working file of the repository whose entry they are |
| A file whose directories hold no `.git` entry as far as the root of the file system | nothing |
| A buffer visiting no file at all | nothing, since no path is named which a repository could hold a working file of |

The ascent is lexical: it asks the directories the name names and resolves no
symbolic link, which is what `git` does for the same question, so a repository
reached through a link is named where the link stands.

A file of no repository is named plainly, with nothing beside it — which is what
a reader has to be able to tell apart from the states above:

![The line of a buffer visiting `note.txt`, a file of no repository: the name of
the buffer, and nothing beside it](img/git-indicator-absent.png "The same editor visiting a file git knows nothing about: the line names the buffer and says nothing else")

A repository whose `.git` entry holds nothing the program will read — a half
finished `git init`, a directory made by a hand — is answered as nothing is known
about its files: the program says nothing about such a tree and says that it
said nothing, and the line stays at `(git)`. Drawing such a tree as a repository
whose files are all unchanged would be a claim about a program which refused to
answer.

# What it does not do

- It colors no word of the buffer and underlines nothing: the differences of a
  file with what the repository holds do not stand in the text. `M-x
  diff-buffer-with-file` and `M-x ediff-buffers` reach those differences as
  commands of the editor do, and a mark of every line of a file in the text
  itself is a decision of its own, of where to put the marks and what to do with
  a file the buffer holds unsaved.
- It names no branch and no repository path: the line of the buffer is one line
  of labels wide, and a name longer than the window is a name nobody reads.
- It runs no other program: no `git diff` to draw differences, no `git log`, no
  `git add` behind the hand. The one question of the one program is what the
  word and the colors are made of, and nothing else is asked of it.
