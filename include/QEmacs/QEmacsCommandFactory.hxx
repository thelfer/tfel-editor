/*!
 * \file  QEmacsCommandFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_QEMACS_QEMACSCOMMANDFACTORY_HXX
#define LIB_QEMACS_QEMACSCOMMANDFACTORY_HXX

#include <map>
#include <memory>
#include <QtCore/QString>
#include "QEmacs/Config.hxx"
#include "QEmacs/QEmacsCommand.hxx"

namespace qemacs {

  struct QEmacsWidget;

  struct QEMACS_VISIBILITY_EXPORT QEmacsCommandProxy {
    virtual QString getName() const = 0;
    virtual QEmacsCommand* getQEmacsCommand(QEmacsWidget&) const = 0;
    /*!
     * destructor
     */
    virtual ~QEmacsCommandProxy();
  };  // end of struct QEmacsCommandProxy

  /*!
   * a standard implementation of a proxy
   * \param T : major mode
   */
  template <typename T>
  struct StandardQEmacsCommandProxy : public QEmacsCommandProxy {
    /*!
     * \param[in] n : name of the major mode
     */
    StandardQEmacsCommandProxy(const QString&, const bool = true);
    //! \return the name of the command
    QString getName() const override;
    //! \return a new instance of command
    QEmacsCommand* getQEmacsCommand(QEmacsWidget&) const override;
    /*!
     * destructor
     */
    ~StandardQEmacsCommandProxy() override;

   private:
    //! command name
    const QString name;
  };  // end of struct StandardQEmacsCommandProxy

  //! \brief qemacs command factory
  struct QEMACS_VISIBILITY_EXPORT QEmacsCommandFactory {
    //! a simple alias
    using QEmacsCommandPtr = std::shared_ptr<QEmacsCommand>;
    //! a simple alias
    using QEmacsCommandProxyPtr = std::shared_ptr<QEmacsCommandProxy>;

    static QEmacsCommandFactory& getQEmacsCommandFactory();

    void loadLibrary(const QString&);

    /*!
     * \return a command according to the given name or NULL if no
     * command is found
     *
     * \param[in] n : name \param[in] w : qemacs
     * widget
     */
    QEmacsCommand* getQEmacsCommand(const QString&,
                                    QEmacsWidget&) const;

    void addQEmacsCommand(const QEmacsCommandProxyPtr);

    QStringList getAvailableQEmacsCommandsNames() const;

    bool hasQEmacsCommand(const QString&) const;

   private:
    QEmacsCommandFactory();

    ~QEmacsCommandFactory();

    std::map<QString, QEmacsCommandProxyPtr> proxies;

  };  // end of struct QEmacsCommandFactory

}  // end of namespace qemacs

#include "QEmacs/QEmacsCommandFactory.ixx"

#endif /* LIB_QEMACS_QEMACSCOMMANDFACTORY_HXX */
