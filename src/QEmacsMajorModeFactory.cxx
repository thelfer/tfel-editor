/*! 
 * \file  QEmacsMajorModeFactory.cxx
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
#include"QEmacs/QEmacsMajorModeChangeCommand.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{

  QEmacsMajorModeProxy::~QEmacsMajorModeProxy()
  {} // end of QEmacsMajorModeProxy::~QEmacsMajorModeProxy()
      
  void
  QEmacsMajorModeFactory::loadLibrary(const QString& lib)
  {
    using namespace tfel::system;
    ExternalLibraryManager& lm = ExternalLibraryManager::getExternalLibraryManager();
    lm.loadLibrary(lib.toStdString());
  }
      
  QEmacsMajorMode *
  QEmacsMajorModeFactory::getQEmacsMajorModeByName(const QString& n,
						   QEmacsWidget& w,
						   QEmacsBuffer& b,
						   QEmacsTextEditBase& t) const
  {
    for(auto p=proxies.begin();p!=proxies.end();++p){
      if(p->proxy->getName()==n){
	return (*p).proxy->getQEmacsMajorMode(w,b,t);
      }
    }
    w.displayInformativeMessage(QObject::tr("no major mode named '%1' registred.").arg(n));
    return nullptr;
  } // end of QEmacsMajorModeFactory::getQEmacsMajorModeByName

  bool
  QEmacsMajorModeFactory::hasQEmacsMajorMode(const QString& n) const
  {
    QVector<Proxy>::const_iterator p;
    for(p=proxies.begin();p!=proxies.end();++p){
      if(p->proxy->getName()==n){
	return true;
      }
    }
    return false;
  } // end of QEmacsMajorModeFactory::hasQEmacsMajorModeByName

  QEmacsMajorMode *
  QEmacsMajorModeFactory::getQEmacsMajorModeForFile(const QString& f,
						    QEmacsWidget& w,
						    QEmacsBuffer& b,
						    QEmacsTextEditBase& t) const
  {
    for(auto p=proxies.begin();p!=proxies.end();++p){
      for(auto pr=p->rexp.begin();pr!=p->rexp.end();++pr){
	const QRegExp& r = *pr;
	if(r.indexIn(f)>=0){
	  return (*p).proxy->getQEmacsMajorMode(w,b,t);
	}
      }
    }
    w.displayInformativeMessage(QObject::tr("no major mode for file '%1'").arg(f));
    return nullptr;
  } // end of QEmacsMajorModeFactory::getQEmacsMajorMode

  void
  QEmacsMajorModeFactory::addQEmacsMajorMode(const QEmacsMajorModeFactory::QEmacsMajorModeProxyPtr proxy,
					     const QVector<QRegExp>& e,
					     const bool b)
  {
    using namespace std;
    Proxy p;
    p.proxy = proxy;
    p.rexp  = e;
    if(b){
      typedef QEmacsMajorModeChangeCommandProxy CProxy;
      shared_ptr<QEmacsCommandProxy> cp;
      const QString& n = proxy->getName();
      QEmacsCommandFactory& f = QEmacsCommandFactory::getQEmacsCommandFactory();
      cp = shared_ptr<QEmacsCommandProxy>(new CProxy(n+"-mode",n));
      f.addQEmacsCommand(cp);
    }
    this->proxies.push_front(p);
  } // end of QEmacsMajorModeFactory::getQEmacsMajorMode

  QStringList
  QEmacsMajorModeFactory::getAvailableQEmacsMajorModesNames(void) const
  {
    QStringList n;
    QVector<Proxy>::const_iterator p;
    for(p=proxies.begin();p!=proxies.end();++p){
      n.push_back(p->proxy->getName());
    }
    return n;
  } // end of QEmacsMajorModeFactory::getAvailableQEmacsMajorModesNames(void) const

  QEmacsMajorModeFactory&
  QEmacsMajorModeFactory::getQEmacsMajorModeFactory()
  {
    static QEmacsMajorModeFactory m;
    return m;
  } // end of QEmacsMajorModeFactory::~QEmacsMajorModeFactory()

  QEmacsMajorModeFactory::QEmacsMajorModeFactory()
  {} // end of QEmacsMajorModeFactory::QEmacsMajorModeFactory()

  QEmacsMajorModeFactory::~QEmacsMajorModeFactory()
  {} // end of QEmacsMajorModeFactory::~QEmacsMajorModeFactory

} // end of namespace qemacs

