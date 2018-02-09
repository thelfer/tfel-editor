/*! 
 * \file  MFrontMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef LIB_QEMACS_MFRONTMAJORMODE_HXX
#define LIB_QEMACS_MFRONTMAJORMODE_HXX 

#include<QtCore/QTimer>
#include"QEmacs/CxxMajorMode.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
  
  /*!
   * major mode
   */
  class MFrontMajorMode
    : public CxxMajorMode
  {

    Q_OBJECT

  public:
    MFrontMajorMode(QEmacsWidget &, QEmacsBuffer &, QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QMenu* getSpecificMenu() override;

    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    QCompleter* getCompleter() override;

    QString getCompletionPrefix() override;

    QString getLanguageName() const override;

    QString getDefaultCompilationCommand() const override;

    QIcon getIcon() const override;
    //! destructor    
    ~MFrontMajorMode() override;

  protected slots:
    //! method called periodically to refresh the syntax highlighter
    //! and the completer
    virtual void updateSyntaxHighlighterAndCompleter();
    //! method called when the Help menu is called
    virtual void actionTriggered(QAction *);
    //! slot called to prepare a MFront analysis
    virtual void runMFront();
    //! slot called to start a new MFront analysis
    virtual void startMFront();
  protected:
    //! \return the name of the DSL
    virtual QString getDSLName();
    //! return the list of MTest keywords
    virtual QStringList getKeyWordsList();
    /*!
     * \brief a timer to refresh syntax highlighting
     * This is mandatory since the @DSL can change with the user inputs
     */
    QTimer *rt = nullptr;
    //! action associated with the runMFront method
    QAction *rm = nullptr;
    //! completer
    QCompleter* c = nullptr;
    //! help action
    QAction *ha = nullptr;
  }; // end of struct MFrontMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTMAJORMODE_H */

