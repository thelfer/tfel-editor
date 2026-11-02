/*!
 * \file  LicosMajorMode.hxx
 * \brief LicosMajorMode: major mode for licos input files, runs studies.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_LICOSMAJORMODE_HXX
#define LIB_TFEL_GUI_LICOSMAJORMODE_HXX

#include <QtCore/QVector>
#include <QtCore/QRegularExpression>
#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct LicosTokenizer;
  // forward declaration
  struct LicosBlock;

  //! major mode for Licos input files
  struct LicosMajorMode : public MajorModeBase {
    /*!
     * \return the extensions of the files a licos input file may refer to, that
     * is `ple`, `txt`, `data`, `dat`, `py`, `mfront` and `ps`
     *
     * \note the list is built once, on the first call; the syntax highlighter
     * uses it to underline the file names which may be opened
     */
    static const QStringList &getLicosExtensionsSuffix();

    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor widget the mode acts on
     *
     * \note the completer is filled with the keywords of the language and with
     * the names of the blocks and their `EndOf` counterpart; it is a child of
     * \c t, which owns it
     *
     * \note mouse tracking is enabled on \c t; the hover feedback which needed
     * it is now commented out
     */
    LicosMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "Licos"
    QString getName() const override;

    //! \return "major mode dedicated to the licos input file"
    QString getDescription() const override;

    /*!
     * \brief install a \c LicosSyntaxHighlighter on the given document
     * \param[in] d : document to be highlighted
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     *
     * \note this mode adds no action of its own and only calls the
     * implementation of \c MajorModeBase, which adds none either: the actions
     * dedicated to the files, libraries and functions under the cursor are
     * commented out
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \return the completer built by the constructor
     *
     * \note the completer is a child of the text edit, not of the mode
     */
    QCompleter *getCompleter() override;

    /*!
     * \brief insert the completion the user has selected
     * \param[in] t : text edit to complete the word in
     * \param[in] w : completion chosen by the user
     *
     * \note when \c w names a block, the matching `EndOf` line is inserted
     * after the current one and the cursor is left between the two lines
     */
    void completeCurrentWord(TextEditBase &, const QString &) override;

    /*!
     * \return the "Licos" menu, which runs the study, adds material properties
     * and opens the wizards
     *
     * \note the menu is a child of the text edit and its actions are created
     * here and connected to the slots of this mode
     *
     * \note a null pointer is returned when the parent of the mode is not a
     * widget
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief treat the keys the mode intercepts before \c TextEditBase does
     * \param[in] e : key event
     * \return true if the mode handled the key, in which case \c TextEditBase
     * applies no shortcut of its own
     *
     * \note `Ctrl-m` and `Return` open the file whose name the cursor is on;
     * they are not handled when the cursor is on no such name
     *
     * \note `Alt-m` runs the study and is always handled
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief open, on a left button click, the file whose name is under the
     * mouse position
     * \param[in] e : mouse event
     * \return true if a file name of a known extension was found, in which
     * case \c TextEditBase ignores the event
     *
     * \note other buttons and clicks which are not on a file name are not
     * handled
     */
    bool mousePressEvent(QMouseEvent *const) override;

    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
     * TextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     *
     * \note `Ctrl-C Ctrl-E` opens the input line which inserts a block and is
     * the only sequence reported as handled
     *
     * \note `Ctrl-C Ctrl-M` shows the material wizard but is reported as not
     * handled, so \c TextEditBase still reports the unknown shortcut
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;

    /*!
     * indent the current line
     *
     * \param[in] tc : text cursor of the line to indent
     *
     * \note the indentation is that the licos blocks enclosing the line require
     *
     * \note all the lines which precede it are replayed to know which blocks
     * are open, so the result does not depend on their own indentation
     *
     * \note the cursor is left next to the character it was next to
     */
    void indentLine(const QTextCursor &) override;

    /*!
     * indent selected region
     *
     * \param[in] tc : text cursor defining the region to indent
     *
     * \note the region is left untouched when the cursor holds no selection and
     * a selection which stays on a single line is handed over to \c indentLine
     *
     * \note the position of the cursor is saved by
     * \c MajorModeBase::getCursorMark before the region is indented and
     * restored by \c MajorModeBase::restoreCursorMark once it has been, so that
     * the cursor remains next to the character it was next to
     */
    void indentRegion(const QTextCursor &) override;

    //! \return the icon of the mode, read from the resources of the package
    QIcon getIcon() const override;

    /*!
     * \brief comment the lines of the selection with `//` or uncomment them
     * when they are all commented; without a selection, insert `// ` at the
     * cursor
     *
     * \note the file is parsed from its first line to know whether a C style
     * comment is opened; when that parsing fails, the reason is displayed in
     * the minibuffer and nothing is changed
     *
     * \note without a selection, nothing is inserted when a C style comment is
     * already opened at the cursor
     *
     * \note the editor's cursor is left at the end of the region which has been
     * commented or uncommented
     */
    void comment() override;

    /*!
     * \brief format the text the cursor is in
     *
     * \note this implementation does nothing: licos input files are only
     * indented, never reformatted
     */
    void format() override;

    /*!
     * destructor
     *
     * \note the completer is not released: it is a child of the text edit,
     * which owns it
     */
    ~LicosMajorMode() override;

   protected slots:

    /*!
     * \brief run the study in dry-run mode; the "Dry-run Licos" action of the
     * specific menu is connected to this slot
     *
     * \note this implementation does nothing
     */
    virtual void dryrunLicos();

    /*!
     * \brief show the modal material wizard and, when the user accepts it,
     * insert the material it describes at the cursor of the text edit
     *
     * \note nothing is inserted when the wizard is rejected
     */
    virtual void showMaterialWizard();

    /*!
     * \brief show a modal wizard titled "Thermal behaviour wizard"; the
     * "ThermalBehaviour Wizard" action of the specific menu is connected to
     * this slot
     *
     * \note no page is added to that wizard: the slot only displays an empty
     * dialog
     */
    virtual void showThermalBehaviourWizard();

    /*!
     * \brief show a modal wizard titled "Mechanical behaviour wizard"; the
     * "MechanicalBehaviour Wizard" action of the specific menu is connected to
     * this slot
     *
     * \note no page is added to that wizard: the slot only displays an empty
     * dialog
     */
    virtual void showMechanicalBehaviourWizard();

    /*!
     * \brief run the study of the current buffer; the "Run Licos" action of the
     * specific menu and the `Alt-m` shortcut are connected to this slot
     *
     * \note when the buffer is modified, the user is asked to save it first and
     * \c startLicos is called once that question is answered, whatever the
     * answer is
     */
    virtual void runLicos();

    /*!
     * \brief ask the study options in a dialog, then run the study of the file
     * the buffer is saved in and show its output in a secondary task named
     * "Licos Output"
     *
     * \note nothing is run when the buffer has no file name or when the options
     * dialog is rejected; the first case is reported in the minibuffer
     *
     * \note \c studyFinished is called when the study is over
     */
    virtual void startLicos();

    /*!
     * \brief report the end of a study in a message box
     * \param[in] s : true if the study succeeded
     * \param[in] e : error message of the study, empty on success
     *
     * \note on failure, the lines of \c e are walked from the end to the
     * beginning and the earliest one of the form "Parsing error at line N in
     * file 'F'." wins: when such a line is found, \c F is opened and the cursor
     * is moved to the line \c N
     */
    virtual void studyFinished(bool, QString);

    /*!
     * \brief treat an action the user has triggered from a menu
     * \param[in] a : triggered action
     *
     * \note this implementation does nothing: the actions this slot compared
     * \c a to and the code which connected the menus to it have been removed
     */
    virtual void actionTriggered(QAction *);

    /*!
     * \brief open the file stored in the data of the given action with the
     * handler the desktop uses for such files
     * \param[in] a : triggered action; nothing is done when it is null
     *
     * \note no menu is connected to this slot anymore: the "Documentations"
     * sub-menu which used to do so is not created
     */
    virtual void openDocumentation(QAction *);

    /*!
     * \brief show the modal dialog which lets the user choose material
     * properties and insert them at the cursor of the text edit; the "Add
     * material properties" action of the specific menu is connected to this
     * slot
     */
    virtual void addMaterialProperties();

   protected:
    /*!
     * \brief the input line which asks the name of a block and inserts a
     * matching pair of lines, `Block` and `EndOfBlock`, in the text edit
     *
     * \note it is created by \c handleShortCut for the `Ctrl-C Ctrl-E`
     * shortcut
     */
    struct LicosInsertBlock;

    /*!
     * \return the extensions \c getLicosExtensionsSuffix returns
     */
    static QStringList buildLicosExtensionsSuffix();

    /*!
     * check if the given file exists and is readable
     * \param[out] r : absolute path of the file when the check succeeds
     * \param[in] f : the file to be checked
     * \return true if the file exists and is readable
     *
     * \note \c r is cleared first: it is set to the absolute path of the file
     * when the check succeeds and left empty otherwise
     *
     * \note a file which exists but cannot be read is reported in the
     * minibuffer and makes the check fail, as a name which is not a file does
     */
    virtual bool checkFile(QString &, const QString &);

    /*!
     * search a file using the LICOS_USER_FILES environment variable
     * \param[in] f : name of the file to look for
     * \return an empty string if nothing was found
     *
     * \note the candidates are built from the directory of the file of the
     * buffer and from the current directory, both of them being also looked at
     * through their `inputs`, `scripts` and `data` sub-directories, then from
     * the directories the variable lists, which are separated by colons
     *
     * \note the installation tree of licos is the last candidate; it is only
     * reached when \c getLicosPath returns an empty string, and that call
     * reports the missing `LICOSHOME` variable in the minibuffer
     *
     * \note a file which is not found is reported in the minibuffer
     */
    virtual QString getLicosFile(const QString &);

    /*!
     * search a file using the given path
     * \param[in] f : name of the file to be found
     * \param[in] path : directories to look in, separated by colons
     * \return an empty string if nothing was found
     *
     * \note the directory of the file of the buffer and the current directory
     * are looked at before the given ones; the `inputs`, `scripts` and `data`
     * sub-directories \c getLicosFile tries are not candidates here
     *
     * \note nothing is reported in the minibuffer when nothing is found
     */
    QString getFileInPath(const QString &, const QString &);

    /*!
     * \return the indentation, in blanks, the line the given cursor is on shall
     * have, that is the one the licos blocks which enclose it require
     * \param[inout] blocks : stack of the blocks which are open, the innermost
     * one last; it is completed by the lines which declare a sub-block
     * \param[inout] tokenizer : tokenizer which holds the state of the
     * beginning of the line: an opened C style comment or an opened array
     * \param[in] tc : text cursor of the line; only the block it belongs to is
     * used
     *
     * \note all the lines which precede the given one are replayed, from the
     * first line of the document; \c blocks and \c tokenizer shall describe the
     * state of the document before this replay
     *
     * \note the caller seeds \c blocks with the main block of the file
     */
    int getLineIndentation(QVector<QSharedPointer<LicosBlock>> &,
                           LicosTokenizer &,
                           const QTextCursor &) const;

    /*!
     * \return the indentation, in blanks, the given line shall have; it is zero
     * when the parsing of the file has failed
     * \param[inout] blocks : stack of the open blocks, the innermost one last;
     * it is updated by the given line, which may open or close a block
     * \param[inout] tokenizer : tokenizer holding the state of the end of the
     * previous line; it is left in the state of the end of the given line
     * \param[inout] i : indentation of the given line; it is updated to the
     * indentation the lines which follow shall have
     * \param[in] line : contents of the line
     * \param[in] lineNumber : number of the line, which only appears in the
     * message raised by a line whose licos syntax is invalid
     *
     * \note a line which closes a block is indented as its opening line, a line
     * which opens one indents the next lines by two blanks, and a line which
     * continues an opened array is indented by one blank more than the brace
     * which opened it
     *
     * \note an invalid line does not throw: the tokenizer is set to its failed
     * state and the indentation of everything which follows is zero
     */
    int getIndentationIncrement(QVector<QSharedPointer<LicosBlock>> &,
                                LicosTokenizer &,
                                int &,
                                const QString &,
                                const unsigned short) const;

    /*!
     * \return the file name under the given cursor. An empty string
     * if no suitable file name is found.
     * \param[in] tc : text cursor
     * \param[in] e  : extension tested
     *
     * \note a file name is a name of a licos file followed by the extension
     * \c e, between simple quotes; the returned name does not include those
     * quotes
     *
     * \note the whole line is scanned: the cursor must stand inside the quoted
     * name for it to be returned
     */
    QString getFileNameUnderCursor(const QTextCursor &, const QString &);

    /*!
     * \return the file name under the given cursor. An empty string
     * if no suitable file name is found.
     * \param[in] tc : text cursor
     * \param[in] e  : valid extensions
     *
     * \note the extensions are tested in the given order and the first name
     * found is returned
     */
    QString getFileNameUnderCursor(const QTextCursor &, const QStringList &);

    /*!
     * look if the QTextCursor points to a licos extension file.
     * If so, import the file in a new buffer
     *
     * \note a `py` file is looked for in the directories of `PYTHONPATH`, an
     * `mfront` file is handed over to \c openMFrontSource and the other
     * extensions are looked for by \c getLicosFile
     *
     * \note a `ps` file is recognised but nothing is done with it: the code
     * which opened it in the desktop viewer is commented out
     *
     * \return true if a name of one of those extensions was found, even when
     * the file itself could not be found nor opened
     */
    bool import(const QTextCursor &);

    /*!
     * look if the qtextcursor points to a library
     * \return the library name
     *
     * \note a library is the name of a shared object between simple quotes; the
     * quotes are not part of the returned name
     *
     * \note an empty string is returned when the cursor is not on such a name
     */
    QString libraryNameUnderCursor(const QTextCursor &);

    /*!
     * \brief create the actions of the mode
     *
     * \note this implementation does nothing: the documentation actions it
     * created from the files of the licos installation are commented out, which
     * is why \c getSpecificMenu has no "Documentations" sub-menu
     */
    virtual void createActions();

    /*!
     * \brief create the actions which search a word in the input files of the
     * licos distribution
     *
     * \note this implementation does nothing: those actions are commented out,
     * as are the menus and the slot which used them
     */
    virtual void createSearchActions();

    /*!
     * \brief create the action which analyses a library with the `mfm`
     * executable
     * \param[in] l : name of the library the action is to be associated with
     *
     * \note the parameter is ignored and nothing is created: the action is
     * commented out, as is the code which added it to the context menu
     */
    virtual void createAnalyseUsingMFMAction(const QString &);

    /*!
     * \brief search a word in the files of a given extension, recursively, from
     * the given directory, and show the result in a secondary task named
     * "*search for ... *" whose major mode is "grep output"
     * \param[in] s : word to search
     * \param[in] w : directory the search starts from
     * \param[in] ext : extension of the files to search, without the dot
     *
     * \note the search runs `grep -nH` in a shell, so the `SHELL` environment
     * variable must be defined: the reason for its absence is reported in the
     * minibuffer and no process is started
     *
     * \note nothing at all is done when \c w is empty
     */
    virtual void search(const QString &, const QString &, const QString &);

    /*!
     * \return the value of the `LICOSHOME` environment variable, the
     * installation directory of licos
     *
     * \note the absence of the variable is reported in the minibuffer and an
     * empty string is returned
     */
    virtual QString getLicosPath() const;

    /*!
     * \return the value of the `MFRONTMATERIALSHOME` environment variable, the
     * installation directory of `MFrontMaterials`
     *
     * \note the absence of the variable is reported in the minibuffer and an
     * empty string is returned
     */
    virtual QString getMFrontMaterialsPath() const;

    /*!
     * \return the value of the `LICOSSTUDIESHOME` environment variable, the
     * directory where the studies of licos are
     *
     * \note the absence of the variable is reported in the minibuffer and an
     * empty string is returned
     *
     * \note only the search actions which are commented out use this directory
     */
    virtual QString getLicosStudiesPath() const;

    /*!
     * \brief analyse a library with the `mfm` executable and show its output in
     * a secondary task named "*mfm results for ... *"
     * \param[in] l : name of the library to analyse
     *
     * \note the library is looked for in the directories of
     * `LD_LIBRARY_PATH`; when that variable is not set or the library is not
     * found in those directories, the reason is reported in the minibuffer and
     * no process is started
     *
     * \note no menu nor shortcut calls this method anymore: the action which
     * used to do so is commented out
     */
    virtual void analyseUsingMFM(const QString &);

    /*!
     * \brief open the source of an MFront behaviour, property or model in a new
     * buffer
     * \param[in] f : name of the file to open
     *
     * \note the file is opened when \c f points to it and the search then goes
     * on: in the `mfront/properties`, `mfront/behaviours` and `mfront/models`
     * sub-directories of the current directory, then in
     * `MFRONTMATERIALSHOME`, where every file whose name is \c f is opened
     *
     * \note a file found in one of those sub-directories is opened under the
     * name \c f, not under the path which was tested
     *
     * \note an unreadable file and an absolute path stop the search and are
     * reported in the minibuffer; so does the end of the search, whether it has
     * found files in `MFRONTMATERIALSHOME` or nothing at all
     */
    virtual void openMFrontSource(const QString &);

    /*!
     * completer of the keywords of the language and of the names of the blocks
     * and their `EndOf` counterpart
     *
     * \note it is a child of the text edit it completes, which owns it
     */
    QCompleter *c;

    /*!
     * regular expression which matches the name of a shared object between
     * simple quotes; the name itself is the captured string
     *
     * \note it is used by \c libraryNameUnderCursor
     */
    QRegularExpression rlib;

    //     //! run licos
    //     QAction *ra;
    //     //! run licos in dry-run mode
    //     QAction *dra;
    //
    //     //! add material property
    //     QAction *ampa;
    //
    //     //! material wizard action
    //     QAction *mwa;
    //     //! thermal behaviour wizard action
    //     QAction *tbwa;
    //     //! mechanical behaviour wizard action
    //     QAction *mbwa;
    //
    //     //! open data file in tplot
    //     QAction *ta1;
    //     //! open licos curve in tplot
    //     QAction *ta2;
    //
    //     //! analyse a library using mfm
    //     QAction *mfma;
    //
    //     //! open mfront source
    //     QAction *msrca;
    //
    //     //! search in licos input files in Licos
    //     QAction *lsil;
    //     //! search in licos input files in MFrontMaterials
    //     QAction *lsim;
    //     //! search in licos input files in LicosStudies
    //     QAction *lsis;
    //     //! search in licos input files in
    //     //  Licos+MFrontMaterials+LicosStudies
    //     QAction *lsia;
    //
    //     //! search in mfront input files in Licos
    //     QAction *msil;
    //     //! search in mfront input files in MFrontMaterials
    //     QAction *msim;
    //     //! search in mfront input files in LicosStudies
    //     QAction *msis;
    //     //! search in mfront input files in
    //     //  Licos+MFrontMaterials+LicosStudies
    //     QAction *msia;
    //
    //     QAction *ddoca;
    //     QAction *udoca;
    //     QAction *tdoca;
    //     QAction *l10doca;
    //     QAction *l11doca;
    //     QAction *idoca;
    //     QAction *mdoca;
    //     QVector<QAction *> tudoca;

   private:
    Q_OBJECT

  };  // end of struct LicosMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSMAJORMODE_HXX */
