/*!
 * \file   LineEdit.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   10/03/2018
 */

#ifndef LIB_TFEL_GUI_LINEEDIT_HXX
#define LIB_TFEL_GUI_LINEEDIT_HXX

#include <QtWidgets/QLineEdit>
#include <TFEL/GUI/Config.hxx>

namespace tfel{

  namespace gui{

  // forward declaration
  struct EditorWidget;

  struct TFEL_GUI_VISIBILITY_EXPORT LineEdit : QLineEdit {
    /*!
     * \brief constructor
     * \paren[in] q: editor emacs widget
     * \paren[in] p: parent widget
     */
    LineEdit(EditorWidget &, QWidget *const = nullptr);

    virtual void setInputHistory(const QStringList &);

    virtual QStringList getInputHistory() const;

    void keyPressEvent(QKeyEvent *) override;
    //! destructor
    ~LineEdit() override;

   protected:
    /*!
     * treat "Ctrl-k1 Mod-k2"
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    virtual void handleShortCut(const int,
                                const Qt::KeyboardModifiers,
                                const int);
    EditorWidget &editor;
    QStringList inputHistory;
    int pring = 0;
    bool ctrlx = false;
    bool ctrlc = false;
    bool yank = false;
    int pHistory = 0;
    bool hMove = false;

   private:
    Q_OBJECT
  };

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_LINEEDIT_HXX */
