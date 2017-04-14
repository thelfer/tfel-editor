/*! 
 * \file  CompiledLanguageMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 août 2012
 */

#ifndef _LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_H_
#define _LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_H_ 

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

    virtual ~CompiledLanguageMajorModeBase();

  protected:

    virtual QString
    getLanguageName(void) const = 0;
    
    virtual QString
    getDefaultCompilationCommand(void) const;
    
    void runCompilation();

    ProcessOutputFrame *po;
    
  }; // end of struct CompiledLanguageMajorModeBase

} // end of namespace qemacs

#endif /* _LIB_QEMACS_COMPILEDLANGUAGEMAJORMODEBASE_H */

