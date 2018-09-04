/*!
 * \file  editor.cxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/GUI/VerboseLevel.hxx"
#include "TFEL/GUI/BuildInformation.hxx"
#include "TFEL/GUI/MainWindow.hxx"

static void loadExternalPackages() {
  using namespace tfel::gui;
  using namespace tfel::system;
  QStringList paths;
  // trying the editor load path
  auto* userFiles = ::getenv("TFEL_EDITOR_LOAD_PATH");
  if (userFiles != nullptr) {
#ifdef Q_OS_WIN
    paths << QString(userFiles).split(";", QString::SkipEmptyParts);
#else  /* Q_OS_WIN */
    paths << QString(userFiles).split(":", QString::SkipEmptyParts);
#endif /* Q_OS_WIN */
  }
  // add the standard path
  paths << BuildInformation::getDataDirectory();
  for (const auto& p : paths) {
    QDir d(p);
    if (d.exists()) {
#ifdef _WIN32
      const auto libs = d.entryList(QStringList("*.dll"), QDir::Files);
#else
      const auto libs = d.entryList(QStringList("*.so"), QDir::Files);
#endif /* WIN32 */
      for (const auto& rl : libs) {
        const auto l = (p + QDir::separator() + rl).toStdString();
        try {
          auto& lm =
              ExternalLibraryManager::getExternalLibraryManager();
          lm.loadLibrary(l);
        } catch (std::exception& e) {
          std::cerr << "loading library : '" << l << "' failed ('"
                    << e.what() << "')" << std::endl;
          std::exit(EXIT_FAILURE);
        }
      }
    }
  }
}

int main(int argc, char** const argv) {
  QApplication a(argc, argv);
  a.setApplicationName("tfel-editor");
  a.setApplicationVersion(PACKAGE_VERSION);
  a.setOrganizationName("CEA");
  loadExternalPackages();
  QStringList files;
  for (int i = 1; i != argc; ++i) {
    if (argv[i][0] != '-') {
      files.append(argv[i]);
    } else if (strcmp(argv[i], "--debug") == 0) {
      tfel::gui::setVerboseMode(tfel::gui::VERBOSE_DEBUG);
    }
  }
  // setting the current locale to "C"
  setlocale(LC_NUMERIC, "C");
  tfel::gui::MainWindow editor(files);
  editor.show();
  return QApplication::exec();
}
