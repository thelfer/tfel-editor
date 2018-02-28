/*!
 * \file  QEmacsBuffer.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#ifndef LIB_QEMACS_QEMACSBUFFER_HXX
#define LIB_QEMACS_QEMACSBUFFER_HXX

#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAbstractScrollArea>
#include "QEmacs/Config.hxx"

namespace qemacs {

  //! forward declaration
  struct QEmacsWidget;

  //! forward declaration
  struct SecondaryTaskManager;

  //! forward declaration
  struct QEmacsPlainTextEdit;

  /*!
   * \brief class in charge of managing one buffer.
   *
   * A buffer is made of:
   * - a main widget (most of the time an instance of
   *   `QEmacsPlainTextEdit`)
   * - SecondaryTasks of that main widget
   *
   *  \note: a SecondaryTask can be shared between several buffers.
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsBuffer : public QWidget {
    QEmacsBuffer(const int, QEmacsWidget &);

    QEmacsBuffer(const QString &, const int, QEmacsWidget &);

    virtual int getId() const;

    virtual QString getBufferName() const;

    virtual QString getBufferNameSuffix() const;

    virtual QString getBufferRawName() const;

    virtual QEmacsPlainTextEdit &getMainFrame();
    //! \return the current SecondaryTask if any, nullptr otherwise
    virtual QWidget *getCurrentSecondaryTask();
    /*!
     * \return the title of the current SecondaryTask if any,
     * an empty string otherwise
     */
    virtual QString getCurrentSecondaryTaskTitle() const;

    virtual QVector<QMenu *> getSpecificMenus();

    virtual QIcon getIcon() const;

    int getSecondaryTaskIndex(QWidget *const p) const;

    virtual void refreshSecondaryTaskTabWidget();

    /*!
     * \brief attach a secondary task to the buffer.
     * \param[in] t: title
     * \param[in] w: associated widget
     */
    virtual QWidget *attachSecondaryTask(const QString &,
                                         QWidget *const);
    /*!
     * \brief attach an existing task to the buffer
     * \param[in] w: widget
     */
    virtual void attachSecondaryTask(QWidget *const);

    virtual void removeSecondaryTask(QWidget *const);

    virtual QString getSecondaryTaskTitle(QWidget *const) const;

    virtual void setSecondaryTaskTitle(QWidget *const, const QString &);

    virtual void setSecondaryTaskIcon(QWidget *const, const QIcon &);

    virtual void showSecondaryTask(QWidget *const);

    virtual void hideSecondaryTask(QWidget *const);

    virtual bool hasSecondaryTasks() const;

    virtual bool areSecondaryTasksVisible() const;

    virtual bool isOkToClose() const;

   public slots:

    virtual void showSecondaryTasks();

    virtual void showSecondaryTasks(const Qt::Orientation);

    virtual void hideSecondaryTasks();

    virtual void focusCurrentSecondaryTask();

    virtual void focusMainFrame();

    virtual void closeCurrentSecondaryTask();

    virtual void updateMenu();

   signals:

    void mainFrameMajorModeChanged();
    //! signal launched when the menu of the buffer shall be updated
    void updatedMenu();
    /*!
     * \param[out] b : reference to this
     * \param[out] o : old name
     * \param[out] n : new name
     */
    void bufferNameChanged(QEmacsBuffer *,
                           const QString &,
                           const QString &);

    void newTreatedFile(const QString &);

   protected slots:

    void focusInEvent(QFocusEvent *) override;

    virtual void updatePosition();

    virtual void updateDate();

    virtual void updateBufferInformations();

    virtual void updateBufferName();

    virtual void closeSecondaryTask(int);

    virtual void emitNewTreatedFile(const QString &);

   protected:
    struct SecondaryTaskTabWidget;

    void initialize();

    QEmacsWidget &qemacs;

    //! widget handling the main buffer and its SecondaryTasks
    QSplitter *splitter;
    //! SecondaryTask widgets
    SecondaryTaskTabWidget *stw;
    //! main widget
    QEmacsPlainTextEdit *e;
    // timer to update the date
    QTimer *timer;
    //! display information about the current buffer
    QHBoxLayout *info;
    //! buffer name
    QLabel *bni;
    //! relative position
    QLabel *rpi;
    //! absolute position
    QLabel *api;
    //! icon
    QLabel *ii;
    //! mode name
    QLabel *mi;
    //! time
    QLabel *ti;
    //! buffer suffix
    QString bufferNameSuffix;
    //! buffer id
    const int id;

   private:
    Q_OBJECT

    friend struct SecondaryTaskManager;

  };  // end of QEmacsBuffer

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSBUFFER_HXX */
