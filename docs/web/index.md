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
