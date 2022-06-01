/*!
 * \file  MTestMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_MTESTMAJORMODE_HXX
#define LIB_TFEL_GUI_MTESTMAJORMODE_HXX

#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel::gui {

  //! forward declaration
  struct TextEditBase;

  /*!
   * major mode
   */
  struct MTestMajorMode : public CxxMajorMode {
    /*!
     * \brief constructor
     */
    MTestMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QCompleter *getCompleter() override;

    QString getCompletionPrefix() override;

    QMenu *getSpecificMenu() override;

    bool keyPressEvent(QKeyEvent *const) override;
    //! destructor
    ~MTestMajorMode() override;

   protected slots:
    //! \brief launch the import behaviour wizard
    virtual void showImportBehaviourWizard();
    //! \brief launch the import MFM behaviour dialog
    virtual void showImportMFMBehaviourWizard();
    //! \brief method called when the `Show Results with TPlot` menu is
    //! called
    virtual void showResults();
    /*!
     * \param[in] a: action
     */
    virtual void insertKeyword(QAction *);

   protected:
    //! \brief return the list of MTest keywords
    virtual const std::vector<std::string> &getKeyWordsList() const;
    //! \brief return the scheme name
    virtual QString getScheme() const;
    //! \brief completer
    QCompleter *c = nullptr;

   private:
    Q_OBJECT

  };  // end of struct MTestMajorMode

  void runMTest(EditorWidget &);
  void runPTest(EditorWidget &);

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTESTMAJORMODE_H */
