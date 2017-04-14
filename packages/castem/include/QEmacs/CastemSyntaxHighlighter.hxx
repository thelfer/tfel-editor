/*! 
 * \file  CastemSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 août 2012
 */

#ifndef _LIB_CASTEMSYNTAXHIGHLIGHTER_H_
#define _LIB_CASTEMSYNTAXHIGHLIGHTER_H_ 

#include<QtGui/QSyntaxHighlighter>

namespace qemacs
{
  
  struct CastemSyntaxHighlighter
    : public QSyntaxHighlighter
  {
    CastemSyntaxHighlighter(QTextDocument*);

    virtual void
    highlightBlock(const QString &) override;

  protected:

    /*!
     * a simple wrapper around the highligthing rule
     */
    struct HighlightingRule
    {
      QRegExp         pattern;
      QTextCharFormat format;
    }; // end of struct HighlightingRule

    //! list of highlighting rules
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keyFormat;

    QTextCharFormat commentFormat;

    QTextCharFormat quotationFormat;

    // for characters whose position is greater than 80 (those
    // caracters will be ignored by Cast3M)
    QTextCharFormat outOfBoundsFormat;

  }; // end of struct CastemSyntaxHighlighter

} // end of namespace qemacs

#endif /* _LIB_CASTEMSYNTAXHIGHLIGHTER_H */

