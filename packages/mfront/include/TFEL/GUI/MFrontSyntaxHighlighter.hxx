/*!
 * \file  MFrontSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/07/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_HXX

#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"

namespace tfel::gui {

  //! a syntax highlighter for the MFront code generator
  struct MFrontSyntaxHighlighter : public CxxSyntaxHighlighter {
    /*!
     * \param[in] t : text document to be highlighted
     * \param[in] n : dsl name
     */
    MFrontSyntaxHighlighter(QTextDocument *const, const QString &);
    //! \brief destructor
    ~MFrontSyntaxHighlighter() override;

   protected:
    QTextCharFormat mfrontKeyFormat;

   private:
    Q_OBJECT
  };  // end of struct MFrontSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_H */
