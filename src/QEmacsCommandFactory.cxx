/*! 
 * \file  QEmacsCommandFactory.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include<utility>
#include<stdexcept>

#include<QtCore/QDebug>

#include"TFEL/System/ExternalLibraryManager.hxx"

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsCommandFactory.hxx"

namespace qemacs
{

  QEmacsCommandProxy::~QEmacsCommandProxy()
  {} // end of QEmacsCommandProxy::~QEmacsCommandProxy()
      
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
  QEmacsCommandFactory::getAvailableQEmacsCommandsNames(void) const
  {
    return this->proxies.keys();
  } // end of QEmacsCommandFactory::getAvailableQEmacsCommandsNames(void) const

  QEmacsCommandFactory&
  QEmacsCommandFactory::getQEmacsCommandFactory()
  {
    static QEmacsCommandFactory m;
    return m;
  } // end of QEmacsCommandFactory::~QEmacsCommandFactory()

  QEmacsCommandFactory::QEmacsCommandFactory()
  {} // end of QEmacsCommandFactory::QEmacsCommandFactory()

  QEmacsCommandFactory::~QEmacsCommandFactory()
  {} // end of QEmacsCommandFactory::~QEmacsCommandFactory

} // end of namespace qemacs

