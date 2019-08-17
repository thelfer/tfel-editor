/*!
 * \file   MFrontBehaviourPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTBEHAVIOURPAGE_HXX
#define LIB_TFEL_GUI_MFRONTBEHAVIOURPAGE_HXX

#include <QtWidgets/QWizard>
#include "MFront/BehaviourDescription.hxx"

// forward declaration
class QLabel;
class QComboBox;
class QTextDocument;

namespace tfel {

  namespace gui {

    // forward declaration
    struct EditorWidget;
    struct TextEditBase;
    struct LineEdit;

    struct MFrontBehaviourPage : public QWizardPage {
      //! \brief a simple alias
      using BehaviourDescription = mfront::BehaviourDescription;
      /*!
       * \brief constructor
       * \param[in] w: editor widget
       * \param[in] cd: current document
       * \param[in] p: parent
       */
      MFrontBehaviourPage(EditorWidget &,
                          TextEditBase &,
                          QWizard *const);

      bool validatePage() override;

      int nextId() const override;
      //! \brief write the output of the wizard
      virtual void write() const;
      //! \return the selected DSL
      virtual QString getSelectedDomainSpecificLanguage() const;
      //! \return the selected behaviour type
      BehaviourDescription::BehaviourType getSelectedBehaviourType()
          const;
      //! \return the selected integration scheme
      BehaviourDescription::IntegrationScheme
      getSelectedIntegrationScheme() const;
      //! destructor
      ~MFrontBehaviourPage() override;

     protected:
      /*!
       * \brief update the list of integration schemes according to the
       * selected behaviour type.
       */
      void updateIntegrationSchemeList();
      //! \brief update the list of DSLs
      void updateDSLList();
      //! \brief update the list of algorithms
      void updateAlgorithmList();
      //! \brief update the list of strain measures
      void updateStrainMeasureList();
      //! \brief update the list of bricks
      void updateBrickList();
      //! \brief update the list of elastic properties
      void updateElasticPropertyList();
      //! \brief update the list of tangent operators
      void updateTangentOperatorList();
      //! \brief update widget associated with numerical criteria
      void updateNumericalCriteria();
      //! \brief write the output of the wizard
      virtual void write(QTextDocument&) const;

      //! combox box associated with behaviour type
      QComboBox *bts = nullptr;
      //! combox box associated with integration scheme
      QComboBox *iss = nullptr;
      //! combox box showing available DSLs
      QComboBox *dsls = nullptr;
      //! labBel associated with algorithms
      QLabel *algorithms_label = nullptr;
      //! combox box showing available algorithms
      QComboBox *algorithms = nullptr;
      //! label associated with material hypotheses
      QLabel *hypotheses_label = nullptr;
      //! combox box showing available hypotheses
      QComboBox *hypotheses = nullptr;
      //! label associated with material symmetries
      QLabel *symmetries_label = nullptr;
      //! combox box showing available symmetries
      QComboBox *symmetries = nullptr;
      //! label associated with strain measures
      QLabel *strain_measures_label = nullptr;
      //! combox box showing available strain measures
      QComboBox *strain_measures = nullptr;
      //! label associated with bricks
      QLabel *bricks_label = nullptr;
      //! combox box showing available bricks
      QComboBox *bricks = nullptr;
      //! label associated with elastic properties
      QLabel *elastic_properties_label = nullptr;
      //! combox box showing available elastic properties
      QComboBox *elastic_properties = nullptr;
      //! label associated with crystal structure
      QLabel *crystal_structures_label = nullptr;
      //! combox box showing available crystal structures
      QComboBox *crystal_structures = nullptr;
      //! label associated with tangent operators
      QLabel *tangent_operators_label = nullptr;
      //! combox box showing available tangent operators
      QComboBox *tangent_operators = nullptr;
      //! label associated with the convergence criterion
      QLabel *convergence_criterion_label = nullptr;
      //! editing line for convergence criterion
      LineEdit *convergence_criterion = nullptr;
      //! label associated with the perturbation value
      QLabel *perturbation_value_label = nullptr;
      //! editing line for the perturbation value
      LineEdit *perturbation_value = nullptr;
      //!\brief underlying editor
      EditorWidget& editor;
      //!\brief document
      TextEditBase &d;

     private:
      Q_OBJECT
    };  // end of struct MFrontBehaviourPage

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTBEHAVIOURPAGE_HXX */
