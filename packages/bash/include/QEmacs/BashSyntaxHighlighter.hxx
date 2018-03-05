/*!
 * \file   BashSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_QEMACS_BASHSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_BASHSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QTextDocument>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "QEmacs/Config.hxx"
#include "QEmacs/ShSyntaxHighlighter.hxx"

namespace qemacs {

  /*!
   * \brief syntax highlighter class for `bash` files
   */
  struct BashSyntaxHighlighter : public ShSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param t: text to be highlighted
     */
    BashSyntaxHighlighter(QTextDocument* const);
    //! \return all `bash` keywords
    const QStringList& getKeywordsList() override;
    //! \return all `bash` build-in keywords
    const QStringList& getBuildInKeywordsList() override;
    //! \return all `bash` functions
    const QStringList& getFunctionsList() override;
    //! \return all `bash` commands
    const QStringList& getCommandsList() override;
    //! destructor
    ~BashSyntaxHighlighter() override;

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace qemacs

#endif /* LIB_QEMACS_BASHSYNTAXHIGHLIGHTER_HXX */
