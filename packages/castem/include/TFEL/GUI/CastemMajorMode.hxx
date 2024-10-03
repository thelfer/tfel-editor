/*!
 * \file  CastemMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   05/08/2012
 */

#include <QtCore/QVector>

#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

#include "TFEL/GUI/ProcessInteractionFrame.hxx"
#include "TFEL/GUI/FileDownloader.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"

namespace tfel::gui {

  /*!
   * \brief a major mode dedicated to the `Cast3M` finite element solver
   */
  struct CastemMajorMode : public MajorModeBase {
    //! \return the list of `Cast3M` keyword
    static const QStringList &getKeysList();

    static bool isCastemKeyWord(const QString &);

    CastemMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *) override;

    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;

    QCompleter *getCompleter() override;
    //! \return a menu specific to this mode
    QMenu *getSpecificMenu() override;
    /*!
     * \brief complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    void completeCurrentWord(TextEditBase &, const QString &) override;

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

}  // end of namespace tfel::gui
