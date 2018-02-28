This page describes the `MTest` and `PipeTest` major modes.

Those modes are dedicated to:

- Editing `MTest` files when describing a single material point (default
  file extension `.mtest`). This is the `MTest` major mode.
- Editing `MTest` files when describing pipes (default file extension
  `.ptest`). This is the `PipeTest` major mode.
- Running `MTest`.

# Main features

![Editing an `MTest` file](img/MTest/screenshot1.png "Editing an
`MTest` file")

- Completion of `MTest` keywords. Keywords depends on the currently
  used `DSL`.
- Ease implementation of `MTest` files for providing wizards.
- Easy access to the command-line documentation of each keyword.
- Easy access to the `MTest` documentation.
- Easy access of implementations provided by the `MFrontGallery` project
  (if installed). This requires the `MFRONTGALLERYHOME` environment
  variable to be defined.
- Easy access of implementations provided by a user defined material
  management project (build on the basis of the `MFrontGallery`
  project). This requires the `MFMHOME` environment variable to be
  defined.

## Access to implementations provided by the `MFrontGallery` project

![Accessing an implementation provided by the `MFrontGallery`
project](img/mtest/screenshot-import-mfm.png
"Accessing an implementation provided by the `MFrontGallery` project (Windows 10, Visual Studio 2017)")



