/*!
 * \file  MarkdownSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_HXX

#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"

namespace qemacs {

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
     /*!
      * \brief highlight the current word, if mispelled
      * \param[in] w: current word
      * \param[in] p: position of the beginning of the word
      */
     void highLightMispellWords(const QString &, const int);
     //! calling object, to share the same spell checker
     MarkdownMajorMode& mode;
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTSYNTAXHIGHLIGHTER_H */
