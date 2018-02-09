/*!
 * \file  QEmacsBuffer.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 juin 2012
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
  struct QEmacsPlainTextEdit;

  /*!
   * class in charge of managing one buffer
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsBuffer : public QWidget {

    QEmacsBuffer(const int, QEmacsWidget &);

    QEmacsBuffer(const QString &, const int, QEmacsWidget &);

    virtual int getId() const;

    virtual QString getBufferName() const;

    virtual QString getBufferNameSuffix() const;

    virtual QString getBufferRawName() const;

    virtual QEmacsPlainTextEdit &getMainFrame();

    virtual QVector<QMenu *> getSpecificMenus();

    virtual QIcon getIcon() const;

    int getSlaveIndex(QWidget *const p) const;

    virtual QWidget *addSlave(const QString &, QWidget *const);

    virtual QString getSlaveName(QWidget *const) const;

    virtual void setSlaveName(QWidget *const, const QString &);

    virtual void setSlaveIcon(QWidget *const, const QIcon &);

    virtual void removeSlave(QWidget *const);

    virtual void hideSlave(QWidget *const);

    virtual bool hasSlaves() const;

    virtual bool areSlavesVisible() const;

    virtual bool isOkToClose() const;

   public slots:

    virtual void showSlaves();

    virtual void showSlaves(const Qt::Orientation);

    virtual void hideSlaves();

    virtual void focusCurrentSlave();

    virtual void focusMainFrame();

    virtual void closeCurrentSlave();

    virtual void updateMenu();

   signals:

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

    virtual void closeSlave(int);

    virtual void emitNewTreatedFile(const QString &);

   protected:
    struct SlaveTabWidget;

    void initialize();

    QEmacsWidget &qemacs;

    //! widget handling the main buffer and its slaves
    QSplitter *splitter;
    //! slave widgets
    SlaveTabWidget *slaves;

    QEmacsPlainTextEdit *e;

    // timer to update the date
    QTimer *timer;

    // display information about the current buffer
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

  };  // end of QEmacsBuffer

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSBUFFER_HXX */
