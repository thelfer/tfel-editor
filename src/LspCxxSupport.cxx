/*!
 * \file  LspCxxSupport.cxx
 * \brief finding `clangd`, and deciding what a C++ file is.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "TFEL/GUI/LspCxxSupport.hxx"

namespace tfel::gui {

  namespace {

    // the extensions the `C++` major mode is registered for, see the
    // standard mode proxy at the bottom of `CxxMajorMode.cxx`; the set is
    // case sensitive on purpose, `.CC` is registered and `.cc` is not
    const QStringList &cxx_suffixes() {
      static const QStringList s{"cxx", "hxx", "ixx", "cpp", "hpp", "ipp",
                                 "CC",  "hh",  "c",   "h",   "cu"};
      return s;
    }  // end of cxx_suffixes()

    // the highest `clangd-NN` of a directory, empty when there is none; the
    // number is compared as a number, `clangd-9` being older than
    // `clangd-15`, which a string order gets wrong
    QString highestVersioned(const QString &directory, const QString &prefix) {
      auto best = int{-1};
      auto name = QString();
      const auto expression =
          QRegularExpression(QRegularExpression::escape(prefix) + "(\\d+)$");
      for (const auto &entry : QDir(directory).entryList(QDir::Files)) {
        const auto match = expression.match(entry);
        if (match.hasMatch()) {
          const auto version = match.captured(1).toInt();
          if (version > best) {
            best = version;
            name = entry;
          }
        }
      }
      return name;
    }  // end of highestVersioned()

  }  // end of namespace

  bool LspCxxSupport::isEnabled() {
    return QSettings().value("clangd/enable", true).toBool();
  }  // end of LspCxxSupport::isEnabled()

  QString LspCxxSupport::serverExecutable() {
    const auto settings = QSettings();
    const auto configured =
        settings.value("clangd/executable").toString().trimmed();
    if (!configured.isEmpty()) {
      // the user said where it is; whether it exists is the user's business,
      // the transport will report a program which will not start
      return configured;
    }
    const auto plain = QStandardPaths::findExecutable("clangd");
    if (!plain.isEmpty()) {
      return plain;
    }
    // distributions install the versioned name without installing the plain
    // one: the usual places, highest version first
    static const QStringList bins{"/usr/bin", "/usr/local/bin", "/bin"};
    for (const auto &directory : bins) {
      const auto versioned = highestVersioned(directory, "clangd-");
      if (!versioned.isEmpty()) {
        return directory + '/' + versioned;
      }
    }
    auto best = int{-1};
    auto path = QString();
    for (const auto &llvm :
         QDir("/usr/lib").entryList({"llvm-*"}, QDir::Dirs)) {
      const auto suffix = llvm.mid(QStringLiteral("llvm-").size());
      bool ok = false;
      const auto version = suffix.toInt(&ok);
      if (!ok) {
        continue;
      }
      const auto candidate = "/usr/lib/" + llvm + "/bin/clangd";
      if (QFileInfo::exists(candidate) && (version > best)) {
        best = version;
        path = candidate;
      }
    }
    return path;
  }  // end of LspCxxSupport::serverExecutable()

  QStringList LspCxxSupport::serverArguments() {
    return QSettings().value("clangd/arguments").toStringList();
  }  // end of LspCxxSupport::serverArguments()

  bool LspCxxSupport::isCxxSourceFile(const QString &file_name) {
    if (file_name.isEmpty()) {
      return false;
    }
    return cxx_suffixes().contains(QFileInfo(file_name).suffix());
  }  // end of LspCxxSupport::isCxxSourceFile()

}  // end of namespace tfel::gui
