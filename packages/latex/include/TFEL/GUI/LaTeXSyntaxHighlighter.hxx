/*!
 * \file  LaTeXSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_HXX

#include <QtGui/QSyntaxHighlighter>

namespace tfel::gui {

  // forward declaration
  struct LaTeXMajorMode;

  /*!
   * a syntaxt highlighter for LaTeX
   */
  struct LaTeXSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * a simple wrapper around the highligthing rule
     */
    struct HighlightingRule {
      QRegExp pattern;
      QVector<QTextCharFormat> format;
    };  // end of struct HighlightingRule

    static QString stripComment(const QString&);

    static int startOfComment(const QString&);

    //! list of highlighting rules
    const static QVector<HighlightingRule>& getHighlightingRules();

    /*!
     * \param[in] m : latex mode
     * \param[in] t : text document to be highlighted
     */
    LaTeXSyntaxHighlighter(LaTeXMajorMode&, QTextDocument* const);

    virtual void highlightBlock(const QString&) override;

   protected:
    void highLightMispellWords(const QString&, const int);

    //! build the highlighting rules
    static QVector<HighlightingRule> buildHighlightingRules();

    LaTeXMajorMode& mode;

    QTextCharFormat commentFormat;

   private:
    Q_OBJECT

  };  // end of struct LaTeXSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_H */
