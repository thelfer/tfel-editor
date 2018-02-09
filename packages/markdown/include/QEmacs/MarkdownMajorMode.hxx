/*! 
 * \file  MarkdownMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef LIB_QEMACS_MFRONTMAJORMODE_HXX
#define LIB_QEMACS_MFRONTMAJORMODE_HXX 

#include<QEmacs/SpellChecker.hxx>
#include"QEmacs/CompiledLanguageMajorModeBase.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
  
  /*!
   * major mode
   */
  class MarkdownMajorMode
    : public CompiledLanguageMajorModeBase
  {

    Q_OBJECT

  public:
    MarkdownMajorMode(QEmacsWidget &, QEmacsBuffer &, QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QMenu* getSpecificMenu() override;

    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    QCompleter* getCompleter() override;

    QString getCompletionPrefix() override;

    QString getLanguageName() const override;

    QString getDefaultCompilationCommand() const override;

    void format() override;

    QIcon getIcon() const override;

    SpellChecker& getSpellChecker();

    //! destructor    
    ~MarkdownMajorMode() override;

  protected slots:
    //! method called when the Help menu is called
    virtual void actionTriggered(QAction *);
    //! slot called to prepare a Markdown analysis
    virtual void runPandoc();
    //! slot called to start a new Markdown analysis
    virtual void startPandoc();
  protected:
    //! action associated with the runMarkdown method
    QAction *rm = nullptr;
    //! checking
    SpellChecker spellChecker;
  }; // end of struct MarkdownMajorMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTMAJORMODE_H */

