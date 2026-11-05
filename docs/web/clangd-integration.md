This page is the state and the roadmap of the `clangd` integration of
`tfel-editor`: what the editor asks of a language server, what `clangd`
offers and the editor ignores, and in which order the gaps are worth
closing. How to use what exists is on the
[`clangd` support page](clangd-support.html); this page is about what
is *not* there.

The dividing line is the conversation: `LspClient` speaks
`initialize`/`shutdown`/`exit`, `textDocument/didOpen`,
`didChange` and `didClose`, `completion`, `definition`, `declaration`,
`references`, `rename`, `signatureHelp`, `codeAction`, `documentSymbol`,
`hover` and `semanticTokens/full`, and listens to `publishDiagnostics`
and `window/showMessage`. Everything `clangd` can say beyond that never
reaches the screen.

# Supported today

- **Diagnostics**: positions underlined by severity, and the message
  text echoed in the minibuffer when the cursor stands on an underlined
  range or the mouse rests on one.
- **Completion**: `Alt-Tab`, with the details of the candidates.
- **Go to definition**: `Alt-.`, the context menu, and the underline of
  the cursor word with the jump icon under the mouse.
- **Declaration**: asked alongside definition to decide the underline
  and the icon of a word which has no body.
- **Find references**: `Alt-,` lists every use of the symbol under the
  cursor and jumps to the one chosen.
- **Rename**: `Alt-R` asks a new name in the minibuffer and applies the
  workspace edit `clangd` answers to every buffer visiting the files it
  names, opening the files which are not visited yet.
- **Quick fixes**: the context menu offers what the server can repair on
  the line it was opened on — insert a missing `;`, add a dereference,
  spell a name right — and triggering an entry writes exactly the text
  the server sent, through the same workspace-edit path a rename goes
  by. The status tip of an entry is the diagnostic it answers to.
- **Signature help**: `Alt-S`, and the characters `clangd` itself
  lists as places a call is typed around, echo the parameter list in
  the minibuffer with the argument being typed picked out of it.
- **Document symbols**: `Alt-O` (and `Go to symbol...` in the menu) asks
  `documentSymbol` and shows the answer as the overview of the file: the
  symbols in the order the file spells them, each with its qualified name
  (`Widget::paint`) and the signature or kind the server says beside it,
  narrowed as a name is typed and jumped to when answered.
- **Hover**: `Alt-?` shows the declaration and the documentation.
- **Semantic tokens**: `semanticTokens/full` for the whole file, asked
  when a session starts, when the file has been parsed and when the text
  settles; the answer colours the identifiers it resolved — types,
  functions and methods, macros, and a strike-out over the names it says
  are deprecated — as an extra selection, over the faces the hand-written
  highlighter already gave the text. The rest of what the server names,
  a local against a member, is deliberately left uncoloured, and `auto`
  is skipped, because it is answered as the type it is deduced to and the
  keyword is the highlighter's. `Color identifiers` in the menu and the
  setting `clangd/semantic-tokens` are the two doors to that choice.
- **Server messages**: `window/showMessage` is displayed in the
  minibuffer.

# Not supported

## `clangd` offers it, the editor never asks

- **Code actions the editor cannot apply**: the AST tweaks of
  `clangd` — extract a variable, add a missing `override`, generate a
  definition — arrive as a `clangd.applyTweak` command with no text,
  because the change is computed by a second round trip; the editor
  offers the actions which carry their edit and leaves those alone.
- **Workspace symbols** (`workspace/symbol`): no jump to a symbol by name
  across the project; `documentSymbol` covers the file which is on screen.
- **Implementation, type definition, call hierarchy, type hierarchy**
  (`implementation`, `typeDefinition`, `callHierarchy/*`,
  `typeHierarchy/*`): beyond plain go-to-definition.
- **Semantic tokens of a range, and their deltas**
  (`semanticTokens/range`, `semanticTokens/full/delta`): the first is
  refused by `clangd` itself — its provider says `range: false` — and the
  second would let an answer name only what changed since the last one;
  the editor asks for the whole file every time and reads the answer whole.
- **Inlay hints** (`textDocument/inlayHint`): no deduced `auto` types or
  parameter names inlined in the text.
- **Smart select** (`textDocument/selectionRange`) and **code folding**
  (`textDocument/foldingRange`).
- **Go to declaration as an action**: `declaration` is asked only to
  decide the underline and the hover icon; no key or menu entry jumps
  to a declaration specifically. The context-menu jump uses definition,
  which covers most cases since `clangd` falls back to the declaration
  of a body-less symbol.
- **Jump back** after a jump: `Ctrl-X B` (change buffer) is the
  workaround; there is no mark ring for jumps.
- **Auto-hover**: the tooltip is `Alt-?`-only; the mouse resting on a
  word shows the jump icon, not the documentation.

## Protocol features deliberately declined

Refused in the `initialize` handshake, so the server will not use them:

- **Progress reporting** (`window/workDoneProgress: false`): indexing
  and preamble builds stay silent; the minibuffer reports only the
  handshake.
- **Dynamic registration** (`dynamicRegistration: false`).
- **Completion item resolution** (`completionItem/resolve`): what the
  server sends initially is what the popup shows.

## Practical limits

- Buffers **without a file on disk** (`*scratch*`, unsaved files) are
  never analysed: `clangd` identifies documents by file name.
- Diagnostics are underlined with **single underlines, not wavy
  lines**: `Qt 6.2` does not ink wave styles in its text widgets. The
  faces of the semantic tokens are a foreground color and a strike-out,
  which that Qt does ink.
- One `clangd` per editor **window**, launched by the user; the
  compilation database is read from the directory fixed at launch.
  Regenerating that file in place changes the options the next parse of
  a file uses — `clangd` re-reads it by itself — but not what the server
  knows about the files it has already indexed: see
  [When the database changes](clangd-support.html#when-the-database-changes).

## Why the database is not watched

A `QFileSystemWatcher` on `compile_commands.json`, restarting the session
when the file is rewritten, was the first idea: the editor already watches
directories for `DirectoryMajorMode`, and a regenerated database is the
common case. It is left alone because what it would buy is small and what
it would cost is a working session:

- the half a user feels most, the options a file is compiled with, needs
  no help: `clangd` re-reads the database itself and the next edit of the
  buffer uses the new command line;
- the half a restart would reach, the index of the rest of the project, is
  not fixed by the restart either: what `clangd` persists of a file is
  keyed on its contents, so a session launched on the new database keeps
  trusting the shard it wrote for contents it has already seen, and an
  unchanged file stays as unknown as it was until `.cache/clangd` goes;
- a restart ends the diagnostics, the completion state and the parsed
  preamble of every buffer of the window, on a file write which is often
  nothing of the sort: a build touching a build directory the session was
  pointed at is enough to fire it.

Watching the file to *say* something in the minibuffer — the database
changed, the index of the project is stale — is the part of the idea that
would still pay for itself, and is unbuilt.

# Roadmap

In the order the gaps are worth closing, each with the shape it would
take in this editor:

1. [x] **Diagnostic messages** — the positions are shown, the texts
   were not; the first thing a user of `flymake-syntax-check` misses.
   Shape: the message echoed in the minibuffer when the cursor points
   at an underline, and as a tooltip when the mouse rests on one.
2. [x] **Find references** — the other half of navigation. Shape:
   `Alt-,` asks the server and pops a menu of `file:line` entries
   which jump, the same ritual as go-to-definition.
3. [x] **Rename** — `query-replace` is no symbol-aware. Shape: a
   command asking the new name in the minibuffer, a `rename` request,
   and the returned workspace edit applied buffer by buffer.
4. [x] **Signature help** — while typing a call, the parameter list in
   the minibuffer. Shape: `Alt-S` asks it, and the trigger characters the
   server declares in its handshake ask it as the call is typed; the
   answer arrives asynchronously, so typing is never waited on, and a
   call which is not one is passed over in silence.
5. [x] **Code actions** — quick fixes as entries of the context menu
   next to `Go to definition`, applied through the same workspace-edit
   path as rename. Shape: the menu asks definition and actions together
   and waits for both a moment only; an action is offered when it carries
   its edit, so the AST tweaks of `clangd`, which come as a command and
   no text at all, are left to a server that could apply them itself.
6. [x] **Document symbols** — an overview of a file, which the tab bar
   or a completion popup could show. Shape: `Alt-O` asks `documentSymbol`
   and hands the answer to the minibuffer question which names the symbol
   to go to; the overview is displayed as the list of completions, the
   names qualified and narrowed as one is typed, and the answer goes to
   the place the server gave for that name. A tree is what the handshake
   asks for, since the flat answer loses the signature each entry of a
   tree carries beside its name.
7. [x] **Semantic tokens** — would replace the hand-written `C`/`C++`
   highlighter's guess with the server's knowledge; the largest visual
   change of the list, and the one to weigh last. Shape: it replaced
   nothing, it overlaid. The question is asked of the whole file, since
   `clangd` answers no range of it, and the answer paints four faces as
   an extra selection of the text edit — types in a dark blue, functions
   and methods in a dark cyan, macros in a dark magenta, deprecated names
   struck through — while the highlighter keeps the keywords, strings,
   numbers and comments, which are kinds no server names. Locals,
   parameters and members stay as they are, and the words deduced to be a
   type are skipped so that `auto` stays a keyword. On by default, with
   `clangd/semantic-tokens` and a checkable menu entry to turn it off.

The checked items are what the editor learned most recently; the rest is
proposed, not scheduled.
