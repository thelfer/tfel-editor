/*! 
 * \file  LaTeXSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_LATEXSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_LATEXSYNTAXHIGHLIGHTER_HXX 

#include<QtGui/QSyntaxHighlighter>

namespace qemacs
{

  class LaTeXMajorMode;

  /*!
   * a syntaxt highlighter for LaTeX
   */
  class LaTeXSyntaxHighlighter
    : public QSyntaxHighlighter
  {

    Q_OBJECT

  public:

    /*!
     * a simple wrapper around the highligthing rule
     */
    struct HighlightingRule
    {
      QRegExp                  pattern;
      QVector<QTextCharFormat> format;
    }; // end of struct HighlightingRule

    static QString
    stripComment(const QString&);

    static int
    startOfComment(const QString&);

    //! list of highlighting rules
    const static QVector<HighlightingRule>&
    getHighlightingRules();

    /*!
     * \param[in] m : latex mode
     * \param[in] t : text document to be highlighted
     */
    LaTeXSyntaxHighlighter(LaTeXMajorMode&,
			   QTextDocument *const);

    virtual void
    highlightBlock(const QString &) override;

  protected:

    void
    highLightMispellWords(const QString&,
			  const int);


    //! build the highlighting rules
    static QVector<HighlightingRule>
    buildHighlightingRules();

    LaTeXMajorMode& mode;

    QTextCharFormat commentFormat;
    
  }; // end of struct LaTeXSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_LATEXSYNTAXHIGHLIGHTER_H */

