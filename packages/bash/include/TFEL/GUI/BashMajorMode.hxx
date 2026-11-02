/*!
 * \file  BashMajorMode.hxx
 * \brief BashMajorMode: a major mode for bash files, derived from ShMajorMode.
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_BASHMAJORMODE_HXX
#define LIB_TFEL_GUI_BASHMAJORMODE_HXX

#include "TFEL/GUI/ShMajorMode.hxx"

namespace tfel::gui {

  /*!
   * \brief a major mode for editing `bash` files
   *
   * The mode only differs from \c ShMajorMode by its name, its description
   * and the highlighter it installs; it inherits from it the `#` comment
   * syntax and the empty \c format method.
   *
   * \note the mode is registered by a \c StandardMajorModeProxy static
   * initialiser at the end of `BashMajorMode.cxx`, which associates it with
   * the files whose name ends with `.bash`.
   */
  struct BashMajorMode : public ShMajorMode {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor widget the mode acts on
     */
    BashMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return the name of the mode, "bash"
    QString getName() const override;

    //! \return a fixed description of the mode
    QString getDescription() const override;

    /*!
     * \brief install the syntax highlighter of the mode on a document
     * \param[in] d : document to be highlighted
     *
     * \note a \c BashSyntaxHighlighter is installed through
     * \c setHighlighter, which replaces the previous highlighter; the
     * document owns the highlighter it is built with
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    //! destructor
    ~BashMajorMode() override;

   private:
    Q_OBJECT
  };  // end of struct BashMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_BASHMAJORMODE_HXX */
