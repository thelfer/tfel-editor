/*!
 * \file   ShellProcessLineEdit.hxx
 * \brief
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#ifndef LIB_TFEL_GUI_SHELLPROCESSLINEEDIT_HXX
#define LIB_TFEL_GUI_SHELLPROCESSLINEEDIT_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/ProcessLineEdit.hxx"

namespace tfel {

  namespace gui {

    /*!
     * An helper class asking the user for a shell command and
     * displaying the results in a ProcessOutputFrame created as a slave
     * of the current buffer
     */
    struct TFEL_GUI_VISIBILITY_EXPORT ShellProcessLineEdit
        : public ProcessLineEdit {
      /*!
       * \param[in] l : label
       * \param[in] c : command guess
       * \param[in] m : mode used to display the results
       * \param[in] p : parent
       */
      ShellProcessLineEdit(const QString &,
                           const QString &,
                           const QString &,
                           EditorWidget &);
      //! destructor
      ~ShellProcessLineEdit() override;

     protected:
      void treatUserInput() override;

     private:
      Q_OBJECT

    };  // end of ShellProcessLineEdit

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_SHELLPROCESSLINEEDIT_HXX */
