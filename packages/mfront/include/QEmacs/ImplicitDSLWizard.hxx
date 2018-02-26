/*!
 * \file   ImplicitDSLWizard.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#ifndef LIB_QEMACS_IMPLICITDSLWIZARD_HXX
#define LIB_QEMACS_IMPLICITDSLWIZARD_HXX

#include <QtWidgets/QWizard>

namespace qemacs{

  struct ImplicitDSLWizard : public QWizard {
    //! desctructor
    ~ImplicitDSLWizard() override;

   private:
    Q_OBJECT
  }; // end of struct ImplicitDSLWizard

} // end of namespace qemacs

#endif /* LIB_QEMACS_IMPLICITDSLWIZARD_HXX */
