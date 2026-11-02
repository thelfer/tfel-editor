/*!
 * \file  CMajorModeBase.hxx
 * \brief base class for C-based languages, using clang-format for indentation.
 * \author Thomas Helfer
 * \date   05/09/2012
 */

#ifndef LIB_TFEL_GUI_CMAJORMODEBASE_HXX
#define LIB_TFEL_GUI_CMAJORMODEBASE_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CompiledLanguageMajorModeBase.hxx"

namespace tfel::gui {

  /*!
   * \brief base class for C-based language.
   * This mainly deals with indentation by relying on `clang-format`
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CMajorModeBase
      : public CompiledLanguageMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     */
    CMajorModeBase(EditorWidget&, Buffer&, TextEditBase&);
    /*!
     * \brief format the whole buffer
     *
     * The region handed over to \c indent is the whole document, so the
     * formatting is whatever the \c clang-format style file in the directory
     * of the file says, and not a reflow of the current paragraph
     */
    void format() override;
    /*!
     * \brief comment or uncomment a region
     *
     * The state of the region is determined by tokenizing the code which
     * precedes it with a \c CxxTokenizer, so that a text which is already
     * inside a comment is not commented again. \c // comments are used, one
     * per line; uncommenting restores the text the comments hold. With no
     * selection, the comment is appended at the end of the current line,
     * unless that line already ends with a comment.
     *
     * \note the region is indented once (un)commented
     *
     * \note the buffer is left untouched, and the cursor unmoved, when the
     * tokenizer fails on the code which precedes the cursor
     */
    void comment() override;

    /*!
     * \brief indent the line the given cursor stands on, by delegating to
     * \c clang-format
     * \param[in] c : text cursor
     *
     * \note when the cursor is on a blank line, the blank line is restored
     * with the indentation of the preceding non blank line, as \c indent does
     */
    void indentLine(const QTextCursor&) override;

    /*!
     * \brief indent selected region
     * \param[in] tc : text cursor defining the region to indent
     *
     * \note the region is left untouched when the cursor holds no selection
     *
     * \note the real position of the cursor, and not the end of the selection,
     * is used as the anchor \c clang-format reports back, so that the cursor
     * stays next to the character it was next to
     */
    void indentRegion(const QTextCursor&) override;
    //! destructor
    ~CMajorModeBase() override;

   protected:
    /*!
     * \brief indent a region, by writing the buffer in a temporary file and
     * handing it over to `clang-format`
     * \param[in] b: beginning of the region
     * \param[in] e: end of the region
     * \param[in] c: current position
     *
     * Only the lines of `[b, e]` are reformatted (`-lines`), using the style
     * file of the directory of the buffer (`-style=file`) and the C++ language,
     * whatever the extension of the file (`-assume-filename=cxx`). `c` is
     * passed as `-cursor` so that the position the tool reports back for that
     * anchor can be reused to place the editor's cursor next to the character
     * it was next to before the indentation.
     *
     * \note the whole document is replaced by the output of the tool, though
     * only the lines of the region may actually differ from it
     *
     * \note `c` is a Qt position in UTF-16 code units while `clang-format`
     * counts bytes of the UTF-8 encoded buffer: the two are converted
     *
     * \note when `b`, `e` and `c` are the same cursor standing on a blank line,
     * the tool has nothing to format: the last non blank line above the cursor
     * is formatted instead and a new blank line holding the indentation of that
     * line is inserted after it, so that the cursor stays on an indented blank
     * line
     *
     * \note on failure, the buffer is left untouched and the reason is
     * reported in the minibuffer: the temporary file can not be opened or
     * written, `clang-format` is missing or does not end, its output can not
     * be analysed
     *
     * \note the editor's cursor is moved to the position `clang-format`
     * reports, and the whole call is wrapped in a single edit block, so it is
     * undone in one step
     */
    virtual void indent(const QTextCursor&,
                        const QTextCursor&,
                        const QTextCursor&);

   private:
    Q_OBJECT
  };  // end of struct CMajorModeBase

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CMAJORMODEBASE_HXX */
