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

  /*!
   * a syntaxt highlighter for the MFront code generator
   */
  class MFrontSyntaxHighlighter
    : public CxxSyntaxHighlighter
  {

    Q_OBJECT

  public:

    /*!
     * \param[in] t : text document to be highlighted
     * \param[in] n : dsl name 
     */
    MFrontSyntaxHighlighter(QTextDocument *const,
			    const QString&);
    
  protected:

    QTextCharFormat mfrontKeyFormat;

  }; // end of struct MFrontSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_H */

