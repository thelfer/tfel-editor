/*!
 * \file  MTestMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_QEMACS_MTESTMAJORMODE_HXX
#define LIB_QEMACS_MTESTMAJORMODE_HXX

#include "QEmacs/CxxMajorMode.hxx"

namespace qemacs {

  //! forward declaration
  struct QEmacsTextEditBase;

  /*!
   * major mode
   */
  struct MTestMajorMode : public CxxMajorMode {
    /*!
     * \brief constructor
     */
    MTestMajorMode(QEmacsWidget &,
                   QEmacsBuffer &,
                   QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void completeContextMenu(QMenu *const,
                             const QTextCursor &) override;

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

  void runMTest(QEmacsWidget &);
  void runPTest(QEmacsWidget &);

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MTESTMAJORMODE_H */
