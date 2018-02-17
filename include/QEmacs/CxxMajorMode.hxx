/*! 
 * \file  CxxMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_QEMACS_CXXMAJORMODE_HXX
#define LIB_QEMACS_CXXMAJORMODE_HXX 

#include"QEmacs/Config.hxx"
#include"QEmacs/CMajorModeBase.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsTextEditBase;

  //! major mode dedicated to the C++ language
  struct QEMACS_VISIBILITY_EXPORT CxxMajorMode : public CMajorModeBase {
    CxxMajorMode(QEmacsWidget &, QEmacsBuffer &, QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    QIcon getIcon() const override;
    
    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default shortcuts
     */
    bool keyPressEvent(QKeyEvent * const) override;

    void setSyntaxHighlighter(QTextDocument *const) override;
    //! destructor
    ~CxxMajorMode() override;

   protected:

    QString getLanguageName() const override;

   private:
    Q_OBJECT
  }; // end of struct CxxMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_CXXMAJORMODE_HXX */

