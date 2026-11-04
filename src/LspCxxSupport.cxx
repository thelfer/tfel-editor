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

    // the name CMake writes the compilation database under, and the prefix of
    // the directories an out-of-source build is usually configured in
    const char *const compilation_database = "compile_commands.json";

    //! the value of `--compile-commands-file`, see the accessor's documentation
    QString &default_compile_commands_file() {
      static QString value;
      return value;
    }  // end of default_compile_commands_file()

    //! whether a directory holds a `CMakeLists.txt`, the marker of the project
    bool holdsCMakeListsFile(const QString &directory) {
      return QFileInfo::exists(directory + '/' + "CMakeLists.txt");
    }  // end of holdsCMakeListsFile()

    /*!\brief the database of a `build*` sub-directory of a directory
     * \param[in] directory : the directory to look into
     * \return empty when no sub-directory named `build*` holds one
     */
    QString databaseInBuildSubDirectory(const QString &directory) {
      const auto names = QDir(directory).entryList(
          {"build*"}, QDir::Dirs | QDir::NoDotAndDotDot | QDir::CaseSensitive);
      // the entry list is alphabetical, which is as good an order as any when
      // several build directories coexist; `build` is simply tried first
      auto candidates = QStringList();
      if (names.contains("build")) {
        candidates << "build";
      }
      for (const auto &name : names) {
        if (name != "build") {
          candidates << name;
        }
      }
      for (const auto &name : candidates) {
        const auto file = directory + '/' + name + '/' + compilation_database;
        if (QFileInfo::exists(file)) {
          return QFileInfo(file).absoluteFilePath();
        }
      }
      return QString();
    }  // end of databaseInBuildSubDirectory()

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

  QStringList LspCxxSupport::compileCommandsArguments(const QString &path,
                                                      QString &error) {
    error = QString();
    const auto trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
      // an empty answer is an answer: the plain session, in which clangd
      // searches a database the way it does when run in a terminal
      return QStringList();
    }
    auto expanded = trimmed;
    if (expanded.startsWith("~/")) {
      // the line edit of a file path expands a leading '~' when it completes
      // an answer, not when the answer is given
      expanded =
          QDir::home().absolutePath() + QDir::separator() + expanded.mid(2);
    }
    const auto info = QFileInfo(expanded);
    if (!info.exists()) {
      error = QStringLiteral("no compilation database at '%1'").arg(expanded);
      return QStringList();
    }
    // a file is only interesting for the directory it lives in, which is where
    // clangd expects to find the `compile_commands.json` it reads
    const auto directory =
        info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    return QStringList() << (QStringLiteral("--compile-commands-dir=") +
                             directory);
  }  // end of compileCommandsArguments()

  void LspCxxSupport::setDefaultCompileCommandsFile(const QString &file) {
    default_compile_commands_file() = file.trimmed();
  }  // end of setDefaultCompileCommandsFile()

  QString LspCxxSupport::defaultCompileCommandsFile() {
    return default_compile_commands_file();
  }  // end of defaultCompileCommandsFile()

  QString LspCxxSupport::searchCompileCommandsFile(const QString &directory) {
    if (directory.trimmed().isEmpty()) {
      return QString();
    }
    // the starting directory is read as it is given, with or without a
    // trailing separator; a file is searched as if the search started from the
    // directory it lives in
    const auto info = QFileInfo(directory);
    const auto start =
        info.isFile() ? info.absolutePath() : QDir(directory).absolutePath();
    if (start.isEmpty()) {
      return QString();
    }
    // first pass: the directory itself, then the parents which are still part
    // of the project, that is whose own parent holds a `CMakeLists.txt`
    auto current = start;
    while (!current.isEmpty()) {
      const auto file = current + '/' + compilation_database;
      if (QFileInfo::exists(file)) {
        return QFileInfo(file).absoluteFilePath();
      }
      const auto parent = QFileInfo(current).absolutePath();
      if (parent == current || !holdsCMakeListsFile(parent)) {
        break;
      }
      current = parent;
    }
    // second pass: the project has ended, `current` is the last directory
    // which was part of it and `parent` is outside of it; look for a `build*`
    // sub-directory of either, which is where an out-of-source build lives
    const auto parent = QFileInfo(current).absolutePath();
    for (const auto &dir : {current, parent}) {
      const auto found = databaseInBuildSubDirectory(dir);
      if (!found.isEmpty()) {
        return found;
      }
    }
    return QString();
  }  // end of searchCompileCommandsFile()

  bool LspCxxSupport::isCxxSourceFile(const QString &file_name) {
    if (file_name.isEmpty()) {
      return false;
    }
    return cxx_suffixes().contains(QFileInfo(file_name).suffix());
  }  // end of LspCxxSupport::isCxxSourceFile()

}  // end of namespace tfel::gui
