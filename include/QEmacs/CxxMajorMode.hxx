/*! 
 * \file  CxxMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef LIB_QEMACS_CXXMAJORMODE_HXX
#define LIB_QEMACS_CXXMAJORMODE_HXX 

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

    QString getName() const override;

    QString getDescription() const override;
    
    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default shortcuts
     */
    bool keyPressEvent(QKeyEvent * const) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    ~CxxMajorMode() override;

  protected:

    QString getLanguageName() const override;

  }; // end of struct CxxMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_CXXMAJORMODE_HXX */

