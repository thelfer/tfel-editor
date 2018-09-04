/*! 
 * \file  MTestSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MTESTSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_MTESTSYNTAXHIGHLIGHTER_HXX 

#include<QtCore/QStringList>

#include"TFEL/GUI/CxxSyntaxHighlighter.hxx"

namespace tfel{

  namespace gui{

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
  
} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MTESTSYNTAXHIGHLIGHTER_H */

