/*!
 * \file  CastemMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   05/08/2012
 */

#include <QtCore/QVector>

#include "QEmacs/QEmacsMajorModeBase.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

#include "QEmacs/ProcessInteractionFrame.hxx"
#include "QEmacs/QEmacsFileDownloader.hxx"
#include "QEmacs/QEmacsPlainTextEdit.hxx"

namespace qemacs {

  /*!
   * \brief a major mode dedicated to the `Cast3M` finite element solver
   */
  struct CastemMajorMode : public QEmacsMajorModeBase {
    //! \return the list of `Cast3M` keyword
    static const QStringList& getKeysList();

    static bool isCastemKeyWord(const QString &);

    CastemMajorMode(QEmacsWidget &,
                    QEmacsBuffer &,
                    QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *) override;

    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;

    QCompleter *getCompleter() override;
    //! \return a menu specific to this mode
    QMenu* getSpecificMenu() override;
    /*!
     * \brief complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    void completeContextMenu(QMenu *const,
                             const QTextCursor &) override;

    void completeCurrentWord(QEmacsTextEditBase &,
                             const QString &) override;

    int getMinimalCompletionLength() override;

    void format() override;

    void indentLine(const QTextCursor &) override;

    QString getCommentSyntax() override;

    QIcon getIcon() const override;

    ~CastemMajorMode() override;

   protected slots:

    virtual bool sendLineToCastem();

    virtual bool sendRegionToCastem();

    virtual bool sendBufferToCastem();

    virtual void showImportBehaviourWizard();

    virtual void showImportMFMBehaviourWizard();

   protected:
    static QStringList buildKeysList();

    virtual void displayHelp(const QString &, const QString &);

    virtual void openWebHelp(const QString &);

    virtual bool sendToCastem(const QString &);

    virtual bool startCastem();

    // completer
    QCompleter *c;

    ProcessInteractionFrame *co;

   private:
    Q_OBJECT
  };  // end of struct CastemMajorMode

}  // end of namespace qemacs
