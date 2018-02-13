/*! 
 * \file  QEmacsCommandFactory.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include<utility>
#include<stdexcept>

#include<QtCore/QDebug>

#include"TFEL/System/ExternalLibraryManager.hxx"

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsCommandFactory.hxx"

namespace qemacs
{

  QEmacsCommandProxy::~QEmacsCommandProxy() = default;
      
  void
  QEmacsCommandFactory::loadLibrary(const QString& lib)
  {
    using namespace tfel::system;
    ExternalLibraryManager& lm = ExternalLibraryManager::getExternalLibraryManager();
    lm.loadLibrary(lib.toStdString());
  }
      
  QEmacsCommand *
  QEmacsCommandFactory::getQEmacsCommand(const QString& n,
					 QEmacsWidget& w) const
  {
    auto p = this->proxies.find(n);
    if(p==this->proxies.end()){
      w.displayInformativeMessage(QObject::tr("no major mode named '%1' registred.").arg(n));
      return nullptr;
    }
    return (*p)->getQEmacsCommand(w);
  } // end of QEmacsCommandFactory::getQEmacsCommandByName

  bool
  QEmacsCommandFactory::hasQEmacsCommand(const QString& n) const
  {
    return this->proxies.find(n)!=this->proxies.end();
  } // end of QEmacsCommandFactory::hasQEmacsCommandByExtension

  void
  QEmacsCommandFactory::addQEmacsCommand(const QEmacsCommandFactory::QEmacsCommandProxyPtr proxy)
  {
    using namespace std;
    const QString& name = proxy->getName();
    QStringList::const_iterator ps;
    if(this->proxies.find(name)!=this->proxies.end()){
      qDebug() << "QEmacsCommandFactory::getQEmacsCommand : "
	       << QObject::tr("a command named '%1' has already been registred.").arg(name);
      return;
    }
    this->proxies.insert(name,proxy);
  } // end of QEmacsCommandFactory::getQEmacsCommand

  QList<QString>
  QEmacsCommandFactory::getAvailableQEmacsCommandsNames() const
  {
    return this->proxies.keys();
  } // end of QEmacsCommandFactory::getAvailableQEmacsCommandsNames() const

  QEmacsCommandFactory&
  QEmacsCommandFactory::getQEmacsCommandFactory()
  {
    static QEmacsCommandFactory m;
    return m;
  } // end of QEmacsCommandFactory::~QEmacsCommandFactory()

  QEmacsCommandFactory::QEmacsCommandFactory() = default;

  QEmacsCommandFactory::~QEmacsCommandFactory() = default;

} // end of namespace qemacs

