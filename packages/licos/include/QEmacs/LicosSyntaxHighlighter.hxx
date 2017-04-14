/*! 
 * \file  LicosSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_H_
#define _LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_H_ 

#include"QEmacs/CSyntaxHighlighterBase.hxx"

namespace qemacs
{

  struct LicosData
    : public QTextBlockUserData
  {
    virtual ~LicosData();
    QString library;
    QString function;
    int pos;
  }; // end of   struct LicosData

  /*!
   * a syntaxt highlighter for C-based languages
   */
  class LicosSyntaxHighlighter
    : public CSyntaxHighlighterBase
  {

    Q_OBJECT

  public:

    static const QStringList&
    getKeys();

    static const QStringList&
    getBlocks();

    /*!
     * \param[in] t : text document to be highlighted
     */
    LicosSyntaxHighlighter(QTextDocument *const);

    virtual void
    highlightBlock(const QString &);

  protected:

    static QStringList
    buildKeysList(void);

    static QStringList
    buildBlocksList(void);

    QVector<QRegExp> importExprs;

    QTextCharFormat optFormat;
    QTextCharFormat importFormat;
    
  }; // end of struct LicosSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* _LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_H */

