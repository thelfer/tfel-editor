/*!
 * \file   BashSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_BASHSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_BASHSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QTextDocument>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/ShSyntaxHighlighter.hxx"

namespace tfel {

  namespace gui {

    /*!
     * \brief syntax highlighter class for `bash` files
     */
    struct BashSyntaxHighlighter : public ShSyntaxHighlighter {
      /*!
       * \brief constructor
       * \param t: text to be highlighted
       */
      BashSyntaxHighlighter(QTextDocument* const);
      //! \return all `bash` keywords
      const QStringList& getKeywordsList() override;
      //! \return all `bash` build-in keywords
      const QStringList& getBuildInKeywordsList() override;
      //! \return all `bash` functions
      const QStringList& getFunctionsList() override;
      //! \return all `bash` commands
      const QStringList& getCommandsList() override;
      //! destructor
      ~BashSyntaxHighlighter() override;

     private:
      Q_OBJECT
    };  // end of struct CSyntaxHighlighter

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_BASHSYNTAXHIGHLIGHTER_HXX */
