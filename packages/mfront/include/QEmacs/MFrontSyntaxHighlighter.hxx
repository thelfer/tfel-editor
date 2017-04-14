/*! 
 * \file  MFrontSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_H_
#define _LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_H_ 

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

#endif /* _LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_H */

