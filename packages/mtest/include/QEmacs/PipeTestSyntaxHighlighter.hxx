/*!
 * \file  PipeTestSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QStringList>

#include "QEmacs/CxxSyntaxHighlighter.hxx"

namespace qemacs {

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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_PIPETESTSYNTAXHIGHLIGHTER_H */
