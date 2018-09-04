/*!
 * \file  TextEditBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   19/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITBASE_HXX
#define LIB_TFEL_GUI_TEXTEDITBASE_HXX

#include <map>
#include <functional>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QTextCursor>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel{

  namespace gui{

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  // forward declaration
  struct MajorMode;

  struct TFEL_GUI_VISIBILITY_EXPORT TextEditBase : public QWidget {
   signals:

    void fileNameChanged(const QString &);

    void majorModeChanged();

    void specificMenuChanged();

    void cursorPositionChanged();

   public slots:

    virtual void format();

    virtual void print() = 0;

    virtual void clear() = 0;

    virtual void undo() = 0;

    virtual void redo() = 0;

    virtual void cut() = 0;

    virtual void copy() = 0;

    virtual void paste() = 0;

    virtual void selectAll() = 0;

    virtual void setHtml(const QString &) = 0;

    virtual void insertHtml(const QString &) = 0;

    virtual void appendHtml(const QString &) = 0;

    virtual void setPlainText(const QString &) = 0;

    virtual void insertPlainText(const QString &) = 0;

    virtual void appendPlainText(const QString &) = 0;

    virtual void setReadOnly(const bool) = 0;

    virtual void setTextInteractionFlags(Qt::TextInteractionFlags) = 0;

    virtual void setTextCursor(const QTextCursor &) = 0;

    virtual void centerCursor() = 0;

    virtual void setUndoRedoEnabled(const bool) = 0;

    virtual void setFont(const QFont &);
    /*!
     * \param[in] n : mode name
     */
    virtual MajorMode *setMajorMode(const QString &);

    virtual MajorMode *setMajorMode();

    virtual void setMajorMode(MajorMode *const);

    virtual void setFileName(const QString &);

    virtual void writeFile();

    virtual void writeFile(const QString &);

    virtual void save();

    virtual void gotoLine(const int);
    /*!
     * \brief
     * \param[in] l: line number
     * \param[in] c: column number
     */
    virtual void gotoPosition(const int, const int);

    virtual void insertCompletion(const QString &);

    virtual void setMoveMode(QTextCursor::MoveMode);

   public:
    //! return the underlying editor widget
    EditorWidget &getEditorWidget();
    //! return the underlying editor widget
    const EditorWidget &getEditorWidget() const;

    /*!
     * \brief a `CommandLine` asking the user whether to save the
     * current buffer
    */
    struct SaveInput : public YesOrNoUserInput {
      bool isBlocking() const override;

      void treatUserInput() override;

     protected:
      friend struct TextEditBase;

      SaveInput(EditorWidget &, TextEditBase &);

      TextEditBase &textEdit;
    };  // end of struct SaveInput

    /*!
     * "kill other buffer and write file" user input
     */
    struct KillOtherBufferAndWriteFile;

    /*!
     * "overwrite file" user input
     */
    struct OverWriteFile;

    /*!
     * "write file" user input
     */
    struct WriteFile;

    TextEditBase(EditorWidget &, Buffer &);

    ~TextEditBase() override;

    /*!
     * \param[in] e : event
     * \brief convert a mouse move event to the mouseMoved signal.
     * \note this function is only called if mouse tracking is
     * enabled;
     */
    void mouseMoveEvent(QMouseEvent *) override;

    virtual QAbstractScrollArea *widget() = 0;

    /*!
     * \return true if the buffer is a main frame
     */
    virtual bool isMainFrame() const;

    virtual void setSpellCheckLanguage(const QString &);

    /*!
     * \return true if the buffer is not a main frame
     */
    virtual bool isSecondaryTask() const;

    virtual void setFocus();

    virtual QString getFileName() const;

    virtual QString getCompleteFileName() const;

    /*!
     * \return the absolute path to the directory containing the
     * document being treated.
     */
    virtual QString getDirectory() const;

    virtual SaveInput *getSaveInput();

    /*!
     * \return the word under cursor
     */
    virtual QString getCurrentWord() const;

    virtual bool isModified() const;

    virtual bool hasMajorMode() const;

    virtual const MajorMode &getMajorMode() const;

    virtual std::vector<QMenu *> getSpecificMenus();

    virtual QIcon getIcon() const;

    virtual bool setKeyPressEventFilter(
        TextEditKeyPressEventFilter *const);

    virtual void removeKeyPressEventFilter();

    virtual QTextDocument *document() const = 0;

    virtual void moveCursor(
        QTextCursor::MoveOperation,
        QTextCursor::MoveMode = QTextCursor::MoveAnchor) = 0;

    virtual bool isReadOnly() = 0;

    virtual bool find(const QString &,
                      QTextDocument::FindFlags = nullptr) = 0;

    virtual QTextCursor textCursor() const = 0;

    virtual QTextCursor cursorForPosition(const QPoint &pos) const = 0;

    virtual QRect cursorRect(const QTextCursor &cursor) const = 0;

    virtual QRect cursorRect() const = 0;

    virtual void setExtraSelections(
        const QList<QTextEdit::ExtraSelection> &) = 0;

    virtual QList<QTextEdit::ExtraSelection> extraSelections()
        const = 0;

    virtual bool isUndoRedoEnabled() const = 0;

   protected slots:

    virtual void highlightCurrentLine();

    virtual void keyPressEventFilterDestroyed();

    virtual void emitCursorPositionChanged();

   protected:
    /*!
     * a editor user input which asks
     * the user where he wants to go
     */
    struct GotoLine : public CommandLine {
      GotoLine(TextEditBase &, EditorWidget &);

      ~GotoLine() override;

     protected:
      void treatUserInput() override;

      TextEditBase &textEdit;

    };  // end of struct GotoLine

    static QString getModifier(const QKeyEvent &);

    bool event(QEvent *) override;

    bool eventFilter(QObject *, QEvent *) override;

    void keyPressEvent(QKeyEvent *) override;

    virtual bool handleKeyPressEvent(QKeyEvent *);

    virtual bool handleKeyPressEventWithEmacsShortCuts(QKeyEvent *);

    virtual bool handleKeyPressEventWithQtShortCuts(QKeyEvent *);

    virtual bool handleMousePressEvent(QMouseEvent *);

    virtual void initialize(QAbstractScrollArea *const);

    virtual void addToKillRing(const QString &);

    virtual void createContextMenuActions();

    virtual void deleteContextMenuActions();

    //! reference to the widget
    EditorWidget &editor;

    Buffer &buffer;

    //! major mode
    MajorMode *mode = nullptr;

    //! key press filter
    TextEditKeyPressEventFilter *filter = nullptr;

    QTextCursor::MoveMode moveMode = QTextCursor::MoveAnchor;
    /*!
     * save the previous positions of the cursor for Ctrl-P/Ctrl-N
     * moves
     */
    std::map<int, int> positions;

    QString fileName;

    QString spellCheckLanguage;

    // undo action
    QAction *ua = nullptr;
    // redo action
    QAction *ra = nullptr;
    // cut action
    QAction *ca = nullptr;
    // copy action
    QAction *coa = nullptr;
    // paste action
    QAction *pa = nullptr;
    // select all action
    QAction *sa = nullptr;

    int pring;  //<! position if the ring

    bool yank = false;

    bool ctrlx = false;

    bool ctrlc = false;

    bool allowMajorModeChange = true;

   private:
    friend struct Buffer;

    virtual void setMainFrame(const bool);

    bool mainFrame = false;

    bool keyProcessing = false;

    Q_OBJECT

  };  // end of struct TextEditBase

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_TEXTEDITBASE_HXX */
