/*!
 * \file   BehaviourDescription.hxx
 * \brief description of a behaviour of a library or of a gallery.
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

namespace tfel::gui {

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
     *
     * \note the interfaces which need an entry point per modelling hypothesis,
     * that is the `Abaqus`, `AbaqusExplicit` and `Ansys` ones and their
     * aliases, get the suffix \c mtest adds to the entry point; this only
     * happens when the editor was built with the support of the corresponding
     * interface, and the entry point is returned as is for the other interfaces
     *
     * \note an empty string is returned when the hypothesis is unknown or the
     * description can not be analysed
     */
    QString getFunction() const;
    /*!
     * \brief generate an instance of the mtest::Behaviour class based
     * on the information holded by the structure.
     *
     * \return the behaviour, or a null pointer when the description is
     * incomplete, the hypothesis is unknown or the library can not be loaded
     *
     * \note the material properties of the structure are not used: the
     * behaviour is instantiated with default parameters, which is enough to
     * know its material properties and how to insert it in a buffer
     */
    std::shared_ptr<mtest::Behaviour> generate() const;
    //! \brief library in which the behaviour is implemented
    QString library;
    //! \brief entry point associated with the behaviour
    QString behaviour;
    /*!
     * \brief interface
     * \note this can not be called `interface` because it is a restricted
     * keyword under `Visual Studio`
     */
    QString minterface;
    //! \brief modelling hypothesis
    QString hypothesis;
    //! \brief material properties description
    std::vector<MaterialPropertyDescription> material_properties;
  };  // end of BehaviourDescription

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_BEHAVIOURDESCRIPTION_HXX */
