/*!
 * \file   PipeTestMajorMode.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   20/01/2016
 */

#ifndef LIB_QEMACS_PIPETESTMAJORMODE_HXX
#define LIB_QEMACS_PIPETESTMAJORMODE_HXX

#include"QEmacs/MTestMajorMode.hxx"

namespace qemacs {

  //! \brief major mode for editing MTest' input files (pipe scheme)
  struct PipeTestMajorMode : public MTestMajorMode {
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
    const std::vector<std::string>& getKeyWordsList() const override;
    //! return the scheme name
    QString getScheme() const override;
    // completer
    QCompleter* c;

   private:
    Q_OBJECT
  }; // end of struct MTestMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_PIPETESTMAJORMODE_HXX_ */
