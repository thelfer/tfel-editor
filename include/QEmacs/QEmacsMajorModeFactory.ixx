/*! 
 * \file  QEmacsMajorModeFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_QEMACSMAJORMODEFACTORY_IXX_
#define LIB_QEMACS_QEMACSMAJORMODEFACTORY_IXX_ 

#include<QtCore/QMap>

#include<QtCore/QString>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/System/ExternalLibraryManager.hxx"

namespace qemacs
{

    template<typename T>
    StandardQEmacsMajorModeProxy<T>::StandardQEmacsMajorModeProxy(const QString& n,
								  const QVector<QRegExp>& e,
								  const bool b)
      : name(n),
	rexp(e)
    {
      if(b){
	QEmacsMajorModeFactory& f = QEmacsMajorModeFactory::getQEmacsMajorModeFactory();
	f.addQEmacsMajorMode(std::shared_ptr<QEmacsMajorModeProxy>(new StandardQEmacsMajorModeProxy(n,e,false)),e);
      }
    } // end of StandardQEmacsMajorModeProxy<T>::StandardQEmacsMajorModeProxy

    template<typename T>
    QString
    StandardQEmacsMajorModeProxy<T>::getName() const
    {
      return this->name;
    } // end of StandardQEmacsMajorModeProxy<T>::getName() const

    template<typename T>
    QEmacsMajorMode *
    StandardQEmacsMajorModeProxy<T>::getQEmacsMajorMode(QEmacsWidget& w,
							QEmacsBuffer& b,
							QEmacsTextEditBase& t) const
    {
      return new T(w,b,t);
    } // end of StandardQEmacsMajorModeProxy<T>::getQEmacsMajorMode

    template<typename T>
    StandardQEmacsMajorModeProxy<T>::~StandardQEmacsMajorModeProxy()
    {} // end of StandardQEmacsMajorModeProxy::~StandardQEmacsMajorModeProxy

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSMAJORMODEFACTORY_IXX_ */
