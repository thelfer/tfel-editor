/*! 
 * \file  LicosSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
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

  //! \brief a syntaxt highlighter for Licos input files
  struct LicosSyntaxHighlighter : public CSyntaxHighlighterBase {

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

   private:
    Q_OBJECT

  }; // end of struct LicosSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSSYNTAXHIGHLIGHTER_H */

