/*!
 * \file  EditorWidget.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#ifndef LIB_TFEL_GUI_EDITORWIDGET_HXX
#define LIB_TFEL_GUI_EDITORWIDGET_HXX

#include <vector>
#include <QtCore/QString>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtGui/QAction>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SecondaryTaskManager.hxx"

namespace tfel::gui {

  //! forward declaration
  struct Buffer;
  //! forward declaration
  struct LineEdit;
  //! forward declaration
  struct CommandLine;

  //! \brief a wigdet providing an emacs-inspired editor
  struct TFEL_GUI_VISIBILITY_EXPORT EditorWidget : public QWidget,
                                                   public SecondaryTaskManager {
    static QStringList &getRecentFiles();

    void addToRecentFiles(const QString &);

    /*!
     * \param[in] p : parent widget
     */
    EditorWidget(QWidget *const = nullptr);

    virtual bool isOkToClose();

    virtual QStringList getBuffersNames() const;

    virtual std::vector<QIcon> getBuffersIcons() const;

    virtual std::vector<int> getBuffersIds() const;

    virtual void changeBuffer(const QString &);

    virtual void changeBuffer(const int);

    virtual QString chooseBufferNameSuffix(Buffer *, const QString &);

    virtual std::vector<QMenu *> getCurrentBufferSpecificMenus();

    virtual Buffer &getCurrentBuffer();

    virtual void changeBuffer();

    virtual void launchCommand(const QString &);

    virtual void launchCommand();

    virtual const QStringList &getKillRing() const;

    virtual void addToKillRing(const QString &);

    virtual bool hasUserInput() const;

    //! destructor
    ~EditorWidget() override;

   public slots:

    /*!
     * change the font used by the main frames
     * \param[in] f : new font
     */
    void changeMainFramesFont(const QFont &);

    /*!
     * open a file in a new buffer
     */
    virtual void openFile();

    //! \brief print the current widget main frame
    virtual void print();

    /*!
     * open a file in a new buffer
     * \param[in] f : file name
     */
    virtual void openFile(const QString &);

    /*!
     * create a new buffer
     */
    virtual void createEmptyBuffer();

    /*!
     * save all modified buffers
     */
    virtual void saveAllBuffers();

    virtual void displayInformativeMessage(const QString &m);

    /*!
     * set an user input
     * \param[in] l : the line edit
     */
    virtual void setUserInput(CommandLine *const);

    virtual Buffer *getBufferVisitingFile(const QString &);

    virtual void focusUserInput();

    virtual void removeUserInputs();

    virtual void removeUserInput();

    virtual void removeUserInput(CommandLine *);

    /*!
     * \param[in] b : if true, check if the buffer is ok to close
     */
    virtual void closeCurrentBuffer(const bool = true);

    /*!
     * \param[in] b : if true, check if the buffer is ok to close
     */
    virtual void closeBuffer(Buffer &, const bool = true);

    virtual void saveCurrentBuffer();
    /*!
     * \brief go to the next buffer. If the current buffer is the
     * last one, cycle the first one (index 0).
     */
    virtual void nextBuffer();
    /*!
     * \brief go to the previous buffer. If the current buffer is the
     * first one (index 0), cycle the last one.
     */
    virtual void previousBuffer();
    /*!
     * \brief change the orientation of the secondary tasks in all
     * buffers
     * \param[in] o: orientation
     */
    virtual void setSecondaryTasksOrientation(const Qt::Orientation);

   signals:

    void closed();

    void bufferAdded();

    void bufferRemoved();

    void bufferNameChanged();

    void currentBufferChanged();

    void currentBufferMajorModeChanged();

    void updatedMenu();

    void newTreatedFile(const QString &);

   protected slots:

    virtual void resetUserInput();

    virtual void updateBufferName(Buffer *, const QString &, const QString &);

    virtual void emitNewTreatedFile(const QString &);

   protected:
    /*!
     * "open file" user input
     */
    struct OpenFile;

    /*!
     * change buffer user input
     */
    struct ChangeBuffer;

    /*!
     * launch a new command
     */
    struct Command;

    struct SaveCurrentBufferAndKill;

    virtual void closeEvent(QCloseEvent *) override;

    /*!
     * create a new buffer
     */
    virtual Buffer *createNewBuffer(const QString & = "");

    /*!
     * set the current buffer, add it at the top of history, emit
     * the currentBufferChanged signal, and give it the focus
     */
    virtual void setCurrentBuffer(Buffer *const);

    virtual void removeBuffer(Buffer *const) override;

    //! buffers
    QStackedWidget *buffers;
    //! buffer history
    QStringList bHistory;
    //! the minibuffer
    QStackedWidget *minibuffer;
    //! user message
    LineEdit *um;
    //! user input
    std::vector<CommandLine *> ui;
    //! empty line edit
    LineEdit *eui;
    //! the list of tokens pasted/killed
    QStringList killRing;
    //! id of the next buffer
    int nid;

   private:
    Q_OBJECT
  };  // end of struct

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_EDITORWIDGET_HXX */
