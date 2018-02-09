/*! 
 * \file  CSyntaxHighlighterBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef LIB_QEMACS_CSYNTAXHIGHLIGHTERBASE_HXX
#define LIB_QEMACS_CSYNTAXHIGHLIGHTERBASE_HXX 

#include<QtCore/QString>
#include<QtCore/QVector>

#include<QtGui/QTextDocument>
#include<QtGui/QSyntaxHighlighter>

#include"TFEL/Utilities/CxxTokenizerOptions.hxx"
#include"QEmacs/Config.hxx"

namespace qemacs {

/*!
*base class for language based on C
*(such as C/C++/mfront/licos)
*/
class QEMACS_VISIBILITY_EXPORT CSyntaxHighlighterBase
    : public QSyntaxHighlighter {

  Q_OBJECT

public:
  virtual void highlightBlock(const QString &) override;

protected:
  /*!
   * a simple wrapper around the highligthing rule
   */
  struct HighlightingRule {
    std::string key;
    QTextCharFormat format;
  }; // end of struct HighlightingRule

  CSyntaxHighlighterBase(QTextDocument *const);

  //! list of highlighting rules
  QVector<HighlightingRule> highlightingRules;

  QTextCharFormat keyFormat;
  QTextCharFormat numberFormat;
  QTextCharFormat commentFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat preprocessorFormat;

  tfel::utilities::CxxTokenizerOptions options;

  }; // end of struct CSyntaxHighlighter
  
} // end of namespace qemacs


#endif /* LIB_QEMACS_CSYNTAXHIGHLIGHTERBASE_HXX */

