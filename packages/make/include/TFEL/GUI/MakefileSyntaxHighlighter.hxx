/*!
 * \file   MakefileSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_MAKEFILESYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_MAKEFILESYNTAXHIGHLIGHTER_HXX

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QTextDocument>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief syntax highlighter class for Makefile files
   */
  struct MakefileSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param t: text to be highlighted
     */
    MakefileSyntaxHighlighter(QTextDocument *const);
    /*!
     * \brief highlight the current line
     * \param[in] l: current line
     */
    void highlightBlock(const QString &) override;
    //! destructor
    ~MakefileSyntaxHighlighter() override;

   protected:
    //! formating a comment
    QTextCharFormat commentFormat;
    //! formating a target
    QTextCharFormat targetFormat;
    //! parsing options
    tfel::utilities::CxxTokenizerOptions options;

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAKEFILESYNTAXHIGHLIGHTER_HXX */
