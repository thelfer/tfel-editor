/*!
 * \file  CSyntaxHighlighterBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_CSYNTAXHIGHLIGHTERBASE_HXX
#define LIB_TFEL_GUI_CSYNTAXHIGHLIGHTERBASE_HXX

#include <string>
#include <vector>
#include <QtCore/QString>
#include <QtGui/QTextDocument>
#include <QtGui/QSyntaxHighlighter>

#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief base class for language based on `C`
   * (such as C/C++/mfront/mtest/licos)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CSyntaxHighlighterBase
      : public QSyntaxHighlighter {
    //! \brief flag indicating that an error occured while parsing previous
    //! lines
    static constexpr int invalidState = -2;
    //! \brief flag indicating that normal processing state
    static constexpr int defaultState = 0;
    //! \brief flag indicating that a C-style comment is opened
    static constexpr int cStyleCommentOpenedState = 1;
    //! \brief flag indicating that a raw-string comment is opened
    static constexpr int rawStringOpenedState = 2;
    //
    virtual void highlightBlock(const QString &) override;

   protected:
    //! \brief a simple wrapper around the highligthing rule
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
    QTextCharFormat stringFormat;
    QTextCharFormat preprocessorFormat;

    tfel::utilities::CxxTokenizerOptions options;
    //
    static const std::string& getRawStringDelimiter(const int);
    //
    static int getRawStringDelimiterId(const std::string &);
    //
    static std::vector<std::string> &getRawStringDelimiters();

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CSYNTAXHIGHLIGHTERBASE_HXX */
