/*!
 * \file  LicosSyntaxHighlighter.hxx
 * \brief syntax highlighter for licos input files and the data of blocks.
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_LICOSSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_LICOSSYNTAXHIGHLIGHTER_HXX

#include <string>
#include <vector>
#include <QtCore/QRegularExpression>
#include "TFEL/GUI/CSyntaxHighlighterBase.hxx"

namespace tfel::gui {

  /*!
   * \brief the licos information a block of the document carries: the library
   * and the function a material property or a mechanical behaviour refers to
   *
   * \note the highlighter attaches it to the block it has recognised; the text
   * block owns the object and destroys it with the block
   *
   * \note only the code which opens the source of that function from the
   * context menu reads it, and that code is commented out
   */
  struct LicosData : public QTextBlockUserData {
    //! destructor
    virtual ~LicosData();
    //! name of the library, that is the second string of the line
    QString library;
    //! name of the function the library provides
    QString function;
    /*!
     * \brief position of the first character of \c function in its own block,
     * that is the position of the character which follows the opening quote
     *
     * \note the commented-out code of the context menu compares it to the
     * position of the cursor to know whether that cursor stands on the function
     * name
     */
    int pos;
  };  // end of   struct LicosData

  //! \brief a syntaxt highlighter for Licos input files
  struct LicosSyntaxHighlighter : public CSyntaxHighlighterBase {
    /*!
     * \return the keywords of the licos language: the keys a block may hold,
     * such as `Mesh` or `MaterialProperty`, and the types of its parameters,
     * such as `Array` or `Double`
     *
     * \note the list is built once, on the first call; the major mode completes
     * with it
     */
    static const std::vector<std::string>& getKeys();

    /*!
     * \return the names of the blocks of the licos language, such as `Study` or
     * `Material`
     *
     * \note the list is built once, on the first call; the major mode completes
     * with it and the `EndOf` counterpart of each name is highlighted too
     */
    static const std::vector<std::string>& getBlocks();

    /*!
     * \param[in] t : text document to be highlighted
     *
     * \note the highlighting rules of the keywords and of the blocks are built
     * here, as well as the regular expressions which detect the file names of
     * the licos extensions
     *
     * \note the highlighter is owned by the document it is built with
     */
    LicosSyntaxHighlighter(QTextDocument* const);

    /*!
     * \brief highlight a single line of the document
     * \param[in] text : contents of the line
     *
     * \note the line is tokenised as C code: only the first word can be a
     * keyword, the words in angle brackets which follow it are the options of
     * the block, the strings are strings and the comments are comments
     *
     * \note the quoted names of the files of the licos extensions are
     * underlined, as they may be opened
     *
     * \note the block state is set to 1 when a C style comment is left opened
     * at the end of the line and to 0 otherwise
     *
     * \note a `MaterialProperty` line whose option is `castem` and a
     * `MechanicalBehaviour` line followed by three quoted names attach a
     * \c LicosData to the block: it holds the last two of those names, the
     * library and the function
     */
    void highlightBlock(const QString&) override;

   protected:
    /*!
     * \return the keywords \c getKeys returns
     */
    static std::vector<std::string> buildKeysList();

    /*!
     * \return the block names \c getBlocks returns
     */
    static std::vector<std::string> buildBlocksList();

    /*!
     * regular expressions which match the quoted name of a file of one of the
     * licos extensions, the name itself being the captured expression
     */
    QVector<QRegularExpression> importExprs;

    //! format of the options of a block, that is of the words in angle brackets
    QTextCharFormat optFormat;
    /*!
     * format of a file name which may be opened in a new buffer: the format of
     * a string, underlined, with a tool tip
     */
    QTextCharFormat importFormat;

   private:
    Q_OBJECT

  };  // end of struct LicosSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSSYNTAXHIGHLIGHTER_H */
