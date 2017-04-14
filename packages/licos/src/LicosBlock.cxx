/*!
 * \file  LicosBlock.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#include"QEmacs/LicosBlock.hxx"

namespace qemacs
{

  static QStringList
  buildParameterKeys()
  {
    return QStringList() << "Double" << "Int"
			 << "Array"  << "Map"
			 << "Bool"   << "Real"
			 << "String"
			 << "Dictionnary";
  }
  
  const QStringList&
  getParameterKeys(void)
  {
    static QStringList l(buildParameterKeys());
    return l;
  } // end of getParameterKeys
  
  LicosBlock::~LicosBlock()
  {} // end of LicosBlock::~LicosBlock

} // end of namespace qemacs
