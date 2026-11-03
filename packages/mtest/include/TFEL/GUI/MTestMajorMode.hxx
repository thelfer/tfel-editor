/*!
 * \file  MTestMajorMode.hxx
 * \brief MTestMajorMode: major mode for MTest files, runs mtest and tplot.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_MTESTMAJORMODE_HXX
#define LIB_TFEL_GUI_MTESTMAJORMODE_HXX

#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel::gui {

  //! forward declaration
  struct TextEditBase;

  /*!
   * \brief major mode for editing `MTest` input files
   *
   * The mode derives from \c CxxMajorMode, so indentation and formatting are
   * delegated to \c clang-format, which is called with the C++ dialect
   * (\c -assume-filename=cxx) and the style file of the directory of the
   * buffer (\c -style=file).
   *
   * \note the keywords are those of the MTest grammar, a static list shared
   * with the syntax highlighter
   */
  struct MTestMajorMode : public CxxMajorMode {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     *
     * \note a \c QCompleter is created on the keywords of the mode; it is a
     * child of the text edit \c t, which \c setWidget also designates as the
     * widget it completes, as
     * \c MajorModeBase::completeCurrentWord ignores a completer whose widget is
     * another text edit
     */
    MTestMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "MTest", the name the mode is registered under
    QString getName() const override;

    //! \return "major mode dedicated to the mtest code"
    QString getDescription() const override;

    /*!
     * \brief add a "Help on ..." entry to the context menu when the line the
     * cursor stands on starts with a keyword of the mode
     * \param[in] m : menu to complete
     * \param[in] tc : text cursor at the position where the menu appears
     *
     * \note the entry is inserted at the top of the menu, preceded by a
     * separator, and runs `mtest --scheme=<scheme> --help-keyword=<keyword>`;
     * its output is displayed in a secondary task of the buffer
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \brief install the highlighter of the MTest grammar on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is installed through \c setHighlighter, so the
     * previous one is removed from the document and released
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \return the completer of the keywords of the mode
     *
     * \note the completer is a child of the text edit, not of the mode, and so
     * outlives the mode when the buffer keeps being edited with another one
     */
    QCompleter *getCompleter() override;

    /*!
     * \return the completion prefix
     *
     * The prefix is the current word preceded by \c '@' when that word is
     * introduced by an \c '@' token, as the keywords of MTest hold that
     * character, which the base implementation would drop
     */
    QString getCompletionPrefix() override;

    /*!
     * \return a menu named after the mode, with the actions which run `mtest`
     * on the buffer and plot its results with `tplot`, which import a behaviour
     * from a shared library or from the MFrontMaterials database, and which
     * insert one of the keywords of the mode
     *
     * \note a null pointer is returned when the parent of the mode is not a
     * widget, as the menu needs one to be a child of the user interface
     *
     * \note the caller takes ownership of the menu, so its actions refer to the
     * mode, not to the widget which displays the menu
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief handle \c Alt-m, which runs `mtest` on the file of the buffer with
     * the scheme of the mode; the buffer is saved first when it has been
     * modified
     *
     * \return true on \c Alt-m only; any other key is left to
     * \c CxxMajorMode, which uses that key for the compilation command
     */
    bool keyPressEvent(QKeyEvent *const) override;
    /*!
     * \brief destructor
     *
     * The completer is a child of the text edit, not of the mode, and so
     * survives the mode; the syntax highlighter is released by the base class
     * as it belongs to the document
     */
    ~MTestMajorMode() override;

   protected slots:
    /*!
     * \brief launch the import behaviour wizard: the user selects a behaviour
     * of a shared library and its material properties, and the corresponding
     * MTest declarations are inserted at the cursor when the wizard is accepted
     */
    virtual void showImportBehaviourWizard();
    /*!
     * \brief launch the import MFM behaviour dialog: the user selects a
     * behaviour of the MFrontMaterials database and its material properties,
     * and the corresponding MTest declarations are inserted at the cursor when
     * the dialog is accepted
     */
    virtual void showImportMFMBehaviourWizard();
    /*!
     * \brief method called when the `Show Results with TPlot` menu is
     * called
     *
     * `tplot` is started as a separate process on the \c .res file which has
     * the same name as the file of the buffer, in the same directory.
     *
     * \note nothing is run when the buffer is not associated with a file or
     * when the process can not be started; the reason is displayed in the
     * minibuffer
     */
    virtual void showResults();
    /*!
     * \brief insert the keyword of the action at the cursor, followed by a
     * blank
     * \param[in] a: action of the "Keywords" sub-menu, whose data holds the
     * keyword to insert
     *
     * \note nothing is inserted when the action carries no keyword, as is the
     * case for the separators of the menu
     *
     * \note the editor's cursor is moved to the end of the inserted text
     */
    virtual void insertKeyword(QAction *);

   protected:
    /*!
     * \return the list of MTest keywords, the static list used by the syntax
     * highlighter; derived modes return their own keywords
     */
    virtual const std::vector<std::string> &getKeyWordsList() const;
    /*!
     * \return the scheme name, that is the value given to the \c --scheme
     * option of \c mtest when the mode runs a study; the mode is registered on
     * the files of that scheme
     */
    virtual QString getScheme() const;
    /*!
     * \brief completer of the keywords returned by \c getKeyWordsList
     *
     * Created by the constructor of the mode, which owns it (issue #67), with
     * the text edit as widget; a derived mode replaces it with a completer
     * built on its own keywords
     */
    QCompleter *c = nullptr;

   private:
    Q_OBJECT

  };  // end of struct MTestMajorMode

  /*!
   * \brief run `mtest` on the file of the current buffer, with the "mtest"
   * scheme
   * \param[in] editor : editor widget
   *
   * The buffer is saved first when it has been modified, then the options of
   * the study are asked to the user and the output of the code is displayed in
   * a secondary task of the buffer.
   *
   * \note this is the implementation of the "run-mtest" command
   */
  void runMTest(EditorWidget &);
  /*!
   * \brief run `mtest` on the file of the current buffer, with the "ptest"
   * scheme, whatever the major mode of that buffer
   * \param[in] editor : editor widget
   *
   * \note this is the implementation of the "run-ptest" command
   */
  void runPTest(EditorWidget &);

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTESTMAJORMODE_H */
