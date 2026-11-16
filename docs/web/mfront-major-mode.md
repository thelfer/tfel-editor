This page describes the `MFront` major mode.

This mode is dedicated to:

- Editing `MFront` files.
- Analysing and compiling `MFront` files.

# Main features

![Editing an `MFront` file](img/mfront/screenshot1.png "Editing an
`MFront` file")

- Completion of `MFront` keywords. Keywords depends on the currently
  used `DSL`.
- Ease implementation of `MFront` files for providing wizards.
- Easy access to the command-line documentation of each keyword.
- Easy access to the `MFront` documentation.
- Easy access of implementations provided by the `MFrontGallery` project (if
  installed). This requires the `MFRONTGALLERYHOME` environment variable
  to be defined.
- Easy access of implementations provided by a user defined material
  management project (build on the basis of the `MFrontGallery`
  project). This requires the `MFMHOME` environment variable to be
  defined.

## Access to implementations provided by the `MFrontGallery` project

![Accessing an implementation provided by the `MFrontGallery`
project](img/mfront/screenshot-import-mfm.png
"Accessing an implementation provided by the `MFrontGallery` project (Debian Jessie)")

## The inelastic flows of the `StandardElastoViscoPlasticity` brick

The page of the behaviour wizard which configures the
`StandardElastoViscoPlasticity` brick asks for the inelastic flows one
after the other, and a flow which has been given was kept out of sight:
the page held them but showed nothing, so a flow with a wrong parameter
could only be reached by filling the whole page again.

The page now lists the flows it holds in a combo box below the `Add an
inelastic flow` button, and two buttons below the list act on the flow it
selects:

| Button | What it does |
|--------|--------------|
| `Add an inelastic flow` | asks a new flow and adds it to the list |
| `Modify the selected flow` | reopens the dialog with the selected flow already filled in |
| `Remove the selected flow` | takes the selected flow out of the list |

An entry names the flow and the stress criterion it is drawn from —
`Plastic (criterion: Mises)` — and adds the plastic potential when the
flow is not associated: `Norton (criterion: Mises, plastic potential:
Hosford1972)`. Two flows can differ by their hardening rules only, so the
whole description — the flow, the criterion, the potential and each
hardening rule — stands in the tool tip of the entry.

The two buttons which act on the selection are disabled while the list is
empty. A modification keeps the place and the selection of the flow it
rewrote; a removal selects the flow which takes its place, and the dialog
refusing a modification leaves the flow exactly as it was. What the list
holds is what the page writes: the behaviour description it generates
carries one `inelastic_flow:` entry per flow of the list, in their order,
so removing a flow here is the way to take it out of a file which already
holds one.

# Shortcuts

