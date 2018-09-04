/*!
 * \file   PipeTestMajorMode.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   20/01/2016
 */

#ifndef LIB_TFEL_GUI_PIPETESTMAJORMODE_HXX
#define LIB_TFEL_GUI_PIPETESTMAJORMODE_HXX

#include"TFEL/GUI/MTestMajorMode.hxx"

namespace tfel{

  namespace gui{

  //! \brief major mode for editing MTest' input files (pipe scheme)
  struct PipeTestMajorMode : public MTestMajorMode {
    PipeTestMajorMode(EditorWidget&,
                      Buffer&,
                      TextEditBase&);

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

   private:
    Q_OBJECT
  }; // end of struct MTestMajorMode

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_PIPETESTMAJORMODE_HXX_ */
