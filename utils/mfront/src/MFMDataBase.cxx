/*!
 * \file   MFMDataBase.cxx
 * \brief
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#include <stdexcept>
#include <QtCore/QDir>
#include <QtGui/QStandardItemModel>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "MFront/LibraryDescription.hxx"
#include "MFront/TargetsDescription.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/MFMDataBase.hxx"

namespace tfel::gui {

  MFMDataBase::MFMDataBase() = default;

  const QStringList& MFMDataBase::getMaterialPropertiesInterfaces() const {
    return this->mpInterfaces;
  }  // end of MFMDataBase::getMaterialPropertiesInterfaces

  const QStringList& MFMDataBase::getBehavioursInterfaces() const {
    return this->bInterfaces;
  }  // end of MFMDataBase::getBehavioursInterfaces

  const QStringList& MFMDataBase::getModelsInterfaces() const {
    return this->mInterfaces;
  }  // end of MFMDataBase::getModelsInterfaces

  QAbstractItemModel* MFMDataBase::load(QObject* const p) {
    auto* const m = new QStandardItemModel(0, 5, p);
    m->setHeaderData(0, Qt::Horizontal, QObject::tr("Type"));
    m->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    m->setHeaderData(2, Qt::Horizontal, QObject::tr("Material"));
    m->setHeaderData(3, Qt::Horizontal, QObject::tr("Library"));
    m->setHeaderData(4, Qt::Horizontal, QObject::tr("Interface"));
    auto loadImplementations = [this, &m](const char* const env) {
      using tfel::system::ExternalLibraryManager;
      auto& elm = ExternalLibraryManager::getExternalLibraryManager();
      auto add = [this, &m, &elm](const std::string& l, const std::string& e) {
        const auto type = elm.getMaterialKnowledgeType(l, e);
        const auto mkt = [type] {
          if (type == 0) {
            return "MaterialProperty";
          } else if (type == 1) {
            return "Behaviour";
          }
          if (type != 2) {
            throw(std::runtime_error("invalid type"));
          }
          return "Model";
        }();
        const auto mat = elm.getMaterial(l, e);
        const auto i = QString::fromStdString(elm.getInterface(l, e));
        m->insertRow(0);
        m->setData(m->index(0, 0), mkt);
        m->setData(m->index(0, 1), QString::fromStdString(e));
        m->setData(m->index(0, 2), QString::fromStdString(mat));
        m->setData(m->index(0, 3), QString::fromStdString(l));
        m->setData(m->index(0, 4), i);
        if ((type == 0) && (!this->mpInterfaces.contains(i))) {
          this->mpInterfaces.append(i);
        } else if ((type == 1) && (!this->bInterfaces.contains(i))) {
          this->bInterfaces.append(i);
        } else if ((type == 2) && (!this->mInterfaces.contains(i))) {
          this->mInterfaces.append(i);
        }
      };
      const auto mfm = ::getenv(env);
      if (mfm == nullptr) {
        debug("MFMDataBase::load: MFMHOME is not defined");
        return;
      }
#ifdef Q_OS_WIN
      const auto paths = QString(mfm).split(";", Qt::SkipEmptyParts);
#else  /* Q_OS_WIN */
      const auto paths = QString(mfm).split(":", Qt::SkipEmptyParts);
#endif /* Q_OS_WIN */
      for (const auto& path : paths) {
#ifdef Q_OS_WIN
        QDir d(path + "\\bin");
#else  /* Q_OS_WIN */
        QDir d(path + "/lib");
#endif /* Q_OS_WIN */
        debug("MFMDataBase::load: trying ", d.absolutePath());
        if (!d.exists()) {
          continue;
        }
        const auto td = mfront::TargetsDescription();
        const auto prefix = mfront::LibraryDescription::getDefaultLibraryPrefix(
            td.system, td.libraryType);
        const auto suffix = mfront::LibraryDescription::getDefaultLibrarySuffix(
            td.system, td.libraryType);
        for (const auto& ei : d.entryInfoList()) {
          if (!ei.isFile()) {
            debug("MFMDataBase::load:", ei.absoluteFilePath(), "is not a file");
            continue;
          }
          if ((!ei.baseName().startsWith(prefix)) || (ei.suffix() != suffix)) {
            debug("MFMDataBase::load:", ei.absoluteFilePath(),
                  "has not the prefix (", prefix, ") and/or the suffix (",
                  suffix, ") of a shared library");
            continue;
          }
          const auto l = ei.absoluteFilePath().toStdString();
          try {
            debug("MFMDataBase::load: looking in ", ei.absoluteFilePath());
            for (const auto& ept : elm.getEntryPoints(l)) {
              debug("MFMDataBase::load: treating '",
                    QString::fromStdString(ept), "' in ",
                    ei.absoluteFilePath());
              try {
                add(l, ept);
              } catch (std::exception& e) {
                debug("MFMDataBase::load: failed to load '",
                      QString::fromStdString(ept), "' in ",
                      ei.absoluteFilePath());
              } catch (...) {
                // ignoring the error...
              }
            }
          } catch (std::exception& e) {
            debug("MFMDataBase::load: failed to analyse ",
                  ei.absoluteFilePath());
          } catch (...) {
            // ignoring the error...
          }
        }
      }
    };
    // loading
    loadImplementations("MFMHOME");
    loadImplementations("MFRONTGALLERYHOME");
    return m;
  }  // end of MFMDataBase::load

  MFMDataBase::~MFMDataBase() = default;

}  // end of namespace tfel::gui
