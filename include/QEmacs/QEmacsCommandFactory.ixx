/*! 
 * \file  QEmacsCommandFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_QEMACSCOMMANDFACTORY_IXX_
#define _LIB_QEMACS_QEMACSCOMMANDFACTORY_IXX_ 

#include<QtCore/QMap>

#include<QtCore/QString>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/System/ExternalLibraryManager.hxx"

namespace qemacs
{

    template<typename T>
    StandardQEmacsCommandProxy<T>::StandardQEmacsCommandProxy(const QString& n,
							      const bool b)
      : name(n)
    {
      if(b){
	QEmacsCommandFactory& f = QEmacsCommandFactory::getQEmacsCommandFactory();
	f.addQEmacsCommand(std::shared_ptr<QEmacsCommandProxy>(new StandardQEmacsCommandProxy(n,false)));
      }
    } // end of StandardQEmacsCommandProxy<T>::StandardQEmacsCommandProxy

    template<typename T>
    QString
    StandardQEmacsCommandProxy<T>::getName(void) const
    {
      return this->name;
    } // end of StandardQEmacsCommandProxy<T>::getName(void) const

    template<typename T>
    QEmacsCommand *
    StandardQEmacsCommandProxy<T>::getQEmacsCommand(QEmacsWidget& w) const
    {
      return new T(w);
    } // end of StandardQEmacsCommandProxy<T>::getQEmacsCommand

    template<typename T>
    StandardQEmacsCommandProxy<T>::~StandardQEmacsCommandProxy()
    {} // end of StandardQEmacsCommandProxy::~StandardQEmacsCommandProxy

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSCOMMANDFACTORY_IXX_ */
