/*!
 * \file  MarkdownSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_HXX

#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"

namespace tfel::gui {

  // forward declaration
  struct MarkdownMajorMode;

  /*!
   * \brief a syntaxt highlighter for the Markdown language
   */
  struct MarkdownSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \param[in] m : calling mode
     * \param[in] t : text document to be highlighted
     */
    MarkdownSyntaxHighlighter(MarkdownMajorMode &, QTextDocument *const);
    /*!
     * \brief highlight current line
     * \param[in] l:current line
     */
    void highlightBlock(const QString &) override;
    //! destructor
    ~MarkdownSyntaxHighlighter() override;

   protected:
    //! calling object, to share the same spell checker
    MarkdownMajorMode &mode;
    //! formatting of level 1 headers
    QTextCharFormat h1;
    //! formatting of level 2 headers
    QTextCharFormat h2;
    //! formatting of level 3 headers
    QTextCharFormat h3;
    //! formatting of level 4 (and above) headers
    QTextCharFormat h4;
    //! metadata
    QTextCharFormat md;
    //! parsing options
    tfel::utilities::CxxTokenizerOptions options;

   private:
    Q_OBJECT

  };  // end of struct MarkdownSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_H */
