/*! 
 * \file  CxxMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef _LIB_QEMACS_CXXMAJORMODE_H_
#define _LIB_QEMACS_CXXMAJORMODE_H_ 

#include"QEmacs/Config.hxx"
#include"QEmacs/CMajorModeBase.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
  
  /*!
   * major mode
   */
  class QEMACS_VISIBILITY_EXPORT CxxMajorMode
    : public CMajorModeBase
  {

    Q_OBJECT

  public:

    CxxMajorMode(QEmacsWidget&,
		 QEmacsBuffer&,
		 QEmacsTextEditBase&);

    virtual QString
    getName(void) const override;

    virtual QString
    getDescription(void) const override;
    
    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default shortcuts
     */
    virtual bool
    keyPressEvent(QKeyEvent * const) override;

    virtual void
    setSyntaxHighlighter(QTextDocument *const) override;

    virtual ~CxxMajorMode();

  protected:

    virtual QString
    getLanguageName(void) const override;

  }; // end of struct CxxMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_CXXMAJORMODE_H */

