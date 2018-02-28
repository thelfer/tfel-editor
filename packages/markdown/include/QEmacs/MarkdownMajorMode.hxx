/*!
 * \file  MarkdownMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#ifndef LIB_QEMACS_MFRONTMAJORMODE_HXX
#define LIB_QEMACS_MFRONTMAJORMODE_HXX

#include <QtGui/QSyntaxHighlighter>
#include <QEmacs/SpellChecker.hxx>
#include "QEmacs/CompiledLanguageMajorModeBase.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsTextEditBase;

  //! \brief major mode for editing Markdown documents
  struct MarkdownMajorMode : public CompiledLanguageMajorModeBase {
    MarkdownMajorMode(QEmacsWidget &,
                      QEmacsBuffer &,
                      QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSpellCheckLanguage(const QString &) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QMenu *getSpecificMenu() override;

    void completeContextMenu(QMenu *const,
                             const QTextCursor &) override;

    QCompleter *getCompleter() override;

    QString getCompletionPrefix() override;

    QString getLanguageName() const override;

    QString getDefaultCompilationCommand() const override;

    void format() override;

    bool keyPressEvent(QKeyEvent *const) override;

    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;

    QIcon getIcon() const override;

    SpellChecker &getSpellChecker();

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
    QSyntaxHighlighter *highlighter = nullptr;

   private:
    Q_OBJECT
  };  // end of struct MarkdownMajorMode

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTMAJORMODE_H */
