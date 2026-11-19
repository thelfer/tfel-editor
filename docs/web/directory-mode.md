When a buffer is given the name of a directory — `Ctrl-X Ctrl-F`, or any other
question the editor asks for a file — the editor does not visit it as a file: it
shows a **listing of its entries**, one line per entry, in a buffer whose major
mode is `directory-mode`:

~~~~{.text}
..                    th202608 Thu Nov 19 11:57:12 2026
brand_new.txt         th202608 Thu Nov 19 11:57:12 2026
committed_runnable.sh th202608 Thu Nov 19 11:57:12 2026
ignored.o             th202608 Thu Nov 19 11:57:12 2026
inner_changed.txt     th202608 Thu Nov 19 11:57:12 2026
inner_clean.txt       th202608 Thu Nov 19 11:57:12 2026
newdir                th202608 Thu Nov 19 11:57:12 2026
notes                 th202608 Thu Nov 19 11:57:12 2026
runnable.sh           th202608 Thu Nov 19 11:57:12 2026
src  (git)   10%   (1,0)   directory-mode   11:57
~~~~

The columns name the entry, its owner and the date it was last modified, spaced
apart; the first line is always the parent directory `..`, which is how a listing
is left with the same key that enters it. The line under the buffer names the
directory, not a file — and says `(git)` of it, since a directory of a repository is
as much a working directory of it as one of its files is. It says `(git)` and not
`(git:modified)`: what the program is asked about is that directory, and a directory
is named by `git status` only when it holds nothing but new files, which is the
sentence the [`git` support page](git-support.html) has for the states of a
directory.

The text of the listing is not editable: it is what the file system holds, and it
is rewritten when the directory changes — a file created beside the buffer
refreshes the listing, and the line the cursor stands on is kept through that
rewrite.

`Enter` is the key of the listing: it **opens the entry the line it is pressed on
names** — a file in a new buffer, a directory in place of the one on the screen,
`..` in the directory above. The line decides, not the word under the cursor, so
the key answers the same wherever on the line the cursor stands; an empty line
names nothing.

# The colors of a listing

The name of an entry is drawn with what the entry *is*, and with what the
repository it stands in says of it:

| The name is drawn | Because |
|---|---|
| blue | no commit and no index holds the entry — a new file, a new directory, anything under a new directory, an executable nobody has added |
| dark red | the repository holds the entry differently from what it is: edited text, staged addition or deletion, rename, conflict |
| dark blue | a directory — either one the repository holds as it is, or one which stands in no repository at all |
| dark green | an executable file in either of those two states |
| the color of the desktop | a plain file, in either of those two states too; and an ignored file, which is the one state `git status` names nothing about |

The two inks of the first two rows are the inks of the [file explorer
panel](file-explorer.html), and they come from the same answer: both surfaces ask
the same client, which asks `git` every five seconds about the repositories the
window looks at — a buffer of a directory is one of them, because a directory of a
repository is as much a working directory of it as one of its files is. The
[git support page](git-support.html) says what that answer is made of, when it is
asked, and how the two entries of the configuration turn it off.

![One working tree, seen from the two surfaces which ask about it. The panel on the
left is rooted at the root of the tree: `modified.txt` dark red, `loose_new.txt`
blue, `tracked.txt` and the directory `src` left the ink of the desktop. The
listing on the right is a buffer of the subdirectory `src`: `brand_new.txt`,
`runnable.sh` and the new directory `newdir` blue, `inner_changed.txt` dark red,
`committed_runnable.sh` dark green, `notes` — a directory the commit holds — and
the parent entry dark blue, `ignored.o` and `inner_clean.txt` the ink of a plain
file. The line of the buffer names the mode and says `(git)`](
img/git-directory-colors.png "One
answer, drawn in two places. In the listing, the blue of `runnable.sh` stands over
the dark green of an executable and that of `newdir` over the dark blue of a
directory: what the repository says of a name is drawn over what the name *is*,
which is what the panel does as well")

So a directory is dark blue and a new directory is blue, and the two faces are one
hue apart: what the state of an entry is has the priority, since the state is the
thing the color was added to say. An executable in a new directory is blue for the
same reason. A directory is never drawn dark red, because `git status` names the
file which differs and not the directory it stands in, and a red invented for the
directory would say the directory itself differs, which is a thing the program
never says.

The parent entry `..` is never drawn with a state: it names a directory the listing
does not show, and often enough one out of the repository of the directory it
shows. It stands dark blue, like every directory nothing is known to say about.

Entering a directory asks its repository at once, rather than at the next question
of the timer, because a listing which just appeared on the screen is a moment a
person is looking at. Typing asks nothing — and there is nothing to type in a
listing: the state is what the file *on the disk* is to the repository.

# What it does not do

- It shows no hidden entry: a name which begins with a dot is none of the
  listing's business — the parent entry `..` excepted, which is not a name but the
  directory above — which is why the listing of a repository does not name its
  `.git` entry either.
- It shows no sizes and no permissions, and no `total` line.
- It offers no commands of the file system: no copy, no delete, no rename. Those
  are the file manager's office, or `dired`'s, which this listing is not.
- It keeps no tree open: entering a directory replaces the listing, and `..` is the
  way back, which is what keeps the buffer a listing of one directory.
