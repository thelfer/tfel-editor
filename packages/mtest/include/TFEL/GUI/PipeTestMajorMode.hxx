/*!
 * \file   PipeTestMajorMode.hxx
 * \brief PipeTestMajorMode: major mode for the pipe scheme of mtest.
 * \author THOMAS HELFER
 * \date   20/01/2016
 */

#ifndef LIB_TFEL_GUI_PIPETESTMAJORMODE_HXX
#define LIB_TFEL_GUI_PIPETESTMAJORMODE_HXX

#include "TFEL/GUI/MTestMajorMode.hxx"

namespace tfel::gui {

  //! \brief major mode for editing MTest' input files (pipe scheme)
  struct PipeTestMajorMode : public MTestMajorMode {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     *
     * \note a completer built on the keywords of the pipe scheme replaces the
     * member \c c; the completer of the base class, which is a child of the
     * text edit \c t and not of the mode, stays alive until that text edit is
     * destroyed, although nothing refers to it any more
     */
    PipeTestMajorMode(EditorWidget&, Buffer&, TextEditBase&);

    //! \return "PipeTest", the name the mode is registered under
    QString getName() const override;

    //! \return "major mode dedicated to the mtest extension to pipe"
    QString getDescription() const override;

    /*!
     * \brief install the highlighter of the pipe scheme on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is installed through \c setHighlighter, so the
     * previous one is removed from the document and released
     */
    void setSyntaxHighlighter(QTextDocument* const) override;
    /*!
     * \return the completer of the keywords of the pipe scheme, built by the
     * constructor of this mode
     */
    QCompleter* getCompleter() override;
    //! destructor
    ~PipeTestMajorMode() override;

   protected:
    /*!
     * \return the keywords of the pipe scheme, as the parser of that scheme
     * gives them; a static list shared with the syntax highlighter of this mode
     */
    const std::vector<std::string>& getKeyWordsList() const override;
    //! \return "ptest", the scheme the studies of this mode run with
    QString getScheme() const override;

   private:
    Q_OBJECT
  };  // end of struct MTestMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PIPETESTMAJORMODE_HXX_ */
