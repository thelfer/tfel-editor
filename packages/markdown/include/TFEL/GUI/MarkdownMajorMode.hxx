/*!
 * \file  MarkdownMajorMode.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTMAJORMODE_HXX
#define LIB_TFEL_GUI_MFRONTMAJORMODE_HXX

#include <QtGui/QSyntaxHighlighter>
// #include <TFEL/GUI/SpellChecker.hxx>
#include "TFEL/GUI/CompiledLanguageMajorModeBase.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;

  //! \brief major mode for editing Markdown documents
  struct MarkdownMajorMode : public CompiledLanguageMajorModeBase {
    MarkdownMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSpellCheckLanguage(const QString &) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QMenu *getSpecificMenu() override;

    void completeContextMenu(QMenu *const, const QTextCursor &) override;

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

    //    SpellChecker &getSpellChecker();

    //! destructor
    ~MarkdownMajorMode() override;

   protected slots:

    //! slot called to prepare a Markdown analysis
    virtual void runPandoc();
    //! slot called to start a new Markdown analysis
    virtual void startPandoc();

   protected:
    //! checking
    //    SpellChecker spellChecker;
    QSyntaxHighlighter *highlighter = nullptr;

   private:
    Q_OBJECT
  };  // end of struct MarkdownMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTMAJORMODE_H */
