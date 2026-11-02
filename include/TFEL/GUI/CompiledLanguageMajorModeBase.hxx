/*!
 * \file  CompiledLanguageMajorModeBase.hxx
 * \brief base class for all compiled languages.
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_COMPILEDLANGUAGEMAJORMODEBASE_HXX
#define LIB_TFEL_GUI_COMPILEDLANGUAGEMAJORMODEBASE_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  // forward declaration
  struct ProcessOutputFrame;

  //! \brief base class for all compiled language
  struct TFEL_GUI_VISIBILITY_EXPORT CompiledLanguageMajorModeBase
      : public MajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     */
    CompiledLanguageMajorModeBase(EditorWidget &, Buffer &, TextEditBase &);

    ~CompiledLanguageMajorModeBase() override;

   protected:
    /*!
     * \return the name of the language of the mode, used as the address under
     * which its compilation command history is stored in the settings
     *
     * \note this is what \c runCompilation uses to build the key
     * `<language>/compilation/history`
     */
    virtual QString getLanguageName() const = 0;

    /*!
     * \return the compilation command proposed to the user when no
     * compilation command has been recorded in the settings yet
     *
     * \note the default implementation returns "make"
     */
    virtual QString getDefaultCompilationCommand() const;

    /*!
     * \brief ask the user for a compilation command in the minibuffer and run
     * it in a shell once it is typed
     *
     * The command proposed is the last one of the history of the language, or
     * the default compilation command if that history is empty. The output of
     * the command is displayed in a secondary task of the current buffer,
     * titled after the command itself and displayed with the
     * "compilation-output" major mode.
     *
     * \note the minibuffer input is owned by the editor once
     * \c EditorWidget::setUserInput has been given it
     */
    virtual void runCompilation();

    /*!
     * \brief frame the output of the compilation process is displayed in
     *
     * \note neither this class nor the modes derived from it use this member:
     * \c runCompilation lets the minibuffer input create the output frame it
     * needs. The constructor does not initialise it
     */
    ProcessOutputFrame *po;

   private:
    Q_OBJECT
  };  // end of struct CompiledLanguageMajorModeBase

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_COMPILEDLANGUAGEMAJORMODEBASE_HXX */
