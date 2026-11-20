A file whose name ends with `.ple` is read by the `Licos` mode, which is
dedicated to the input files of the [`Licos`](https://www.sciencedirect.com/science/article/pii/S0029549315003842)
program: a study of that language names, between quotes, the files it reads and
the files it writes, and the mode is built around those names.

The menu of the mode offers `Run Licos`, which is also `Alt-M`: the program is
started on the file the buffer visits and its output stands in a page of the
buffer, the same way as [the output of a compilation](compilation-output.html)
does. `Ctrl-C Ctrl-M` opens the wizard which writes the definition of a material
property into the study, `Ctrl-C Ctrl-E` inserts the block which starts the
definition of a block of data, and a submenu imports an `MFront` behaviour as a
material property of the study.

# Following a file name

A file name of a study is a **link**: it is drawn underlined, its tool tip says
*open in new buffer*, and one key or one click opens the file it names.

~~~~{.text}
Study 'example'
/-----------------------------------------------------------\
Type               'model-thermo.mfront'
Curve              'inputs/inpath.txt'
MaterialProperty   'table.dat'
Print              "report.ps"
\-----------------------------------------------------------/
EndOf
~~~~

What is a link is one answer, drawn and read with the same rule: a name between
two quotes — the single quotes the wizards of the mode write and the double
quotes a hand writes — whose extension is one the language knows, which are
`ple`, `txt`, `data`, `dat`, `py`, `mfront` and `ps`. A name may hold a `+`, a
`-`, a `_`, a `.`, accents, and the `/` of a sub-directory, so
`'inputs/inpath.txt'` and `'résultat.txt'` are links exactly as `'plain.txt'`
is. A name opened by one kind of quote and closed by the other is no name of
this language and is read as nothing.

`Enter` follows the link the cursor stands on — anywhere between the two quotes
of the name, not only on a particular character of it — and `Ctrl-M` does the
same. A **left click** on the name follows it too, which is what the tool tip of
the underline promises. The file opens in a buffer of its own, with the major
mode its own name chooses: the `MFront` mode for a `.mfront` source, the `C++`
mode for a `.py` script of a study, and nothing for a `.txt` file the language
has no opinion about. When the file the link names is already visited, its
buffer is brought to the screen rather than opened twice.

# Where the editor looks for a name

A study names its files by name, not by path, and the language lays the files of
a study in three sub-directories of the directory of the study: `inputs`,
`scripts` and `data`. So a name is asked of the directory of the file the buffer
visits and of those three sub-directories of it, and then of the same four
places of the directory the editor was started in — which is what lets a study
of a name typed at the prompt, or a file written elsewhere, be followed.

Then come the places the environment names:

| Variable | What it stands for |
| --- | --- |
| `LICOS_USER_FILES` | directories of the user's own files, separated by `:` — each of them is asked as the directory of a study is, that is itself and its `inputs`, `scripts` and `data` sub-directories |
| `LICOSHOME` | the installation of the program, whose own data files stand under `share/licos`, with the same three sub-directories |

An installation of the program holds the description files of the models and the
data files it ships with, so its tree is asked **last**: a name a study also
holds is answered by the study, which is what a person editing that study means,
and it is answered without asking anything of the environment — a machine where
`LICOSHOME` names nothing still opens the files of the study it is editing, and
says nothing about it.

A name which is nowhere is the one case the mode reports: the minibuffer reads
`file '<name>' not found`, and the buffer of the study stays on the screen.

# The three extensions which are not data files

A **`.ps` report** is looked for as the data files are and opens in a buffer of
the editor, like every other name the underline draws: a report of a study is a
file of the study, and reading the PostScript source of it is what the editor can
actually do with it.

A **`.mfront` source** is the material property the study builds on, and the
language keeps those sources in sub-directories of its own. The candidates are
the name as it stands beside the study, in the directory the editor was started
in and in the directories of `LICOS_USER_FILES`, and then the `mfront/properties`,
`mfront/behaviours` and `mfront/models` sub-directories of the directory of the
study. A source of none of those places is asked of `MFRONTMATERIALSHOME`, the
tree of the material properties the `MFront` installation ships with, last. A
source which *is* found opens, and nothing is reported about it: the search
stopped where the file was.

A **`.py` script** is looked for in the directories of `PYTHONPATH` — which is
where the Python scripts of a study of this language stand — and nothing happens
when `PYTHONPATH` names nothing.
