/*!
 * \file   QEmacsShellProcessLineEdit.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#ifndef LIB_QEMACS_QEMACSSHELLPROCESSLINEEDIT_HXX
#define LIB_QEMACS_QEMACSSHELLPROCESSLINEEDIT_HXX

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsProcessLineEdit.hxx"

namespace qemacs{

  /*!
   * An helper class asking the user for a shell command and
   * displaying the results in a ProcessOutputFrame created as a slave
   * of the current buffer
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsShellProcessLineEdit
      : public QEmacsProcessLineEdit {
    /*!
     * \param[in] l : label
     * \param[in] c : command guess
     * \param[in] m : mode used to display the results
     * \param[in] p : parent
     */
    QEmacsShellProcessLineEdit(const QString &,
                               const QString &,
                               const QString &,
                               QEmacsWidget &);
    //! destructor
    ~QEmacsShellProcessLineEdit() override;

   protected:
    void treatUserInput() override;

   private:
    Q_OBJECT

  };  // end of QEmacsShellProcessLineEdit

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSSHELLPROCESSLINEEDIT_HXX */
