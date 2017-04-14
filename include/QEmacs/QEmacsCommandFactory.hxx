/*! 
 * \file  QEmacsCommandFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_QEMACSCOMMANDFACTORY_H_
#define _LIB_QEMACS_QEMACSCOMMANDFACTORY_H_ 

#include<memory>
#include<QtCore/QMap>
#include<QtCore/QString>

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsCommand.hxx"

namespace qemacs
{

  class QEmacsWidget;

  struct QEMACS_VISIBILITY_EXPORT QEmacsCommandProxy
  {
    virtual QString
    getName(void) const = 0;
    virtual QEmacsCommand*
    getQEmacsCommand(QEmacsWidget&) const = 0;
    /*!
     * destructor
     */
    virtual ~QEmacsCommandProxy();
  }; // end of struct QEmacsCommandProxy

  /*!
   * a standard implementation of a proxy
   * \param T : major mode
   */
  template<typename T>
  struct StandardQEmacsCommandProxy
    : public QEmacsCommandProxy
  {
    /*!
     * \param[in] n : name of the major mode
     */
    StandardQEmacsCommandProxy(const QString&,
			       const bool = true);
    /*!
     * \param[in] n : return the major mode name
     */
    virtual QString
    getName(void) const override;
    /*!
     * \return a new instance of the major mode
     */
    virtual QEmacsCommand *
    getQEmacsCommand(QEmacsWidget&) const override;
    /*!
     * destructor
     */
    ~StandardQEmacsCommandProxy();
  private:
    //! command name
    const QString name;
  }; // end of struct StandardQEmacsCommandProxy
  
  /*!
   * Major mode factory
   */
  struct QEmacsCommandFactory
  {
    typedef std::shared_ptr<QEmacsCommand>      QEmacsCommandPtr;    
    typedef std::shared_ptr<QEmacsCommandProxy> QEmacsCommandProxyPtr;    

    static QEmacsCommandFactory&
    getQEmacsCommandFactory();

    void
    loadLibrary(const QString&);

    /*!
     * \return a command according to the given name or NULL if no
     * command is found
     *
     * \param[in] n : name \param[in] w : qemacs
     * widget
     */
    QEmacsCommand *
    getQEmacsCommand(const QString&,
		     QEmacsWidget&) const;
    
    void
    addQEmacsCommand(const QEmacsCommandProxyPtr);
    
    QList<QString>
    getAvailableQEmacsCommandsNames(void) const;

    bool
    hasQEmacsCommand(const QString&) const;

  private:

    QEmacsCommandFactory();

    ~QEmacsCommandFactory();
    
    QMap<QString,QEmacsCommandProxyPtr> proxies;

  }; // end of struct QEmacsCommandFactory

} // end of namespace qemacs

#include"QEmacs/QEmacsCommandFactory.ixx"

#endif /* _LIB_QEMACS_QEMACSCOMMANDFACTORY_H */
