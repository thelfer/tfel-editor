# Why another text editor ?

The primary idea behind this project is to have a GUI editor which can
leverage the full power of the tools that I have developped, for example
`MFront` (see http://tfel.sourceforge.net and
https://www.sciencedirect.com/science/article/pii/S0898122115003132),
`MTest`, `Licos`
(https://www.sciencedirect.com/science/article/pii/S0029549315003842)
and bring this power to standard engineers.

Those tools are mostly based in text files, so I decided to create a
text editor:

- which is inspired by `GNU Emacs` main concepts (such as buffers, major
  modes, etc..).
- that would make full use of the Qt library (hence the name,
  `qemacs=Qt+emacs`)

This text editor is meant to have the following advantages:

- Use `C++` as the main language.
- Portability (`Linux` is the primary target, but `Windows/FreeBSD/Mac Os`
  are also considered as first class targets).
- The ability to use
  Windows-type short-cuts for users not at ease with `GNU Emacs` ones and
  have all the power of the editor accessible from the GUI.
- The ability to integrate this editor into external software. More
  precisely, the projet is decomposed in a shared library caleed
  `TFELQEmacs` and an executable qemacs. The library provides the main
  widgets (for example the `QEmacsWidget`) that can be reused out of the
  box.
- Develop some new ideas.