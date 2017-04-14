/*!
 * \file   PipeTestMajorMode.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   20 janv. 2016
 */

#ifndef _LIB_QEMACS_PIPETESTMAJORMODE_HXX_
#define _LIB_QEMACS_PIPETESTMAJORMODE_HXX_

#include"QEmacs/MTestMajorMode.hxx"

namespace qemacs
{
  
  /*!
   * major mode
   */
  class PipeTestMajorMode
    : public MTestMajorMode
  {

    Q_OBJECT

  public:

    PipeTestMajorMode(QEmacsWidget&,
		      QEmacsBuffer&,
		      QEmacsTextEditBase&);

    virtual QString
    getName(void) const override;

    virtual QString
    getDescription(void) const override;
    
    virtual void
    setSyntaxHighlighter(QTextDocument *const) override;
    virtual QCompleter*
    getCompleter(void) override;
    //! destructor
    virtual ~PipeTestMajorMode();
  protected:
    //! return the list of MTest keywords
    virtual QStringList
    getKeyWordsList() const override;
    //! return the scheme name
    virtual QString
    getScheme(void) const override;
    // completer
    QCompleter* c;
  }; // end of struct MTestMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_PIPETESTMAJORMODE_HXX_ */
