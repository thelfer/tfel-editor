/*! 
 * \file  QEmacsMajorModeFactory.cxx
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
#include"QEmacs/QEmacsMajorModeChangeCommand.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{

  QEmacsMajorModeProxy::~QEmacsMajorModeProxy() = default;
      
  void QEmacsMajorModeFactory::loadLibrary(const QString& lib)
  {
    using namespace tfel::system;
    auto& lm = ExternalLibraryManager::getExternalLibraryManager();
    lm.loadLibrary(lib.toStdString());
  }

  QEmacsMajorMode* QEmacsMajorModeFactory::getQEmacsMajorModeByName(
      const QString& n,
      QEmacsWidget& w,
      QEmacsBuffer& b,
      QEmacsTextEditBase& t) const {
    for(auto& p : proxies){
      if(p.proxy->getName()==n){
        return p.proxy->getQEmacsMajorMode(w, b, t);
      }
    }
    w.displayInformativeMessage(
        QObject::tr("no major mode named '%1' registred.").arg(n));
    return nullptr;
  } // end of QEmacsMajorModeFactory::getQEmacsMajorModeByName

  bool QEmacsMajorModeFactory::hasQEmacsMajorMode(
      const QString& n) const {
    for (const auto& p : this->proxies) {
      if (p.proxy->getName() == n) {
        return true;
      }
    }
    return false;
  } // end of QEmacsMajorModeFactory::hasQEmacsMajorModeByName

  QEmacsMajorMode* QEmacsMajorModeFactory::getQEmacsMajorModeForFile(
      const QString& f,
      QEmacsWidget& w,
      QEmacsBuffer& b,
      QEmacsTextEditBase& t) const {
    for (const auto& p : this->proxies) {
      for(const auto& r : p.rexp){
        if (r.indexIn(f) >= 0) {
          return p.proxy->getQEmacsMajorMode(w,b,t);
        }
      }
    }
    w.displayInformativeMessage(QObject::tr("no major mode for file '%1'").arg(f));
    return nullptr;
  } // end of QEmacsMajorModeFactory::getQEmacsMajorMode

  void QEmacsMajorModeFactory::addQEmacsMajorMode(
      const QEmacsMajorModeProxyPtr proxy,
      const QVector<QRegExp>& e,
      const bool b) {
    Proxy p;
    p.proxy = proxy;
    p.rexp  = e;
    if(b){
      const auto& n = proxy->getName();
      auto& f = QEmacsCommandFactory::getQEmacsCommandFactory();
      auto cp = std::make_shared<QEmacsMajorModeChangeCommandProxy>(n+"-mode",n);
      f.addQEmacsCommand(cp);
    }
    this->proxies.push_front(p);
  } // end of QEmacsMajorModeFactory::getQEmacsMajorMode

  QStringList
  QEmacsMajorModeFactory::getAvailableQEmacsMajorModesNames() const {
    QStringList n;
    for(const auto& p : proxies){
      n.push_back(p.proxy->getName());
    }
    return n;
  } // end of QEmacsMajorModeFactory::getAvailableQEmacsMajorModesNames() const

  QEmacsMajorModeFactory&
  QEmacsMajorModeFactory::getQEmacsMajorModeFactory() {
    static QEmacsMajorModeFactory m;
    return m;
  } // end of QEmacsMajorModeFactory::~QEmacsMajorModeFactory()

  QEmacsMajorModeFactory::QEmacsMajorModeFactory() = default;
  QEmacsMajorModeFactory::~QEmacsMajorModeFactory() = default;
  
} // end of namespace qemacs

