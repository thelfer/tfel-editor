/*! 
 * \file  CompiledLanguageMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 août 2012
 */

#ifndef LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_HXX
#define LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_HXX 

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs
{

  class ProcessOutputFrame;

  /*!
   * base class for all compiled language
   */
  class QEMACS_VISIBILITY_EXPORT CompiledLanguageMajorModeBase
    : public QEmacsMajorModeBase
  {

    Q_OBJECT

  public:

    CompiledLanguageMajorModeBase(QEmacsWidget&,
				  QEmacsBuffer&,
				  QEmacsTextEditBase&);

    ~CompiledLanguageMajorModeBase() override;

  protected:

    virtual QString getLanguageName() const = 0;
    
    virtual QString getDefaultCompilationCommand() const;
    
    void runCompilation();

    ProcessOutputFrame *po;
    
  }; // end of struct CompiledLanguageMajorModeBase

} // end of namespace qemacs

#endif /* LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_HXX */

