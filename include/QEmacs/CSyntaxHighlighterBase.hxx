/*! 
 * \file  CSyntaxHighlighterBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#ifndef LIB_QEMACS_CSYNTAXHIGHLIGHTERBASE_HXX
#define LIB_QEMACS_CSYNTAXHIGHLIGHTERBASE_HXX 

#include<vector>
#include<QtCore/QString>
#include<QtGui/QTextDocument>
#include<QtGui/QSyntaxHighlighter>

#include"TFEL/Utilities/CxxTokenizerOptions.hxx"
#include"QEmacs/Config.hxx"

namespace qemacs {

  /*!
   * \brief base class for language based on `C`
   * (such as C/C++/mfront/mtest/licos)
   */
  struct QEMACS_VISIBILITY_EXPORT CSyntaxHighlighterBase
      : public QSyntaxHighlighter {

    virtual void highlightBlock(const QString &) override;

   protected:
    /*!
     * a simple wrapper around the highligthing rule
     */
    struct HighlightingRule {
      std::string key;
      QTextCharFormat format;
    };  // end of struct HighlightingRule

    CSyntaxHighlighterBase(QTextDocument *const);

    //! list of highlighting rules
    std::vector<HighlightingRule> highlightingRules;

    QTextCharFormat keyFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat preprocessorFormat;

    tfel::utilities::CxxTokenizerOptions options;

   private:
    Q_OBJECT
  }; // end of struct CSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_CSYNTAXHIGHLIGHTERBASE_HXX */
