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
- Checking the English prose of a buffer — the comments of a source file,
  a `LaTeX` document, a `Markdown` note — with the
  [`harper`](https://writewithharper.com) grammar checker, which is what
  the underlines of a wrong agreement or of a foreign spelling come from.
  See [the grammar checking page](grammar-checking.html) for details.
- Watching the file a buffer visits: another program writing over it is
  warned about in a banner above the buffer, which offers to reload the
  file, and a save never overwrites such a change without asking. See
  [the file modified on disk page](file-modified-on-disk.html) for
  details.
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
