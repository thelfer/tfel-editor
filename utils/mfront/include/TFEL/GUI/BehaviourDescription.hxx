/*!
 * \file   BehaviourDescription.hxx
 * \brief
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#ifndef LIB_TFEL_GUI_BEHAVIOURDESCRIPTION_HXX
#define LIB_TFEL_GUI_BEHAVIOURDESCRIPTION_HXX

#include <memory>
#include <vector>
#include <QtCore/QString>
#include "MTest/Behaviour.hxx"
#include "TFEL/GUI/MaterialPropertyDescription.hxx"
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel{

  namespace gui{

  //! \brief structure describing a behaviour
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT BehaviourDescription {
    //! default constructor
    BehaviourDescription();
    //! move constructor
    BehaviourDescription(BehaviourDescription&&);
    //! copy constructor
    BehaviourDescription(const BehaviourDescription&);
    //! move assignement
    BehaviourDescription& operator=(BehaviourDescription&&);
    //! copy assignement
    BehaviourDescription& operator=(const BehaviourDescription&);
    /*!
     * \return the function implementing the behaviour (depends of the
     * entry point, the hypothesis and the interface)
     */
    QString getFunction() const;
    /*!
     * \brief generate an instance of the mtest::Behaviour class based
     * on the information holded by the structure.
     */
    std::shared_ptr<mtest::Behaviour> generate() const;
    //! \brief library in which the library is implemented
    QString library;
    //! \brief entry point associated with the behaviour
    QString behaviour;
    /*!
     * \brief interface
     * \note this can be called `interface` because it is a restricted
     * keyword under `Visual Studio`
     */
    QString minterface;
    //! \brief modelling hypothesis
    QString hypothesis;
    //! \brief material properties description
    std::vector<MaterialPropertyDescription> material_properties;
  };  // end of BehaviourDescription

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_BEHAVIOURDESCRIPTION_HXX */
