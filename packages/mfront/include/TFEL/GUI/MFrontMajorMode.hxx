/*!
 * \file  MFrontMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTMAJORMODE_HXX
#define LIB_TFEL_GUI_MFRONTMAJORMODE_HXX

#include <QtCore/QTimer>
#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel{

  namespace gui{

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct MFrontSyntaxHighlighter;

  //! \brief a major mode for editing `MFront` input files
  struct MFrontMajorMode : public CxxMajorMode {
    MFrontMajorMode(EditorWidget &,
                    Buffer &,
                    TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QMenu *getSpecificMenu() override;

    void completeContextMenu(QMenu *const,
                             const QTextCursor &) override;

    QCompleter *getCompleter() override;

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
    //! run the Implicit DSL wizard
    void runMFrontBehaviourWizard();

   protected:
    //! return the list of MTest keywords
    virtual QStringList getKeyWordsList();
    void runCompilation() override;
    /*!
     * \brief a timer to refresh syntax highlighting
     * This is mandatory since the value after `@DSL` can change with
     * the user inputs.
     */
    QTimer *rt = nullptr;
    //! completer
    QCompleter *c = nullptr;
    //! current dsl
    QString current_dsl;
    // current syntax highlighter
    MFrontSyntaxHighlighter *highlighter = nullptr;

   private:
    Q_OBJECT
  };  // end of struct MFrontMajorMode

  void runMFront(EditorWidget &);

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTMAJORMODE_H */
