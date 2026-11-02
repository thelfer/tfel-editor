/*!
 * \file  MajorMode.hxx
 * \brief abstract interface all the major modes implement.
 * \author Thomas Helfer
 * \date   28/06/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODE_HXX
#define LIB_TFEL_GUI_MAJORMODE_HXX

#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtGui/QTextDocument>
#include <QtGui/QSyntaxHighlighter>
#include <QtWidgets/QMenu>
#include <QtWidgets/QCompleter>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;

  //! \brief abstract interface for all major modes
  struct TFEL_GUI_VISIBILITY_EXPORT MajorMode : public QObject {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    MajorMode(QWidget* const);
    //! \return the name of the major mode
    virtual QString getName() const = 0;
    //! \return the description of the major mode
    virtual QString getDescription() const = 0;
   public slots:
    //! update the major mode
    virtual void update() = 0;
   signals:
    //! signal send when the menu associated with the major mode is
    //! updated
    void updatedMenu();

   public:
    /*!
     * \brief hightlight the current document
     *
     * Implementations must install the highlighter through setHighlighter(),
     * which replaces the previous one. A QTextDocument accepts several
     * highlighters at once and they all apply, so simply leaking a new one
     * does not merely waste memory: it keeps the stale formatting active and
     * redoes the work on every refresh.
     */
    virtual void setSyntaxHighlighter(QTextDocument* const) = 0;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    virtual void completeContextMenu(QMenu* const, const QTextCursor&) = 0;

    /*!
     * \return the completer which provides the completions of the word under
     * the cursor, a null pointer if the mode does not complete anything
     *
     * \note the mode owns the completer and keeps it for its whole life: it is
     * created with the text edit it completes as widget, as
     * \c MajorModeBase::completeCurrentWord ignores a completer whose widget is
     * another text edit
     *
     * \note \c TextEditBase only queries this completer, it never creates nor
     * installs one
     */
    virtual QCompleter* getCompleter() = 0;
    /*!
     * \return the completion prefix
     * \note most of the time, it is this->textEdit.getCurrentWord()
     */
    virtual QString getCompletionPrefix() = 0;
    /*!
     * \return the minimal number of characters for a completion to be
     * considered
     */
    virtual int getMinimalCompletionLength() = 0;

    /*!
     * \brief insert a completion the user selected in the completion popup
     * \param[in] t : text edit the word is completed in
     * \param[in] w : completion to insert
     *
     * \note the default implementation of \c MajorModeBase does nothing when
     * the mode has no completer or when that completer belongs to another text
     * edit. It otherwise inserts the part of \c w which is missing from the
     * word, followed by a blank, and moves the editor's cursor
     */
    virtual void completeCurrentWord(TextEditBase&, const QString&) = 0;
    /*!
     * \return a menu specific to the current mode
     * \note the caller will take ownership of the menu
     * and may free the memory associated, including the
     * underlying QAction' objects. Therefore, the
     * returned menu shall not reference a member.
     */
    virtual QMenu* getSpecificMenu() = 0;
    //! \return the icon associated to the current mode
    virtual QIcon getIcon() const = 0;
    /*!
     * \brief format the text the cursor is in
     *
     * \note what is formatted depends on the mode: \c LaTeXMajorMode reflows
     * the paragraph the cursor belongs to, \c CMajorModeBase lets
     * \c clang-format rewrite the whole buffer and most of the other modes do
     * nothing
     *
     * \note the mode is responsible for restoring the editor's cursor, which
     * the reformatting would otherwise leave at the end of the rewritten text
     * (see \c MajorModeBase::AnchorMark and \c CMajorModeBase::indent)
     */
    virtual void format() = 0;
    //! \brief comment/uncomment a region
    virtual void comment() = 0;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     * \return true if the mode handled the event, in which case TextEditBase
     * stops there and applies no shortcut of its own
     */
    virtual bool keyPressEvent(QKeyEvent* const) = 0;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default behaviour
     * \return true if the mode handled the event, in which case TextEditBase
     * ignores it
     */
    virtual bool mousePressEvent(QMouseEvent* const) = 0;
    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
     * TextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    virtual bool handleShortCut(const int,
                                const Qt::KeyboardModifiers,
                                const int) = 0;
    //! \brief indent the whole text
    virtual void indentAll() = 0;
    //! \brief indent the current line
    virtual void indentLine(const QTextCursor&) = 0;
    //! \brief indent selected region
    virtual void indentRegion(const QTextCursor&) = 0;

    /*!
     * \brief set the language the spell checker of the mode uses
     * \param[in] l : language name, as it is named by the installed
     * dictionaries
     *
     * \note an unknown language, as well as the empty string, disables the
     * spell checking. Modes which do not spell check their text do nothing
     *
     * \note the modes which spell check their text rehighlight the document
     * so that the misspelled words are underlined in the new language
     */
    virtual void setSpellCheckLanguage(const QString&) = 0;
    /*!
     * \brief destructor
     *
     * The syntax highlighter installed by this mode is released: it is a child
     * of the document, so keeping it alive would leave it registered on that
     * document, still highlighting it after the mode is gone, whereas many
     * highlighters refer to their mode.
     */
    ~MajorMode() override;

   protected:
    /*!
     * \brief install the given syntax highlighter, replacing the one
     * currently in use by this mode.
     *
     * \param[in] h: highlighter to install. The highlighter is owned by the
     * document it was built with, as QSyntaxHighlighter requires; this method
     * only remembers it so that the next call, and the destructor, can
     * release it.
     *
     * A QTextDocument accepts several highlighters at once and they all
     * apply, so the previous one has to go: keeping it does not merely waste
     * memory, it keeps the stale formatting active and repeats the work on
     * every refresh.
     */
    void setHighlighter(QSyntaxHighlighter* const);

    /*!
     * \brief current syntax highlighter, null if the mode has none.
     *
     * A QPointer, not a raw pointer: the highlighter is owned by the document,
     * which may outlive it, and a raw pointer would then be dereferenced by
     * setSpellCheckLanguage() after the highlighter is gone.
     */
    QPointer<QSyntaxHighlighter> highlighter;

   private:
    Q_OBJECT

  };  // end of class MajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAJORMODE_HXX */
