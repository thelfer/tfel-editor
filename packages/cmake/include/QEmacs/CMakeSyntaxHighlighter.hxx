/*!
 * \file   CMakeSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_QEMACS_CMAKESYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_CMAKESYNTAXHIGHLIGHTER_HXX

#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtGui/QTextDocument>
#include <QtGui/QSyntaxHighlighter>

#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "QEmacs/Config.hxx"

namespace qemacs {

  /*!
   * \brief syntax highlighter class for `CMakeLists.txt` files
   */
  struct CMakeSyntaxHighlighter : public QSyntaxHighlighter {
    //! \return all cmake standard commands
    static const QStringList& getCMakeCommandsList();
    /*!
     * \brief constructor
     * \param t: text to be highlighted
     */
    CMakeSyntaxHighlighter(QTextDocument* const);
    /*!
     * \brief highlight the current line
     * \param[in] l: current line
     */
    void highlightBlock(const QString&) override;
    //! destructor
    ~CMakeSyntaxHighlighter() override;

   protected:
    //! formating a comment
    QTextCharFormat commentFormat;
    //! formating a standard command
    QTextCharFormat commandFormat1;
    //! formating a (possibly) user defined macro or function
    QTextCharFormat commandFormat2;
    //! formatting a string
    QTextCharFormat stringFormat;
    //! parsing options
    tfel::utilities::CxxTokenizerOptions options;

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace qemacs

#endif /* LIB_QEMACS_CMAKESYNTAXHIGHLIGHTER_HXX */
