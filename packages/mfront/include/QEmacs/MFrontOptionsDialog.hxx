/*!
 * \file   MFrontOptionsDialog.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   16/03/2018
 */

#ifndef LIB_QEMACS_MFRONTOPTIONSDIALOG_HXX
#define LIB_QEMACS_MFRONTOPTIONSDIALOG_HXX

#include<QtWidgets/QDialog>

namespace qemacs{

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct MFrontOptions;

  //! \brief a dialog used to choose the options passed to `MTest`
  struct MFrontOptionsDialog : public QDialog {
    //! type of material knowlegde treated
    enum MaterialKnowledgeType { MATERIALPROPERTY, BEHAVIOUR, MODEL };
    /*!
     * \param[in] q: qemacs widget
     * \param[in] o: options to be modified
     * \param[in] t: material knowledge type
     * \param[in] p: parent
     */
    MFrontOptionsDialog(QEmacsWidget &,
                        MFrontOptions &,
                        const MaterialKnowledgeType,
                        QWidget *const);

   protected:
    MFrontOptions& opts;

   private:
    Q_OBJECT
  };

} // end of namespace qemacs

#endif /* LIB_MFRONTOPTIONSDIALOG_HXX */
