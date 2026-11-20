The `tfel-editor` text editor is meant to be a text editor merging the
power of [`Emacs`](https://www.gnu.org/software/emacs/) concepts with
the amazing abilities of the [`Qt` library](https://www.qt.io/).

# Main features

`tfel-editor` provides with advanced major modes for:

- Editing and compiling [`MFront`
  files](http://tfel.sourceforge.net). See [the description
  of `MFront` major mode](mfront-major-mode.html) for details.
- Editing and running [`MTest`
  files](http://tfel.sourceforge.net/mtest.html).See [the description
  of `MTest` major mode](mtest-major-mode.html) for details.
- Editing and running `Licos` input files (see
  <https://www.sciencedirect.com/science/article/pii/S0029549315003842>). See
  [the description of the `Licos` major mode](licos-mode.html) for what
  following a link to a file does.
- Editing and running `Cast3M` input files <http://www-cast3m.cea.fr>
- Editing `C` and `C++` files, with the diagnostics, the completion, the
  go to definition and the hover provided by a
  [`clangd`](https://clangd.llvm.org) language server. See [the
  `clangd` support page](clangd-support.html) for details.
- Editing and running `Python` files, with the diagnostics, the
  completion, the go to definition, the rename and the formatting
  provided by a [`pylsp`](https://python-lsp-server.readthedocs.io)
  language server, a highlighter which reads a docstring as prose, an
  indentation which follows the blocks of the file as `python.el` does, and
  an interpreter the line, the region or the whole buffer are sent to. See
  [the `Python` support page](python-support.html) for details.
- Checking what the words of a buffer say: the spelling of a word no
  dictionary knows, and the way a sentence is written — the comments of a
  source file, a `LaTeX` document, a `Markdown` note. The proofreader is a
  choice between [`hunspell`](https://hunspell.github.io), the
  [`harper`](https://writewithharper.com) language server and
  [LanguageTool](https://languagetool.org), and it is one choice for the
  editor, made under `Options > Orthograph/Grammar`. See
  [the grammar checking page](grammar-checking.html) for details.
- Watching the file a buffer visits: another program writing over it is
  warned about in a banner above the buffer, which offers to reload the
  file, and a save never overwrites such a change without asking. See
  [the file modified on disk page](file-modified-on-disk.html) for
  details.
- Editing by rectangles: inserting a string on every line of a block of
  columns, killing such a block and putting it back somewhere else. The lines
  and columns a rectangle covers and where the cursor stands afterwards follow
  `GNU Emacs`, command by command. See [the rectangle editing
  page](rectangle-editing.html) for details.
- Searching and replacing, from a dialog which stays on screen while the buffer
  is read — with the three options a reader wants, the letter case, the whole
  words and the regular expression — or from the minibuffer, one match at a
  time, as `GNU Emacs` does. The replacement of an expression may name the
  groups it matched. See [the searching page](searching.html) and [the find and
  replace page](find-replace.html) for details.
- Walking the file system beside the buffers: a panel on the left of them,
  shown by `Display > Hide/Show Explorer Panel`, rooted at the directory of the
  file on the screen, where a double click opens a file in a buffer, which ends
  above the mode line of the buffer and the line which reports what the editor
  did — both of which keep the whole width of the window — and whose open state
  and width come back to the window you open next. See [the
  file explorer page](file-explorer.html) for details.
- Browsing a directory in a buffer: naming one to `Ctrl-X Ctrl-F` shows a listing
  of its entries — name, owner, modification date — whose `Enter` key opens the
  file or enters the directory that the line it is pressed on names, and whose
  names wear what the repository they stand in says of them. See [the directory
  mode page](directory-mode.html) for details.
- Knowing that the file a buffer visits stands in the working tree of a
  [`git`](https://git-scm.com) repository and saying so in the line of that
  buffer, beside the name of the file: a short `(git)` for a working file of one,
  `(git:untracked)` for a file no commit holds and `(git:modified)` for a file the
  repository holds differently. The first of those is read off the directories
  above the file, so nothing is started and nothing is waited for; the two states
  come from asking `git` every five seconds about the repositories the window
  looks at, which the same answer uses to color the entries of the [file explorer
  panel](file-explorer.html). Which program to ask, and whether to ask it at all,
  is chosen in the configuration. See [the `git` support
  page](git-support.html) for details.
- etc.

The configuration of the editor, which is gathered by themes and can be
edited from inside the editor, is described on [the configuration
page](configuration.html).

See also the [Frequently Asked Questions](faq.html).

## Portability

`tfel-editor` is being developped and tested on:

- `Debian LiNuX` (gcc, clang)
- `Windows` (Visual Studio 2015 and 2017)

`tfel-editor` is expected to work on most platforms where
[`TFEL`](http://tfel.sourceforge.net/) and [`Qt`](https://www.qt.io/)
have been ported, which makes it quite universal.
