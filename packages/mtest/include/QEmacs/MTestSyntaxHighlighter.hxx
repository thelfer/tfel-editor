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

namespace qemacs {

  //! \brief a syntax highlighter for MTest' input files
  struct MTestSyntaxHighlighter : public CxxSyntaxHighlighter {
    //! \return the list of keys
    static const std::vector<std::string>& getMTestKeys();
    /*!
     * \param[in] t : text document to be highlighted
     */
    MTestSyntaxHighlighter(QTextDocument *const);
    //! \brief destructor
    ~MTestSyntaxHighlighter() override;

   protected:
    //! \return the list of keys
    virtual const std::vector<std::string>& getKeyWordsList() const;
    // format used for syntax highlighting
    QTextCharFormat mtestKeyFormat;

   private:
    Q_OBJECT
  }; // end of struct MTestSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_MTESTSYNTAXHIGHLIGHTER_H */

