/*!
 * \file  FindReplaceDialog.hxx
 * \brief modeless dialog which searches for a text or a regular expression and
 *        replaces what it finds.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#ifndef LIB_TFEL_GUI_FINDREPLACEDIALOG_HXX
#define LIB_TFEL_GUI_FINDREPLACEDIALOG_HXX

#include <QtCore/QString>

#include <QtGui/QIcon>

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SearchPattern.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEditBase;

  /*!
   * \brief dialog which searches a text or a regular expression through the
   * buffer worked on and replaces what it finds
   *
   * This is the `Edit` menu entry `Find/Replace...` and the `find-replace`
   * command. The dialog is modeless: the buffers stay editable while it is
   * shown, which is the reason it exists beside the incremental search of
   * `Ctrl-S` and the query replace of `Alt-%`, the two commands which hold the
   * minibuffer while they search.
   *
   * \note an editor holds one such dialog, named `find-replace` and reused when
   * the entry is chosen again, so what was typed in its fields is kept and a
   * second call only brings the dialog back to the front. The dialog belongs to
   * the editor and not to a buffer: it asks the buffer worked on for its text
   * edit on every button, which is what lets a `Ctrl-X b` change of buffer reach
   * the dialog instead of leaving it behind with the buffer it was raised on.
   *
   * \note the three options decide what a match is, and the search is built
   * again on every question so that a tick which changes under a button changes
   * that button's answer: the letter case, the whole words, which ask that a
   * match stands between two characters which are neither a letter nor a digit,
   * and the regular expression, which reads the find field as an expression of
   * `QRegularExpression` and gives `\1` to `\9` to the replace field.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT FindReplaceDialog : public QDialog {
    Q_OBJECT
   public:
    /*!
     * \brief constructor
     * \param[in,out] editor : editor the dialog searches the buffer of and which
     * owns it once it is built
     *
     * \note the editor is the parent of the dialog: it is destroyed with the
     * window it belongs to, which is what `showFor` relies on to find again the
     * dialog an editor already holds
     *
     * \note the object name of the dialog is `find-replace`, which is how
     * `showFor` finds the dialog an editor already holds, and how a test singles
     * the dialog and its status out
     */
    explicit FindReplaceDialog(EditorWidget& editor);

    /*!
     * \brief shows the dialog of an editor, building it the first time it is
     * asked for
     * \param[in,out] editor : editor the dialog works on
     *
     * \note the find field is filled with the selection of the text edit when it
     * holds nothing and that selection is a text of one line, which is the way a
     * search for what is on the screen starts.
     */
    static void showFor(EditorWidget& editor);

   protected:
    /*!
     * \brief what asking for a match answers, which is the three answers the two
     * buttons which only search have to tell apart
     */
    enum class Outcome {
      //! a match was selected and the line under the fields has nothing to say
      Found,
      //! nothing was found ahead of the place and nothing behind it either
      NotFound,
      //! a match was selected, on the far side of the buffer
      Wrapped,
    };  // end of enum class Outcome

    /*!
     * \brief selects the next match of a pattern, from the selection the text
     * edit holds, and wraps once
     * \param[in] search : the pattern
     * \param[in] backward : look before the selection instead of after it
     * \return what the question answered, which the caller reports
     *
     * The line under the fields is not touched here: the two buttons which only
     * search report what this answers, and the button which replaces reports what
     * it wrote and lets the walk it uncovers say nothing.
     */
    Outcome selectNextMatch(const SearchPattern& search,
                            const bool backward = false);

    /*!
     * \brief builds the fields, the three options, the four buttons and the line
     * which says what the last question answered
     */
    void createWidgets();

    /*!
     * \brief the pattern the fields and the three options answer
     * \return the pattern, whose `isValid` answers false and `errorString` says
     * what is wrong when the find field does not hold a valid expression
     */
    SearchPattern pattern() const;

    /*!
     * \brief the text edit the questions are asked of
     * \return the main frame of the buffer worked on
     */
    TextEditBase& textEdit() const;

    /*!
     * \brief selects the next match, or the previous one
     * \param[in] backward : look before the selection instead of after it
     *
     * The search starts from the end of the selection the text edit shows, or
     * from its start when searching backward, so a match the selection already
     * holds is passed over, and from the cursor when nothing is selected.
     *
     * A document which holds no match ahead of that place is searched from its
     * other end, which is reported: a search which wraps in silence loses a
     * match standing behind the cursor to a reader who looks ahead.
     */
    void find(const bool backward = false);

    /*!
     * \brief replaces the selection when it is a match and asks nothing else
     * \return true when a match was replaced
     *
     * A selection which is no match is left where it is and the next match is
     * searched for, which is the way the button walks the buffer one press at a
     * time.
     */
    bool replaceSelected();

    /*!
     * \brief replaces every match of the buffer worked on
     *
     * The walk starts at the cursor and reaches the end of the buffer, then goes
     * back to the cursor, as a replacement asked from the minibuffer does. What
     * was replaced, or that nothing was, is the answer of the dialog.
     */
    void replaceAll();

    /*!
     * \brief says what the last question answered
     * \param[in] message : what the line under the fields shows
     *
     * The line is the only place those answers go: the minibuffer belongs to the
     * commands and the dialog is modeless, so a message there would be a message
     * the reader has to look away from the dialog for.
     */
    void report(const QString& message);

    //! \brief the answer of a button whose question found nothing
    void reportNotFound(const QString& what);

   protected slots:
    //! \brief searches for the next match
    void findNext();
    //! \brief searches for the previous match
    void findPrevious();
    //! \brief replaces the selection when it is a match
    void replaceOne();
    //! \brief replaces every match of the buffer
    void replaceAllMatches();
    /*!
     * \brief hides the dialog, which leaves the selection the last search made
     * where it is
     */
    void closeDialog();

   protected:
    /*!
     * \brief handles the two keys which are not a button of their own
     *
     * `Return` is the next match and `Shift-Return` the previous one, which is
     * how the two directions are asked without taking the hand off the fields.
     * The rest is the usual editing of a dialog, `Escape` included, which hides
     * it.
     */
    void keyPressEvent(QKeyEvent*) override;

   protected:
    //! editor the buffer worked on belongs to
    EditorWidget& editor;
    //! text the search looks for, or the expression it looks with
    QLineEdit* findField = nullptr;
    //! text a match is rewritten to
    QLineEdit* replaceField = nullptr;
    //! option which asks a match for the letter case of the pattern
    QCheckBox* caseOption = nullptr;
    //! option which asks a match for standing between two word boundaries
    QCheckBox* wordsOption = nullptr;
    //! option which reads the find field as a regular expression
    QCheckBox* regexpOption = nullptr;
    //! button which searches for the next match
    QAbstractButton* findNextButton = nullptr;
    //! button which searches for the previous match
    QAbstractButton* findPreviousButton = nullptr;
    //! button which replaces the selection it shows
    QAbstractButton* replaceButton = nullptr;
    //! button which replaces every match of the buffer
    QAbstractButton* replaceAllButton = nullptr;
    //! button which hides the dialog and leaves what it selected alone
    QAbstractButton* closeButton = nullptr;
    //! label which says what the last question answered
    QLabel* status = nullptr;
  };  // end of struct FindReplaceDialog

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_FINDREPLACEDIALOG_HXX */
