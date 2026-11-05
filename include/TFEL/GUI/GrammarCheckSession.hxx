/*!
 * \file  GrammarCheckSession.hxx
 * \brief the grammar checker of a text edit: a `harper-ls` session of its own,
 *        the lints it draws under the text, and the menu which steers it.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#ifndef LIB_TFEL_GUI_GRAMMARCHECKSESSION_HXX
#define LIB_TFEL_GUI_GRAMMARCHECKSESSION_HXX

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointer>

#include "TFEL/GUI/Config.hxx"

// forward declarations
class QAbstractScrollArea;
class QEvent;
class QMenu;
class QObject;
class QTextDocument;

namespace tfel::gui {

  // forward declarations
  struct EditorWidget;
  struct TextEditBase;
  struct LspClient;

  /*!
   * \brief a grammar checker and the buffer it reads
   *
   * \note one session belongs to one text edit, the way a highlighter does: the
   * checker is asked about the text of a frame and answers about that text
   * only, so a session is held by the mode of the frame, is built with it and
   * dies with it, and a buffer which is not English anymore loses its lints
   * with the mode it loses
   *
   * \note the client is a child of the session and speaks with one `harper-ls`
   * process: unlike \c clangd, which holds an index shared by every file of a
   * project and is therefore one per editor widget, a grammar checker holds
   * nothing but the documents it was given, and two of them sharing a process
   * would buy a startup saved and a state to tell apart
   *
   * \note the checker reads the whole text it is given and is sent the whole
   * text on every change, see \c LspClient::setFullTextSync; the dialect it
   * reads the text as is answered to it on every question it asks, see
   * \c LspHarperSupport::configuration, so \c setDialect reaches a running
   * session without a restart
   *
   * \note a session of a buffer takes the spell checker of hunspell off that
   * buffer while it runs: the two say what is wrong with a word in the same
   * place, in different colors, and a person who turned the grammar checker on
   * is not asking to read both. It is the modes which do it, since the spell
   * checker is theirs, see \c MajorModeBase::setSpellCheckLanguage and the
   * \c stateChanged signal
   *
   * \note the lints come as diagnostics of the protocol, with the severity the
   * server defaults to — a hint, which is why their color is one color and not
   * that of their severity, and why the whole output of the checker is to be
   * seen; what is underlined is drawn as the `harper` contribution to the extra
   * selections of the text edit, so the current line, the diagnostics of
   * \c clangd and the colors of the identifiers are untouched by it. Resting
   * the mouse on an underlined run says what the lint is, as a tooltip
   */
  struct TFEL_GUI_VISIBILITY_EXPORT GrammarCheckSession : QObject {
    Q_OBJECT
   public:
    /*!
     * \brief constructor
     * \param[in] w : editor widget the buffer belongs to, which the minibuffer
     * messages of the conversation are displayed on
     * \param[in] t : text edit whose text is read, and which draws the lints
     * under it
     * \param[in] language_id : what the documents are called in the sense the
     * server parses them with: `cpp` for a C++ buffer, `latex`, `markdown`
     * \param[in] p : parent object, which is the mode that owns the session
     */
    GrammarCheckSession(EditorWidget &w,
                        TextEditBase &t,
                        const QString &language_id,
                        QObject *const p = nullptr);
    /*!
     * \brief destructor: ends the session, if any, without touching the text
     * edit anymore, which may be dying around us
     */
    ~GrammarCheckSession() override;
    /*!
     * \return true when a grammar checker is to be found: the entries which
     * start one are worth showing only then, and a menu which offered a program
     * that is not there would report a failure on every try
     */
    static bool isAvailable();
    /*!
     * \return true when the buffer is checked: a session is running, or on its
     * way, which is what the checkable entry of a menu says
     */
    bool isActive() const;
    /*!
     * \brief start a session on the text edit, or end the one which runs
     * \param[in] on : true to check the buffer, false to stop
     * \param[in] remembered : write the choice in the setting
     * `harper/launch-by-default`, which is what the buffers opened afterwards
     * start with; the constructor launching a buffer on its own accord passes
     * false, since that answer is already the setting
     * \param[in] announced : report the outcome in the minibuffer; false for a
     * session which follows the setting of its own accord
     *
     * \note launching a session which runs is not asked twice: the session is
     * left alone, and `remembered` still writes what was chosen, which is what
     * makes an entry which is already checked still set the default
     */
    void setActive(const bool on,
                   const bool remembered = true,
                   const bool announced = true);
    /*!
     * \brief read the text as a variety of English, under a running session or
     * the next one
     * \param[in] name : one of the names of \c LspHarperSupport::dialects
     *
     * \note the setting is written, the client re-answered with the settings,
     * and the documents of the session are sent their whole text again: a text
     * which changes by itself never would, and the lints of the buffer would
     * keep saying what the dialect which is gone said of them. The menus of the
     * other buffers name the new variety as they are built again, which is what
     * \c addToMenu reads from the setting
     */
    void setDialect(const QString &name);
    /*!
     * \brief add the entry which turns the checker on and off and the submenu
     * of the varieties to the given menu \param[in] menu : menu to complete;
     * the entries are children of it and refer to this session, so they follow
     * it, not the menu \param[in] toggle : name the checkable entry is offered
     * under, which each mode spells for itself: the entry of a mode whose whole
     * text is English says "(English only)", since that entry says nothing
     * about the buffer a person is reading, where the checker reads every word
     * of it, comments included, and only the English it is told to read
     *
     * \note the submenu is the five varieties of \c LspHarperSupport::dialects,
     * the one which is chosen ticked; choosing one writes the setting and makes
     * a running session read its documents again, see \c setDialect
     */
    void addToMenu(QMenu &menu, const QString &toggle);
    /*!
     * \brief show the lints standing under the mouse as a tooltip
     *
     * \note the filter is installed on the viewport of the text edit by the
     * constructor
     */
    bool eventFilter(QObject *const watched, QEvent *const event) override;
   signals:
    /*!
     * \brief the buffer is checked, or is not: what the spell checker of the
     * mode has to follow, see the note in the class documentation
     */
    void stateChanged();

   private slots:
    //! the checker published lints: draw them when they are of this buffer
    void updateDiagnostics(const QString &file);
    /*!
     * \brief follow the buffer to the file it visits now, and to its new name;
     * the previous name is released on the server
     */
    void openOrRenameDocument();

   private:
    //! draw the lints of the buffer, or remove them when there are none
    void applyDiagnostics();
    /*!
     * \return the text of the lints whose range visits the given position,
     * empty when none does; several when the checker has several things to say
     * about one run of text, which it has
     */
    QString diagnosticAt(const QTextDocument &document, int position) const;
    //! the editor widget to report to
    EditorWidget *editor;
    //! the text edit whose text is read, and which draws the lints
    TextEditBase *text_edit;
    /*!
     * \brief the Qt widget behind \c text_edit, the viewport of which this
     * session filters for the tooltips
     *
     * \c QPointer, and what the destructor checks before drawing: the session
     * dies as a child of the mode, which dies as a child of the text edit, and
     * the Qt widget of that text edit is a child created before the mode and
     * therefore destroyed before it — the lifetime trap of issue #20. A mode
     * which is only replaced, as `setMajorMode` does, leaves it alive, and that
     * is when the lints of the session which is going have to be taken off the
     * text, by hand, from there
     */
    QPointer<QAbstractScrollArea> area;
    //! the client of the checker, a child of this session
    LspClient *client;
    /*!
     * \brief true when the buffer is to be checked: what \c isActive answers
     *
     * This is the state a person chose, not the state of a process: between the
     * entry which started a session and the handshake which completed it, the
     * entry is checked and the checker is not answering yet. It goes false when
     * the checker exits of its own accord, since nothing is checking the buffer
     * anymore whatever was chosen
     */
    bool active = false;
    //! the file name the server knows this buffer by, empty if none
    QString document_file;
    /*!
     * \brief the document opened on the server under \c document_file
     *
     * A \c QPointer, never a call to `text_edit->document()`: see
     * \c CxxMajorMode::document, the lifetime trap of issue #20
     */
    QPointer<QTextDocument> document;
  };  // end of struct GrammarCheckSession

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GRAMMARCHECKSESSION_HXX */
