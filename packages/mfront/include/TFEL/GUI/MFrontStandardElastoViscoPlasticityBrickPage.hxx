/*!
 * \file   MFrontStandardElastoViscoPlasticityBrickPage.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX
#define LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX

#include <QtWidgets/QWizard>

// forward declaration
class QComboBox;

namespace tfel {

  namespace gui {

    // forward declaration
    struct EditorWidget;
    struct TextEditBase;
    struct LineEdit;

    struct MFrontStandardElastoViscoPlasticityBrickPage : public QWizardPage {
      /*!
       * \brief constructor
       * \param[in] w: editor widget
       * \param[in] cd: current document
       * \param[in] p: parent
       */
      MFrontStandardElastoViscoPlasticityBrickPage(EditorWidget &,
                                                   TextEditBase &,
                                                   QWizard *const);

      bool validatePage() override;

      int nextId() const override;
      //! \brief write the output of the wizard
      virtual void write() const;
      //! destructor
      ~MFrontStandardElastoViscoPlasticityBrickPage() override;

     protected:

      QComboBox *const stress_potentials;
      //!\brief document
      TextEditBase &d;

     private:
      Q_OBJECT
    };  // end of struct MFrontStandardElastoViscoPlasticityBrickPage

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX \
          */
