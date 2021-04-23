/*!
 * \file  MajorModeFactory.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include <utility>
#include <stdexcept>

#include <QtCore/QDebug>

#include "TFEL/System/ExternalLibraryManager.hxx"

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/CommandFactory.hxx"
#include "TFEL/GUI/MajorModeChangeCommand.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel {

  namespace gui {

    MajorModeProxy::~MajorModeProxy() = default;

    void MajorModeFactory::loadLibrary(const QString& lib) {
      using namespace tfel::system;
      auto& lm = ExternalLibraryManager::getExternalLibraryManager();
      lm.loadLibrary(lib.toStdString());
    }

    MajorMode* MajorModeFactory::getMajorModeByName(const QString& n,
                                                    EditorWidget& w,
                                                    Buffer& b,
                                                    TextEditBase& t) const {
      for (auto& p : proxies) {
        if (p.proxy->getName() == n) {
          return p.proxy->getMajorMode(w, b, t);
        }
      }
      w.displayInformativeMessage(
          QObject::tr("no major mode named '%1' registred.").arg(n));
      return nullptr;
    }  // end of MajorModeFactory::getMajorModeByName

    bool MajorModeFactory::hasMajorMode(const QString& n) const {
      for (const auto& p : this->proxies) {
        if (p.proxy->getName() == n) {
          return true;
        }
      }
      return false;
    }  // end of MajorModeFactory::hasMajorModeByName

    MajorMode* MajorModeFactory::getMajorModeForFile(const QString& f,
                                                     EditorWidget& w,
                                                     Buffer& b,
                                                     TextEditBase& t) const {
      for (const auto& p : this->proxies) {
        for (const auto& r : p.rexp) {
          if (r.indexIn(f) >= 0) {
            return p.proxy->getMajorMode(w, b, t);
          }
        }
      }
      w.displayInformativeMessage(
          QObject::tr("no major mode for file '%1'").arg(f));
      return nullptr;
    }  // end of MajorModeFactory::getMajorMode

    QString MajorModeFactory::getMajorModeNameForFile(const QString& f) {
      for (const auto& p : this->proxies) {
        for (const auto& r : p.rexp) {
          if (r.indexIn(f) >= 0) {
            return p.proxy->getName();
          }
        }
      }
      return "";
    }  // end of MajorModeFactory::getMajorModeNameForFile

    void MajorModeFactory::addMajorMode(const MajorModeProxyPtr proxy,
                                        const QVector<QRegExp>& e,
                                        const bool b) {
      Proxy p;
      p.proxy = proxy;
      p.rexp = e;
      if (b) {
        const auto& n = proxy->getName();
        auto& f = CommandFactory::getCommandFactory();
        auto cp = std::make_shared<MajorModeChangeCommandProxy>(n + "-mode", n);
        f.addCommand(cp);
      }
      this->proxies.push_front(p);
    }  // end of MajorModeFactory::getMajorMode

    QIcon MajorModeFactory::getMajorModeIcon(const QString& n) const {
      for (const auto& p : proxies) {
        if (p.proxy->getName() == n) {
          return p.proxy->getIcon();
        }
      }
      return QIcon();
    }  // end of  MajorModeFactory::getMajorModeIcon()

    QStringList MajorModeFactory::getAvailableMajorModesNames() const {
      QStringList n;
      for (const auto& p : proxies) {
        n.push_back(p.proxy->getName());
      }
      return n;
    }  // end of
       // MajorModeFactory::getAvailableMajorModesNames()
       // const

    MajorModeFactory& MajorModeFactory::getMajorModeFactory() {
      static MajorModeFactory m;
      return m;
    }  // end of MajorModeFactory::~MajorModeFactory()

    MajorModeFactory::MajorModeFactory() = default;
    MajorModeFactory::~MajorModeFactory() = default;

  }  // end of namespace gui
}  // end of namespace tfel
