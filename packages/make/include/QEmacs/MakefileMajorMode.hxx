/*! 
 * \file  MakefileMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 août 2012
 */

#ifndef _LIB_QEMACS_MAKEFILEMAJORMODE_H_
#define _LIB_QEMACS_MAKEFILEMAJORMODE_H_ 

#include"QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs
{
  
  class MakefileMajorMode
    : public QEmacsMajorModeBase
  {
    
    Q_OBJECT

  public:

    MakefileMajorMode(QEmacsWidget&,
		      QEmacsBuffer&,
		      QEmacsTextEditBase&);

    virtual QString
    getName(void) const override;

    virtual QString
    getDescription(void) const override;
    
    /*!
     * This method is called before any treatment by
     * QEmacsTextEditBase and allows the mode to override default
     * shortcuts
     */
    virtual bool
    keyPressEvent(QKeyEvent * const) override;

    virtual void
    setSyntaxHighlighter(QTextDocument *const) override;

    virtual void
    format(void) override;

    virtual QString
    getCommentSyntax(void) override;

    virtual ~MakefileMajorMode();

  }; // end of struct MakefileMajorMode
  
} // end of namespace qemacs

#endif /* _LIB_QEMACS_MAKEFILEMAJORMODE_H */

