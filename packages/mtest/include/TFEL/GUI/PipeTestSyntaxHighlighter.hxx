/*!
 * \file  PipeTestSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_PIPETESTSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_PIPETESTSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QStringList>

#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"

namespace tfel::gui {

  //! \brief a syntax highlighter for MTest' input files (pipe scheme)
  struct PipeTestSyntaxHighlighter : public CxxSyntaxHighlighter {
    //! \return the list of keys
    static const std::vector<std::string>& getPipeTestKeys();
    /*!
     * \param[in] t : text document to be highlighted
     */
    PipeTestSyntaxHighlighter(QTextDocument* const);
    //! \brief destructor
    ~PipeTestSyntaxHighlighter() override;

   protected:
    //! \return the list of keys
    virtual const std::vector<std::string>& getKeyWordsList() const;
    // format used for syntax highlighting
    QTextCharFormat mtestKeyFormat;

   private:
    Q_OBJECT
  };  // end of struct PipeTestSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PIPETESTSYNTAXHIGHLIGHTER_H */
