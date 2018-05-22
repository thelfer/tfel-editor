/*!
 * \file  MFrontMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#ifndef LIB_QEMACS_MFRONTMAJORMODE_HXX
#define LIB_QEMACS_MFRONTMAJORMODE_HXX

#include <QtCore/QTimer>
#include "QEmacs/CxxMajorMode.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsTextEditBase;
  // forward declaration
  struct MFrontSyntaxHighlighter;

  //! \brief a major mode for editing `MFront` input files
  struct MFrontMajorMode : public CxxMajorMode {
    MFrontMajorMode(QEmacsWidget &,
                    QEmacsBuffer &,
                    QEmacsTextEditBase &);

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
    void runImplicitDSLWizard();

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

  void runMFront(QEmacsWidget &);

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTMAJORMODE_H */
