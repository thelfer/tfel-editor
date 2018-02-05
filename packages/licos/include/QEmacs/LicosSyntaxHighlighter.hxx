/*! 
 * \file  LicosSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_HXX 

#include<string>
#include<vector>
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

    static const std::vector<std::string>& getKeys();

    static const std::vector<std::string>& getBlocks();

    /*!
     * \param[in] t : text document to be highlighted
     */
    LicosSyntaxHighlighter(QTextDocument *const);

    void highlightBlock(const QString &) override;

  protected:

    static std::vector<std::string> buildKeysList();

    static std::vector<std::string> buildBlocksList();

    QVector<QRegExp> importExprs;

    QTextCharFormat optFormat;
    QTextCharFormat importFormat;
    
  }; // end of struct LicosSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_H */

