/*! 
 * \file  QEmacsBuffer.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 juin 2012
 */

#ifndef _LIB_QEMACS_QEMACSBUFFER_H_
#define _LIB_QEMACS_QEMACSBUFFER_H_ 

#include<QtCore/QMap>
#include<QtCore/QVector>
#include<QtCore/QString>
#include<QtCore/QStringList>

#ifdef QEMACS_QT4
#include<QtGui/QMenu>
#include<QtGui/QLabel>
#include<QtGui/QWidget>
#include<QtGui/QTabWidget>
#include<QtGui/QHBoxLayout>
#include<QtGui/QAbstractScrollArea>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QMenu>
#include<QtWidgets/QLabel>
#include<QtWidgets/QWidget>
#include<QtWidgets/QTabWidget>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QAbstractScrollArea>
#endif /* QEMACS_QT5 */

namespace qemacs
{

  //! forward declaration
  class QEmacsWidget;

  //! forward declaration
  class QEmacsPlainTextEdit;

  /*!
   * class in charge of managing one buffer
   */
  class QEmacsBuffer
    : public QWidget
  {
      
    Q_OBJECT

  public:

    QEmacsBuffer(const int,
		 QEmacsWidget&);

    QEmacsBuffer(const QString&,
		 const int,
		 QEmacsWidget&);

    virtual int
    getId(void) const;
    
    virtual QString
    getBufferName(void) const;

    virtual QString
    getBufferNameSuffix() const;

    virtual QString
    getBufferRawName() const;

    virtual QEmacsPlainTextEdit&
    getMainFrame();

    virtual QVector<QMenu*>
    getSpecificMenus();

    virtual QIcon
    getIcon() const;


    int
    getSlaveIndex(QWidget * const p) const;

    virtual QWidget *
    addSlave(const QString&,
	     QWidget *const);

    virtual QString
    getSlaveName(QWidget *const) const;

    virtual void
    setSlaveName(QWidget *const,
		 const QString &);

    virtual void
    setSlaveIcon(QWidget *const,
		 const QIcon&);

    virtual void
    removeSlave(QWidget *const);

    virtual void
    hideSlave(QWidget *const);

    virtual bool
    hasSlaves(void) const;

    virtual bool
    areSlavesVisible(void) const;

    virtual bool
    isOkToClose(void) const;
			     
  public slots:

    virtual void
    showSlaves(void);

    virtual void
    hideSlaves(void);

    virtual void
    focusCurrentSlave();

    virtual void
    focusMainFrame();

    virtual void
    closeCurrentSlave(void);
    
  signals:

    /*!
     * \param[out] b : reference to this
     * \param[out] o : old name
     * \param[out] n : new name
     */
    void bufferNameChanged(QEmacsBuffer *,
			   const QString&,
			   const QString&);

    void newTreatedFile(const QString&);

  protected slots:

    virtual void
    focusInEvent(QFocusEvent *) override;

    virtual void
    updatePosition(void);

    virtual void
    updateDate(void);

    virtual void
    updateBufferInformations(void);

    virtual void
    updateBufferName(void);

    virtual void
    closeSlave(int);

    virtual void
    emitNewTreatedFile(const QString&);

  protected:

    struct SlaveTabWidget;

    void
    initialize();

    QEmacsWidget& qemacs;
    
    /*!
     * slave widget
     */
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

  }; // end of QEmacsBuffer

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSBUFFER_H */

