/*!
 * \file  CompiledLanguageMajorModeBase.hxx
 * \brief
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
    CompiledLanguageMajorModeBase(EditorWidget &, Buffer &, TextEditBase &);

    ~CompiledLanguageMajorModeBase() override;

   protected:
    virtual QString getLanguageName() const = 0;

    virtual QString getDefaultCompilationCommand() const;

    virtual void runCompilation();

    ProcessOutputFrame *po;

   private:
    Q_OBJECT
  };  // end of struct CompiledLanguageMajorModeBase

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_COMPILEDLANGUAGEMAJORMODEBASE_HXX */
