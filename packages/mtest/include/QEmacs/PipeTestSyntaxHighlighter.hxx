/*! 
 * \file  PipeTestSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_H_
#define _LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_H_ 

#include<QtCore/QStringList>

#include"QEmacs/CxxSyntaxHighlighter.hxx"

namespace qemacs
{

  /*!
   * a syntaxt highlighter for the PipeTest code generator
   */
  class PipeTestSyntaxHighlighter
    : public CxxSyntaxHighlighter
  {
    Q_OBJECT
  public:
    //! \return the list of keys
    static QStringList&
    getPipeTestKeys(void);
    /*!
     * \param[in] t : text document to be highlighted
     */
    PipeTestSyntaxHighlighter(QTextDocument *const);
    //! \brief destructor
    virtual ~PipeTestSyntaxHighlighter();
  protected:
    //! \return the list of keys
    virtual QStringList getKeyWordsList(void) const;
    // format used for syntax highlighting
    QTextCharFormat mtestKeyFormat;
  }; // end of struct PipeTestSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* _LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_H */

