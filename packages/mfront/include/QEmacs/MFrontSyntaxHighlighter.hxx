/*! 
 * \file  MFrontSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_HXX 

#include"QEmacs/CxxSyntaxHighlighter.hxx"

namespace qemacs
{

  //! a syntax highlighter for the MFront code generator
  struct MFrontSyntaxHighlighter : public CxxSyntaxHighlighter {
    /*!
     * \param[in] t : text document to be highlighted
     * \param[in] n : dsl name 
     */
    MFrontSyntaxHighlighter(QTextDocument *const, const QString &);
    //! destructor
    ~MFrontSyntaxHighlighter();

   protected:
    QTextCharFormat mfrontKeyFormat;

   private:
    Q_OBJECT
  }; // end of struct MFrontSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_H */

