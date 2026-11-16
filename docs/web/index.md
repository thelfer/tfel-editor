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
  <https://www.sciencedirect.com/science/article/pii/S0029549315003842>).
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
