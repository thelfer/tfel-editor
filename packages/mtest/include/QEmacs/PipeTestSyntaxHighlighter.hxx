/*! 
 * \file  PipeTestSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_HXX 

#include<QtCore/QStringList>

#include"QEmacs/CxxSyntaxHighlighter.hxx"

namespace qemacs
{

  /*!
   * a syntaxt highlighter for the PipeTest code generator
   */
  class PipeTestSyntaxHighlighter
    : public CxxSyntaxHighlighter
  {
    Q_OBJECT
  public:
    //! \return the list of keys
    static const std::vector<std::string>& getPipeTestKeys();
    /*!
     * \param[in] t : text document to be highlighted
     */
    PipeTestSyntaxHighlighter(QTextDocument *const);
    //! \brief destructor
    virtual ~PipeTestSyntaxHighlighter();
  protected:
    //! \return the list of keys
    virtual const std::vector<std::string>& getKeyWordsList() const;
    // format used for syntax highlighting
    QTextCharFormat mtestKeyFormat;
  }; // end of struct PipeTestSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_H */

