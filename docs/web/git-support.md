When the file a buffer visits stands in the working tree of a
[`git`](https://git-scm.com) repository, the line under the buffer says so: a
short `(git)` stands right beside the name of the buffer, and nothing stands
there when the file belongs to no repository. There is nothing to turn on, no
entry in the configuration dialog and no program to point at — the answer comes
from the file system alone, so it costs nothing and is there from the first
moment the buffer is.

![The line of a buffer visiting `main.c` of a repository: the name of the
buffer, `(git)` beside it, the place of the cursor in the file, the major mode
and the time](img/git-indicator.png "A buffer of `src/main.c`, which stands
under `work/repo`, the directory whose `.git` entry makes it a repository: the
line names the buffer and says what that file is")

# What `(git)` says

One thing: the file the buffer visits is a working file of a repository, because
the editor found the `.git` entry of one above the directory the file stands in.

It says nothing else. A file nobody has added to the repository yet, a file
unchanged since the last commit, a file with edits nobody staged, and a file the
repository is told to ignore are named in the same way, because what makes them
all working files of the repository is that they stand in its working tree,
which is the one question the indicator asks. It names no branch and it says
nothing about the state of the file: the word is the same whether the file is
tracked, modified, staged or ignored, because none of that is what makes it a
working file of the repository.

That is the whole of the first step of the git support, and it is on purpose
that the answer asks no program: no `git` is started and no subprocess is
waited for, only a handful of questions put to the directories above the file.
A mode line which waited on a program would be a mode line that stops. What a
file *is* to the repository — modified, staged, which branch, and what its
differences with the committed text are — needs `git` itself, and is the next
step.

# Why beside the name, and not inside it

The name of a buffer is what identifies it: in its tab, in the list `Ctrl-X b`
walks, in the signal the editor emits when a buffer is renamed. Putting git in
that name would put it in all of those, and would say something about the file a
buffer happens to visit as if it were part of what the buffer is called — with
the odd consequence that the same file would give a buffer two names, one before
`git init` and one after. The indicator is therefore a label of its own, right
behind the label of the name it qualifies, so the pair reads as one item and the
name stays the name:

~~~~{.text}
main.c (git)          50%       (1,0)        C++        19:51
~~~~

It wears the font of the text edits, as the rest of the line does, and the font
dialog changes it for the indicator as it does for the name.

# What counts as a repository

The search starts at the directory the name of the file names and ascends to the
root of the file system. The first directory holding a `.git` entry is the root
of the repository, and that entry is asked only whether it is there: it is a
directory of a regular checkout and a *file* of a linked working tree or of a
submodule, and both make the directory above them a working tree.

| | |
|---|---|
| A file of a checkout, however deep under its root | `(git)` |
| A file of a linked working tree or of a submodule, whose `.git` entry is a file naming the repository elsewhere | `(git)` |
| The root itself, when the buffer visits a directory | `(git)`, since a directory of a repository is as much a working directory of it as one of its files is |
| A file which is not written yet, in a directory of a repository — the new name `Ctrl-X Ctrl-F` asks for | `(git)`, because the ascent asks the directories the name names, which are there, and not the file, which is not |
| `HEAD`, `config`, or any file under the `.git` entry | nothing: those are the bookkeeping of the program, not a working file of the repository whose entry they are |
| A file whose directories hold no `.git` entry as far as the root of the file system | nothing |
| A buffer visiting no file at all — the buffer of a new file, the output of a compilation, a session of an interpreter | nothing, since no path is named which a repository could hold a working file of |

The ascent is lexical: it asks the directories the name names and resolves no
symbolic link, which is what `git` does for the same question, so a repository
reached through a link is named where the link stands.

A file of no repository is named plainly, with nothing beside it — which is what
a reader has to be able to tell apart from the case above, so the two are shown
here in the same window, one buffer each:

![The line of a buffer visiting `note.txt`, a file of no repository: the name of
the buffer, and nothing beside
it](img/git-indicator-absent.png "The same editor visiting a file git knows
nothing about: the line names the buffer and says nothing else")

# When it is drawn

The question is about the name of the visited file and about nothing else, so it
is asked when that name is set and when the line of the buffer is drawn, and
never on a keystroke: typing in a buffer of a repository changes nothing about
the answer, and a line which asked again on every key would be a line which
asked for nothing.

So the indicator comes and goes when the buffer comes and goes from a
repository — writing a buffer under a new name (`Ctrl-X Ctrl-W`) is the moment
it happens — and it is drawn for a buffer when that buffer is created and again
whenever the editor re-checks the names it shows to tell its buffers apart.
Nothing watches the file system in the background: run `git init` in a directory
while a buffer of one of its files is open and the indicator appears the next
time the line of that buffer is drawn, which opening another file or writing
this one under another name is enough to bring about. Watching a directory for
an answer this cheap to take back would cost more than it buys.

# What it does not do

It is only a word in the line of the buffer:

- it colors no word of the buffer and underlines nothing — the differences of a
  file with what the repository holds are the next step, and they will stand in
  the text, not in the line;
- it names no branch and no repository path: the line of the buffer is one line
  of labels wide, and a name longer than the window is a name nobody reads;
- it is not a setting. There is no address for it in the configuration dialog,
  because there is no choice in it: a file of a repository names one, a file of
  none names nothing.
