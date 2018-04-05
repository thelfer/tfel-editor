/*! 
 * \file  CompiledLanguageMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#ifndef LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_HXX
#define LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_HXX 

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs {

  // forward declaration
  struct ProcessOutputFrame;

  /*!
   * base class for all compiled language
   */
  struct QEMACS_VISIBILITY_EXPORT CompiledLanguageMajorModeBase
      : public QEmacsMajorModeBase {
    CompiledLanguageMajorModeBase(QEmacsWidget &,
                                  QEmacsBuffer &,
                                  QEmacsTextEditBase &);

    ~CompiledLanguageMajorModeBase() override;

  protected:

    virtual QString getLanguageName() const = 0;
    
    virtual QString getDefaultCompilationCommand() const;

    virtual void runCompilation();

    ProcessOutputFrame *po;

   private:
    Q_OBJECT

  }; // end of struct CompiledLanguageMajorModeBase

} // end of namespace qemacs

#endif /* LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_HXX */

