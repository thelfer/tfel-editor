/*!
 * \file  CxxMajorMode.hxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_CXXMAJORMODE_HXX
#define LIB_TFEL_GUI_CXXMAJORMODE_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CMajorModeBase.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;

  /*!
   * \brief major mode dedicated to the C++ language
   *
   * \note the mode is registered by a static \c StandardMajorModeProxy defined
   * at the bottom of its source file, which matches the C and C++ file
   * extensions. It is the only mode derived from \c CMajorModeBase in the
   * editor, and \c clang-format is always called with the C++ dialect
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CxxMajorMode : public CMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     */
    CxxMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "C++", the name the mode is registered under
    QString getName() const override;

    //! \return "major mode dedicated to the C++ language"
    QString getDescription() const override;

    //! \return the C++ icon of the editor resources
    QIcon getIcon() const override;

    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     *
     * \c Alt-m runs a compilation command in the minibuffer.
     *
     * \return true if the mode handled the key, that is on \c Alt-m only; any
     * other key is left to \c CMajorModeBase, which handles none
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief install the highlighter of the C++ language on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is owned by the document from this point on; it is
     * released, and the previous one removed, by the base class
     */
    void setSyntaxHighlighter(QTextDocument *const) override;
    //! destructor
    ~CxxMajorMode() override;

   protected:
    /*!
     * \return the name of the mode, "C++", which is also the language name:
     * the compilation command history is stored under the address
     * "C++/compilation/history"
     */
    QString getLanguageName() const override;

   private:
    Q_OBJECT
  };  // end of struct CxxMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CXXMAJORMODE_HXX */
