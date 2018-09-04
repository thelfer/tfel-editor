/*!
 * \file  LicosBlock.cxx
 * \brief
 * \author Helfer Thomas
 * \date   17/09/2012
 */

#include"TFEL/GUI/LicosBlock.hxx"

namespace tfel{

  namespace gui
{

  static QStringList buildParameterKeys()
  {
    return QStringList() << "Double" << "Int"
			 << "Array"  << "Map"
			 << "Bool"   << "Real"
			 << "String"
			 << "Dictionnary";
  }
  
  const QStringList& getParameterKeys()
  {
    static QStringList l(buildParameterKeys());
    return l;
  } // end of getParameterKeys
  
  LicosBlock::~LicosBlock() = default;

} // end of namespace gui
}// end of namespace tfel
