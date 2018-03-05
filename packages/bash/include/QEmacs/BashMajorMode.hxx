/*!
 * \file  BashMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#ifndef LIB_QEMACS_BASHMAJORMODE_HXX
#define LIB_QEMACS_BASHMAJORMODE_HXX

#include "QEmacs/ShMajorMode.hxx"

namespace qemacs {

  //! \brief a major mode for editing `bash` files
  struct BashMajorMode : public ShMajorMode {
    BashMajorMode(QEmacsWidget &, QEmacsBuffer &, QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    ~BashMajorMode() override;

   private:
    Q_OBJECT
  };  // end of struct BashMajorMode

}  // end of namespace qemacs

#endif /* LIB_QEMACS_BASHMAJORMODE_HXX */
