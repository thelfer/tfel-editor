/*!
 * \file   BehaviourDescription.hxx
 * \brief
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#ifndef LIB_QEMACS_BEHAVIOURDESCRIPTION_HXX
#define LIB_QEMACS_BEHAVIOURDESCRIPTION_HXX

#include <memory>
#include <vector>
#include <QtCore/QString>
#include "MTest/Behaviour.hxx"
#include "QEmacs/MaterialPropertyDescription.hxx"

namespace qemacs {

  //! \brief structure describing a behaviour
  struct BehaviourDescription {
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_BEHAVIOURDESCRIPTION_HXX */
