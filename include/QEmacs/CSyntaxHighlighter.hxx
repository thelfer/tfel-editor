/*! 
 * \file  CSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_CSYNTAXHIGHLIGHTER_H_
#define _LIB_QEMACS_CSYNTAXHIGHLIGHTER_H_ 

#include"QEmacs/CSyntaxHighlighterBase.hxx"

namespace qemacs
{

  /*!
   * a syntaxt highlighter for C-based languages
   */
  class QEMACS_VISIBILITY_EXPORT CSyntaxHighlighter
    : public CSyntaxHighlighterBase
  {

    Q_OBJECT

  public:

    /*!
     * \param[in] t : text document to be highlighted
     */
    CSyntaxHighlighter(QTextDocument *const);

  }; // end of struct CSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* _LIB_QEMACS_CSYNTAXHIGHLIGHTER_H */

