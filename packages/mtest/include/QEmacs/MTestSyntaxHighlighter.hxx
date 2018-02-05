/*! 
 * \file  MTestSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_MTESTSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_MTESTSYNTAXHIGHLIGHTER_HXX 

#include<QtCore/QStringList>

#include"QEmacs/CxxSyntaxHighlighter.hxx"

namespace qemacs
{

  /*!
   * a syntaxt highlighter for the MTest code generator
   */
  class MTestSyntaxHighlighter
    : public CxxSyntaxHighlighter
  {
    Q_OBJECT
  public:
    //! \return the list of keys
    static const std::vector<std::string>& getMTestKeys();
    /*!
     * \param[in] t : text document to be highlighted
     */
    MTestSyntaxHighlighter(QTextDocument *const);
    //! \brief destructor
    virtual ~MTestSyntaxHighlighter();
  protected:
    //! \return the list of keys
    virtual const std::vector<std::string>& getKeyWordsList() const;
    // format used for syntax highlighting
    QTextCharFormat mtestKeyFormat;
  }; // end of struct MTestSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_MTESTSYNTAXHIGHLIGHTER_H */

