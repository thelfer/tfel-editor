/*!
 * \file  BashMajorMode.hxx
 * \brief
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_BASHMAJORMODE_HXX
#define LIB_TFEL_GUI_BASHMAJORMODE_HXX

#include "TFEL/GUI/ShMajorMode.hxx"

namespace tfel::gui {

  //! \brief a major mode for editing `bash` files
  struct BashMajorMode : public ShMajorMode {
    BashMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    ~BashMajorMode() override;

   private:
    Q_OBJECT
  };  // end of struct BashMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_BASHMAJORMODE_HXX */
