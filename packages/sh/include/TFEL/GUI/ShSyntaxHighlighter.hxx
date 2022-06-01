/*!
 * \file   ShSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_SHSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_SHSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QTextDocument>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "TFEL/GUI/ShMajorMode.hxx"

namespace tfel::gui {

  /*!
   * \brief syntax highlighter class for `sh` files
   */
  struct TFELEDITOR_SHMODE_VISIBILITY_EXPORT ShSyntaxHighlighter
      : public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param t: text to be highlighted
     */
    ShSyntaxHighlighter(QTextDocument* const);
    //! \return all `sh` keywords
    virtual const QStringList& getKeywordsList();
    //! \return all `sh` build-in keywords
    virtual const QStringList& getBuildInKeywordsList();
    //! \return all `sh` functions
    virtual const QStringList& getFunctionsList();
    //! \return all `sh` commands
    virtual const QStringList& getCommandsList();
    /*!
     * \brief highlight the current line
     * \param[in] l: current line
     */
    void highlightBlock(const QString&) override;
    //! destructor
    ~ShSyntaxHighlighter() override;

   protected:
    //! formating a comment
    QTextCharFormat commentFormat;
    //! formating a keyword
    QTextCharFormat keyFormat;
    //! formating a build-in keyword
    QTextCharFormat bKeyFormat;
    //! formating a function
    QTextCharFormat functionFormat;
    //! formating a command
    QTextCharFormat commandFormat;
    //! formatting a string
    QTextCharFormat stringFormat;
    //! parsing options
    tfel::utilities::CxxTokenizerOptions options;

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SHSYNTAXHIGHLIGHTER_HXX */
