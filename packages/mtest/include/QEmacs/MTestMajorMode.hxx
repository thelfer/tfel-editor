/*! 
 * \file  MTestMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef LIB_QEMACS_MTESTMAJORMODE_HXX
#define LIB_QEMACS_MTESTMAJORMODE_HXX 

#include"QEmacs/CxxMajorMode.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
  
  /*!
   * major mode
   */
  class MTestMajorMode
    : public CxxMajorMode
  {

    Q_OBJECT

  public:

    MTestMajorMode(QEmacsWidget&,
		    QEmacsBuffer&,
		    QEmacsTextEditBase&);

    QString getName() const override;

    QString getDescription() const override;
    
    void completeContextMenu(QMenu *const,
			     const QTextCursor&) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QCompleter* getCompleter() override;

    QString getCompletionPrefix() override;

    QMenu* getSpecificMenu() override;

    bool keyPressEvent(QKeyEvent * const) override;
    
    ~MTestMajorMode() override;

  protected slots:
    //! \brief method called when the Help menu is called
    virtual void run();
    //! \brief launch the import behaviour wizard
    virtual void showImportBehaviourWizard();
    //! \brief method called when the Help menu is called
    virtual void start();
    //! \brief method called when the Help menu is called
    virtual void
    actionTriggered(QAction *);
    /*!
     * \param[in] a: action
     */
    virtual void
    insertKeyword(QAction *);
  protected:
    //! \brief return the list of MTest keywords
    virtual QStringList
    getKeyWordsList() const;
    //! \brief return the scheme name
    virtual QString getScheme() const;
    //! \brief completer
    QCompleter* c = nullptr;
    //! \brief help action
    QAction *ha = nullptr;
    //! \brief import behaviour action
    QAction *iba = nullptr;
    //! \brief run mtest
    QAction *ra = nullptr;
  }; // end of struct MTestMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_MTESTMAJORMODE_H */

