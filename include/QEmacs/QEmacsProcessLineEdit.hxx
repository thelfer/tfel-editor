/*!
 * \file   QEmacsProcessLineEdit.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#ifndef LIB_QEMACS_QEMACSPROCESSLINEEDIT_HXX
#define LIB_QEMACS_QEMACSPROCESSLINEEDIT_HXX

#include"QEmacs/Config.hxx"
#include "QEmacs/QEmacsCommandLine.hxx"

namespace qemacs{

  /*!
   * An helper class asking the user for a command and displaying the
   * results in a ProcessOutputFrame created as a slave of the current
   * buffer
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsProcessLineEdit
      : public QEmacsCommandLine {
    /*!
     * \param[in] l : label
     * \param[in] c : command guess
     * \param[in] m : mode used to display the results
     * \param[in] p : parent
     */
    QEmacsProcessLineEdit(const QString &,
                          const QString &,
                          const QString &,
                          QEmacsWidget &);
    //! destructor
    ~QEmacsProcessLineEdit() override;

   protected:
    void treatUserInput() override;

    virtual void run(const QString &,
                     const QString &,
                     const QStringList &);
    //! major mode for the process output frame
    const QString mode;

   private:
    Q_OBJECT
  };  // end of QEmacsProcessLineEdit

} // end of namespace qemacs

#endif /* LIB_QEMACSPROCESSLINEEDIT_HXX */
