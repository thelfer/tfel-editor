/*! 
 * \file  qemacs.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 juin 2012
 */

#include<cstdlib>
#include<iostream>
#include<stdexcept>

#include<QtCore/QDir>
#include<QtCore/QDebug>
#include<QtCore/QStringList>
#ifdef QEMACS_QT4
#include<QtGui/QApplication>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QApplication>
#endif /* QEMACS_QT5 */


#include"TFEL/System/ExternalLibraryManager.hxx"

#include"QEmacs/QEmacsBuildInformation.hxx"
#include"QEmacs/QEmacsMainWindow.hxx"

static void
loadExternalPackages()
{
  using namespace std;
  using namespace qemacs;
  using namespace tfel::system;
  QStringList paths;
  QStringList::const_iterator p;
  // trying the qemacs load path
  const char * userFiles = ::getenv("QEMACS_LOAD_PATH");
  if(userFiles!=nullptr){
    paths << QString(userFiles).split(":",QString::SkipEmptyParts);
  }
  // add the standard path
  paths << QEmacsBuildInformation::getDataDirectory();
  for(p=paths.begin();p!=paths.end();++p){
    QDir d(*p); 
    if(d.exists()){
      QStringList libs = d.entryList(QStringList("*.so"),QDir::Files);
      QStringList::const_iterator pl;
      for(pl=libs.begin();pl!=libs.end();++pl){
	const string& l = (*p+QDir::separator ()+*pl).toStdString();
	try{
	  ExternalLibraryManager& lm = ExternalLibraryManager::getExternalLibraryManager();
	  lm.loadLibrary(l);
	} catch(exception& e){
	  cerr << "loading library : '" << l << "' failed ('"
	       << e.what() << "')" << endl;
	  exit(EXIT_FAILURE);
	}
      }
    }
  }
}

int main(int argc,char ** const argv)
{
  using namespace qemacs;
  QApplication a(argc,argv);
  a.setApplicationName("qemacs");
  a.setApplicationVersion(PACKAGE_VERSION); 
  a.setOrganizationName("CEA");
  loadExternalPackages();
  QStringList files;
  for(int i=1;i!=argc;++i){
    if(argv[i][0]!='-'){
      files.append(argv[i]);
    }
  }
  
  QEmacsMainWindow qemacs(files);
  qemacs.show();
  return QApplication::exec();
}
