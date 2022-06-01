/*!
 * \file   MFrontOptionsDialog.hxx
 * \brief
 * \author Thomas Helfer
 * \date   16/03/2018
 */

#ifndef LIB_TFEL_GUI_MFRONTOPTIONSDIALOG_HXX
#define LIB_TFEL_GUI_MFRONTOPTIONSDIALOG_HXX

#include <QtWidgets/QDialog>

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct MFrontOptions;

  //! \brief a dialog used to choose the options passed to `MTest`
  struct MFrontOptionsDialog : public QDialog {
    //! type of material knowlegde treated
    enum MaterialKnowledgeType { MATERIALPROPERTY, BEHAVIOUR, MODEL };
    /*!
     * \param[in] q: editor widget
     * \param[in] o: options to be modified
     * \param[in] t: material knowledge type
     * \param[in] p: parent
     */
    MFrontOptionsDialog(EditorWidget &,
                        MFrontOptions &,
                        const MaterialKnowledgeType,
                        QWidget *const);

   protected:
    MFrontOptions &opts;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_MFRONTOPTIONSDIALOG_HXX */
