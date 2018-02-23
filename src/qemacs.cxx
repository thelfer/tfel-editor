/*! 
 * \file  qemacs.cxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#include<cstdlib>
#include<iostream>
#include<stdexcept>
#include<QtCore/QDir>
#include<QtCore/QDebug>
#include<QtCore/QStringList>
#include<QtWidgets/QApplication>
#include"TFEL/System/ExternalLibraryManager.hxx"
#include"QEmacs/VerboseLevel.hxx"
#include"QEmacs/QEmacsBuildInformation.hxx"
#include"QEmacs/QEmacsMainWindow.hxx"

static void loadExternalPackages()
{
  using namespace qemacs;
  using namespace tfel::system;
  QStringList paths;
  // trying the qemacs load path
  auto * userFiles = ::getenv("QEMACS_LOAD_PATH");
  if(userFiles!=nullptr){
#ifdef Q_OS_WIN
    paths << QString(userFiles).split(";",QString::SkipEmptyParts);
#else  /* Q_OS_WIN */
    paths << QString(userFiles).split(":",QString::SkipEmptyParts);
#endif /* Q_OS_WIN */
  }
  // add the standard path
  paths << QEmacsBuildInformation::getDataDirectory();
  for(const auto& p : paths){
    QDir d(p); 
    if(d.exists()){
#ifdef _WIN32
      const auto libs = d.entryList(QStringList("*.dll"),QDir::Files);
#else
      const auto libs = d.entryList(QStringList("*.so"),QDir::Files);
#endif /* WIN32 */
      for(const auto& rl : libs){
	const auto l = (p+QDir::separator()+rl).toStdString();
	try{
	  auto& lm = ExternalLibraryManager::getExternalLibraryManager();
	  lm.loadLibrary(l);
	} catch(std::exception& e){
	  std::cerr << "loading library : '" << l << "' failed ('"
	       << e.what() << "')" << std::endl;
	  std::exit(EXIT_FAILURE);
	}
      }
    }
  }
}

int main(int argc,char ** const argv)
{
  QApplication a(argc,argv);
  a.setApplicationName("qemacs");
  a.setApplicationVersion(PACKAGE_VERSION); 
  a.setOrganizationName("CEA");
  loadExternalPackages();
  QStringList files;
  for(int i=1;i!=argc;++i){
    if (argv[i][0] != '-') {
      files.append(argv[i]);
    } else if (strcmp(argv[i], "--debug") == 0) {
      qemacs::setVerboseMode(qemacs::VERBOSE_DEBUG);
    }
  }
  qemacs::QEmacsMainWindow qemacs(files);
  qemacs.show();
  return QApplication::exec();
}
