/*!
 * \file   QEmacsLineEdit.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   10/03/2018
 */

#ifndef LIB_QEMACS_QEMACSLINEEDIT_HXX
#define LIB_QEMACS_QEMACSLINEEDIT_HXX

#include <QtWidgets/QLineEdit>
#include <QEmacs/Config.hxx>

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;

  struct QEMACS_VISIBILITY_EXPORT QEmacsLineEdit : QLineEdit {
    /*!
     * \brief constructor
     * \paren[in] q: qemacs emacs widget
     * \paren[in] p: parent widget
     */
    QEmacsLineEdit(QEmacsWidget &, QWidget *const = nullptr);

    virtual void setInputHistory(const QStringList &);

    virtual QStringList getInputHistory() const;

    void keyPressEvent(QKeyEvent *) override;
    //! destructor
    ~QEmacsLineEdit() override;

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
    QEmacsWidget &qemacs;
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSLINEEDIT_HXX */
