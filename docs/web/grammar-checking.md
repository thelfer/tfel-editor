This page describes what reads the words of a buffer of `tfel-editor` and
underlines what it does not like — the spelling of a word no dictionary
knows, and the way a sentence is written.

Four choices are offered, and one of them is in use at a time, for every
buffer of the editor:

| Choice | What reads the buffer | What is underlined |
|--------|-----------------------|--------------------|
| `No spell or grammar checking` | nothing | nothing |
| `Hunspell (spelling only)` | `hunspell` | the words no dictionary holds |
| `Hunspell and Harper (spelling and grammar)` | `hunspell`, and [`harper`](https://writewithharper.com) | the words, and the sentences |
| `LanguageTool (spelling and grammar)` | [LanguageTool](https://languagetool.org) | the words and the sentences, in any of the languages that program knows |

The first choice is what it says: no underline of any kind, which is a
thing one asks for while reading a file somebody else wrote. The second is
what the editor did before any grammar checker was added to it, and it is
the default, so a configuration file which says nothing is read that way.
The third adds a checker of *sentences* to the dictionary: an article which
does not fit the word it introduces, a verb which has to be the base form
after `to`, a pronoun which does not agree with its verb. The fourth hands
the text to a program which says both, and takes the dictionary off the
buffer while it does.

`harper` is a checker of English prose and nothing else: it knows what words
fit a sentence, not whether a name is in a dictionary, and it knows no word
of French. LanguageTool is the same kind of checker over many languages, and
it checks spelling too, which is why choosing it turns `hunspell` off rather
than adding a second set of underlines.

The editor embeds no checker of its own. `hunspell` is a library the editor
is linked against, `harper-ls` is a program it starts as a
[`Language Server Protocol`](https://microsoft.github.io/language-server-protocol/)
server over its standard input, and LanguageTool is a Java program it starts
as an HTTP server on the machine it runs on. Three ways of asking, one
surface: a plain red underline and a tooltip.

# Where the choice is made

| Where | Menu | Entries |
|-------|------|---------|
| The editor | `Options > Orthograph/Grammar` | the four choices |
| `C`/`C++` | `C/C++` | the four choices, beside the `clangd` entries |
| `LaTeX` | `LaTeX` | the four choices, below the `Interpreter` submenu of the engine which compiles the document |
| `Markdown` | `Markdown` | the four choices, which is the whole menu |

One choice for the editor, wherever it is made: the underlines of a text are
the surface where the person who wrote it looks at what they wrote, and two
buffers of one session disagreeing about what an error is makes the choice
unreadable. The buffers which are open when a choice is made follow it, and
so does every buffer opened afterwards.

The four entries are plain clickable rows, the one in use ticked: clicking a
row picks that choice and reports it in the minibuffer. The languages stand
in one `Language` submenu below the rows, which lists — as it is opened, and
so always in agreement with the rows above it — the languages of the choice
in use, and whose leaves pick the choice along with the language. The rows
carry no submenu of their own, because an entry which carries a submenu is
*opened* by a click and never triggered: a choice presented behind its own
languages can not be chosen at all, which is what picking `LanguageTool` used
to look like — a menu that did nothing.

An entry a machine can not run is not offered at all. `None` is always
there, `Hunspell` is there wherever the editor was built against `hunspell`,
and the two others appear when `harper-ls` is to be found in the directories
of the session, or a LanguageTool installation and a Java runtime are named.
An entry which could only report that a program is missing is no entry.

# The language of the choice

The three proofreaders spell a variety of English three different ways —
`en_GB` for a dictionary, `British` for `harper-ls`, `en-GB` for
LanguageTool — and one buffer is read by one of them. So the language is one
choice and not three: choosing `British English` in the submenu writes the
dictionary of `hunspell`, the dialect of `harper-ls` and the code of
LanguageTool, which is what keeps the submenu from ticking an English of its
own accord when the choice changes.

- `hunspell` offers the dictionaries which are installed, named as they name
  themselves (`en_US`, `fr_BE`);
- `harper-ls` offers `American`, `Canadian`, `Australian`, `British` and
  `Indian` English, which is the whole of what that program reads. A session
  which runs reads its buffers again under the new answer, so `colour` stops
  being a lint as soon as the buffer is read as British English;
- LanguageTool offers what it detects on its own (`auto`), the varieties of
  English, and the other languages it is installed with. Left alone, the
  editor asks about `en-US`.

# What a source file is read with

A source file is not a text. LanguageTool is handed a text and answers what
is wrong with it, and a `C++` file handed to it is a text full of names no
language has: every identifier, every `\command` of a LaTeX macro package,
every option name is a typo to it. What such a buffer is read with is
therefore the choice which names `hunspell`: the words its comments spell
are underlined, and the sentences are left to a checker which has no way of
telling them from the code.

That is why the entry of LanguageTool is offered as `LanguageTool (text
files)` in the menu of a mode whose text is the source of a program, and why
the minibuffer says `LanguageTool reads no code: this buffer is checked for
spelling only` when that entry is chosen there. In a `LaTeX` or a
`Markdown` buffer the whole text is prose and the program reads it.

`harper-ls` needs no such difference: it is told what a comment is by the
grammar of the language, and a `C++` buffer is sent to it whole.

# The lints

- **One face**: what a checker finds is underlined in the text, in a plain
  red underline. Every lint of `harper` comes with the same severity, a
  hint, and LanguageTool classifies its answers by the category of the rule
  and by no severity at all, so the whole answer is drawn with one face
  rather than sorted into colors by a scale the servers do not use. That red
  is not the red of an error of `clangd`, which is darker: the two underlines
  can be told apart where they sit beside each other.
- **The message of a lint**: resting the mouse on an underlined word shows
  what the checker said of it, with the name of the rule it applied —
  `Did you mean to spell 'colour' this way? [SpellCheck]`, for instance. The
  name of the rule — `AnA`, `InflectedVerbAfterTo`, `SpellCheck` for harper,
  `EN_A_VS_AN`, `MORFOLOGIK_RULE_EN_US` for LanguageTool — is what a person
  who thinks the rule wrong looks up in the documentation of the checker, so
  it goes with the sentence and is not left out. Several lints of one run are
  said together, one per line.
- **No entry in the context menu**: the checker says what is wrong with a
  word and leaves the pen to the person who wrote the sentence, so the
  message of a lint is read where it is written, as a tooltip.

The lints are drawn as a contribution of their own, keyed `grammar`, to the
extra selections of the text edit, the same mechanism the diagnostics, the
navigation hints and the colors of the identifiers of `clangd` use. A
session which is stopped, or a mode which is replaced, takes its own
underlines away and leaves the rest of the buffer untouched.

# Two checkers of a prose buffer

Under `Hunspell and Harper`, the two checkers answer two different questions
and the buffer shows both answers at once: `hunspell` underlines the words it
does not know, in the face the highlighter of the mode gives them, and the
lints of `harper-ls` stand under the runs of words the agreement does not
like. What the grammar checker says is never about the spelling of a word —
it is about the agreement of one word with another, the article before the
noun it introduces, the variety of English a spelling belongs to — so it
takes nothing off the dictionary of the buffer.

`LanguageTool` is the exception, and deliberately: it checks spelling
itself, in the language it was given, and a buffer it reads is handed an
empty dictionary. Two red underlines of two spellings of the same word would
say nothing a single one does not.

# LanguageTool, the program

The server of LanguageTool is a Java program, `languagetool-server.jar`,
which the editor starts itself:

    java -cp <installation>/languagetool-server.jar \
         org.languagetool.server.HTTPServer --port <port>

It is started without `--public`, which is what keeps it reachable from the
machine it runs on only, and it is asked on that port through the HTTP API,
`POST /v2/check` with the text of a buffer in the body of the request.

- **One server per editor window.** The client is a child of the editor
  widget it serves, and every buffer which asks for the checker is counted as
  a user of it. The reason is the size of what it starts: a virtual machine
  with the rules of every language the program knows loaded in it boots in
  about a second and costs a first request of two. The process is ended when
  the last buffer stops using it — a virtual machine which serves nobody is a
  hundred megabytes of nothing, and a person who turns the checker off in a
  menu means it.
- **A port of its own.** The setting `language-tool/port` names the port to
  start on; zero, the default, means the editor binds port 0, is given a free
  number by the kernel, and starts Java with that number — which is what
  lets two editors sit beside each other on one machine. The editor is the
  one starting the process, so a server somebody else started is a server it
  has no business talking to. A server which does not answer within a few
  seconds is ended and reported by the port it was given.
- **The text is sent whole, when the typing stops.** The API has no notion of
  a change: it is handed a text and answers what is wrong with it, so the
  change of a character is a whole new request. A document is therefore read
  500 ms after the last key, and a text longer than 40 000 characters is cut
  into pieces at empty lines, then at line ends, so that a buffer of any
  size is read and no answer is cut out of the middle of a sentence.
- **What the mode says is read.** The text a buffer hands over is the answer
  of `MajorMode::grammarCheckText`: the source as it stands for `Markdown`,
  the markup of a line blanked out for `LaTeX`. Blanked, not removed, because
  the answer of the server is a place in the text it was handed and a text
  shortened by what was taken out of it would put every answer behind the
  cut on a word the buffer does not hold there. What a run of hidden markup
  leaves is a run of spaces, which the checker reads as a repeated
  whitespace — so a report which names no word of the text is dropped, as it
  is about the text the editor wrote and not about the file.
- **A server which reads the wrong settings is restarted.** The program, the
  port and the Java options are read when the process starts; when the dialog
  says otherwise, the client ends the process it started and the next request
  starts the one the settings name. The language and the two lists of
  categories are read on every request, so the documents of the editor are
  simply handed over again.

The same care about a stale program is taken of `harper-ls`: a session is
compared with what `harper/executable` names now and is ended when the two
disagree.

# The settings

| Theme | Entry | Widget | Takes effect |
|-------|-------|--------|--------------|
| `Spelling and grammar` | `spelling/checker` | a choice | every buffer, at once |
| `Spelling and grammar` | `hunspell default dictionary` | a one line edit | every buffer, at once |
| `Harper` | `harper/executable` | a one line edit | the next session |
| `Harper` | `harper/dialect` | a choice | the buffers as they are read again |
| `LanguageTool` | `language-tool/installation` | a one line edit | the next request |
| `LanguageTool` | `language-tool/java-runtime` | a one line edit | the next server |
| `LanguageTool` | `language-tool/port` | a spin box | the next server |
| `LanguageTool` | `language-tool/java-options` | a list of texts | the next server |
| `LanguageTool` | `language-tool/language` | a one line edit | the next request |
| `LanguageTool` | `language-tool/enabled-categories` | a list of texts | the next request |
| `LanguageTool` | `language-tool/disabled-categories` | a list of texts | the next request |

The language of the dictionary is an entry of the theme of the choice, and
`Harper` and `LanguageTool` are submenus of it: the dialog shows one block
for what reads a buffer, since what one chooses and what the program chosen
is asked with are one decision. See [the configuration
page](configuration.html) for what the dialog makes of those themes. The
menus write four of them — `spelling/checker` and the three languages — and
the dialog writes the rest, which is what keeps a menu and a dialog from
disagreeing about one address.

LanguageTool is a download and not a package of most platforms, so the two
entries which say where it is and which `java` runs it have to be filled
before the choice is offered. Left empty, the installation is looked for in
`LANGUAGETOOL_HOME`, in `LANGUAGETOOLHOME` and in `/usr/share/languagetool`,
and the runtime in the directories of the session, then in `JAVA_HOME`. The
installation entry takes either the directory of the installation or the jar
itself, since that is what a person who knows where it is has under their
hands. A `language-tool/java-options` entry such as `-Xmx2g` is what the
rules of a dozen languages are worth.

Of the two lists of categories, `disabled-categories` is the filter: what is
named there is gone from the answer, whatever else the program makes of the
text. `enabled-categories` *adds* to what the program activates by itself.
No category is named by the editor itself: what the program which was
installed considers an error is the answer of that program.

# What the editor does not ask

There is no completion, no hover, no rename and no code action behind either
checker: the answers of those questions are either none of this feature's
business or a second round trip the editor would have to answer on the
server's behalf. Nor is a file watched from the outside: the checker sees
what the buffer holds as it changes, so saving is none of its business.

There is no command to launch or stop a session, either: the entry of the
menu is the door for a buffer and the setting is the door for the editor. The
[`clangd` integration](clangd-support.html) does have commands, because a
session which needs a compilation database to be chosen is a decision worth
making from the minibuffer; the proofreaders have no such choice to make.

# Building, and testing

Nothing special: both checkers are part of the core library, and the `LaTeX`
and `Markdown` modes use them as they were already built. The tests are
`test-grammar-check`, which drives a real `harper-ls` and a real LanguageTool
server when they are installed and skips the cases which ask them anything
when neither is; see [the unit tests page](unit-tests.html) for how to build
and run them. Where the server of LanguageTool is not part of the build, the
cases which ask it anything are skipped, and where no `hunspell` or no `en_US`
dictionary is installed, the one case which looks at what the two checkers do
to each other is skipped too.
