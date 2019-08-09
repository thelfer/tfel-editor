/*!
 * \file   MFrontAddVariablesPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX
#define LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX

#include <QtWidgets/QWizard>

// forward declaration
class QComboBox;

namespace tfel {

  namespace gui {

    struct MFrontAddVariablesPage : public QWizardPage {
      /*!
       * \brief constructor
       * \param[in] w: editor widget
       * \param[in] cd: current document
       * \param[in] p: parent
       */
      MFrontAddVariablesPage(QWizard *const);

      bool validatePage() override;

      int nextId() const override;
      //! \brief write the output of the wizard
      virtual void write() const;
      //! destructor
      ~MFrontAddVariablesPage() override;

     private:
      Q_OBJECT
    };  // end of struct MFrontAddVariablesPage

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX */
