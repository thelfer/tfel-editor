/*!
 * \file  QEmacsWidget.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#ifndef LIB_QEMACS_QEMACSWIDGET_HXX
#define LIB_QEMACS_QEMACSWIDGET_HXX

#include <vector>
#include <QtCore/QString>
#include <QtWidgets/QLabel>
#include <QtWidgets/QAction>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include "QEmacs/Config.hxx"
#include "QEmacs/SecondaryTaskManager.hxx"

namespace qemacs {

  //! forward declaration
  struct QEmacsBuffer;
  //! forward declaration
  struct QEmacsLineEdit;

  //! \brief a wigdet providing an emacs-inspired editor
  struct QEMACS_VISIBILITY_EXPORT QEmacsWidget
      : public QWidget,
        public SecondaryTaskManager {
    static QStringList &getRecentFiles();

    void addToRecentFiles(const QString &);

    /*!
     * \param[in] p : parent widget
     */
    QEmacsWidget(QWidget *const = nullptr);

    virtual bool isOkToClose();

    virtual QStringList getBuffersNames() const;

    virtual std::vector<QIcon> getBuffersIcons() const;

    virtual std::vector<int> getBuffersIds() const;

    virtual void changeBuffer(const QString &);

    virtual void changeBuffer(const int);

    virtual QString chooseBufferNameSuffix(QEmacsBuffer *,
                                           const QString &);

    virtual std::vector<QMenu *> getCurrentBufferSpecificMenus();

    virtual QEmacsBuffer &getCurrentBuffer();

    virtual void changeBuffer();

    virtual void launchCommand(const QString &);

    virtual void launchCommand();

    virtual const QStringList &getKillRing() const;

    virtual void addToKillRing(const QString &);

    virtual bool hasUserInput() const;

    //! destructor
    ~QEmacsWidget() override;

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
    virtual void setUserInput(QEmacsLineEdit *const);

    virtual QEmacsBuffer *getBufferVisitingFile(const QString &);

    virtual void focusUserInput();

    virtual void removeUserInputs();

    virtual void removeUserInput();

    virtual void removeUserInput(QEmacsLineEdit *);

    /*!
     * \param[in] b : if true, check if the buffer is ok to close
     */
    virtual void closeCurrentBuffer(const bool = true);

    /*!
     * \param[in] b : if true, check if the buffer is ok to close
     */
    virtual void closeBuffer(QEmacsBuffer &, const bool = true);

    virtual void saveCurrentBuffer();

   signals:

    void closed();

    void bufferAdded();

    void bufferRemoved();

    void bufferNameChanged();

    void currentBufferChanged();

    void currentBufferMajorModeChanged();

    void newTreatedFile(const QString &);

   protected slots:

    virtual void resetUserInput();

    virtual void updateBufferName(QEmacsBuffer *,
                                  const QString &,
                                  const QString &);

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
    virtual QEmacsBuffer *createNewBuffer(const QString & = "");

    /*!
     * set the current buffer, add it at the top of history, emit
     * the currentBufferChanged signal, and give it the focus
     */
    virtual void setCurrentBuffer(QEmacsBuffer *const);

    virtual void removeBuffer(QEmacsBuffer *const) override;

    //! buffers
    QStackedWidget *buffers;
    //! buffer history
    QStringList bHistory;
    //! the minibuffer
    QStackedWidget *minibuffer;
    //! user message
    QLineEdit *um;
    //! user input
    std::vector<QEmacsLineEdit *> ui;
    //! empty line edit
    QLineEdit *eui;
    //! the list of tokens pasted/killed
    QStringList killRing;
    //! id of the next buffer
    int nid;

   private:
    Q_OBJECT
  };  // end of struct QEmacs

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSWIDGET_HXX */
