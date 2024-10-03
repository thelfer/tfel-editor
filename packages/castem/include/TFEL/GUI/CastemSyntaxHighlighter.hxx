/*!
 * \file  CastemSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   05/08/2012
 */

#ifndef LIB_CASTEMSYNTAXHIGHLIGHTER_HXX
#define LIB_CASTEMSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>

namespace tfel::gui {

  struct CastemSyntaxHighlighter : public QSyntaxHighlighter {
    CastemSyntaxHighlighter(QTextDocument *);

    virtual void highlightBlock(const QString &) override;

   protected:
    /*!
     * a simple wrapper around the highligthing rule
     */
    struct HighlightingRule {
      QRegularExpression pattern;
      QTextCharFormat format;
    };  // end of struct HighlightingRule

    //! list of highlighting rules
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keyFormat;

    QTextCharFormat commentFormat;

    QTextCharFormat quotationFormat;

    // for characters whose position is greater than 80 (those
    // caracters will be ignored by Cast3M)
    QTextCharFormat outOfBoundsFormat;

  };  // end of struct CastemSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_CASTEMSYNTAXHIGHLIGHTER_H */
