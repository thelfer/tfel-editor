/*!
 * \file   PipeTestMajorMode.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   20 janv. 2016
 */

#ifndef LIB_QEMACS_PIPETESTMAJORMODE_HXX_
#define LIB_QEMACS_PIPETESTMAJORMODE_HXX_

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

    QString getName() const override;

    QString getDescription() const override;
    
    void setSyntaxHighlighter(QTextDocument *const) override;
    QCompleter* getCompleter() override;
    //! destructor
    ~PipeTestMajorMode() override;
  protected:
    //! return the list of MTest keywords
    QStringList getKeyWordsList() const override;
    //! return the scheme name
    QString getScheme() const override;
    // completer
    QCompleter* c;
  }; // end of struct MTestMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_PIPETESTMAJORMODE_HXX_ */
