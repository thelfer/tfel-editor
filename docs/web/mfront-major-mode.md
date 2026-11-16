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

The page now lists the flows it holds, under a `Inelastic flows` label
which names that list, and the three buttons below the list work on it:

| Button | What it does |
|--------|--------------|
| `Add` | asks a new flow and adds it to the list |
| `Modify` | reopens the dialog with the selected flow already filled in |
| `Remove` | takes the selected flow out of the list |

An entry names the flow and the stress criterion it is drawn from —
`Plastic (criterion: Mises)` — and adds the plastic potential when the
flow is not associated: `Norton (criterion: Mises, plastic potential:
Hosford1972)`. Two flows can differ by their hardening rules only, so the
whole description — the flow, the criterion, the potential and each
hardening rule — stands in the tool tip of the entry.

`Modify` and `Remove` work on the selected flow and are disabled while the
list selects none, which is the state of an empty list and of a list whose
entry has been clicked away. A modification keeps the place and the
selection of the flow it rewrote; a removal selects the flow which takes its
place, and the dialog refusing a modification leaves the flow exactly as it
was. Double-clicking an entry is the same request as `Modify`: the dialog
reopens with the flow of the entry the pointer was on, whether or not that
entry was the selected one, and a double-click on the empty space below the
entries asks nothing. What the list holds is what the page writes: the
behaviour description it generates carries one `inelastic_flow:` entry per
flow of the list, in their order, so removing a flow here is the way to take
it out of a file which already holds one.

# Shortcuts

