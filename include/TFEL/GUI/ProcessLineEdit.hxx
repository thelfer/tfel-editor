/*!
 * \file   ProcessLineEdit.hxx
 * \brief
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#ifndef LIB_TFEL_GUI_PROCESSLINEEDIT_HXX
#define LIB_TFEL_GUI_PROCESSLINEEDIT_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CommandLine.hxx"

namespace tfel {

  namespace gui {

    /*!
     * An helper class asking the user for a command and displaying the
     * results in a ProcessOutputFrame created as a slave of the current
     * buffer
     */
    struct TFEL_GUI_VISIBILITY_EXPORT ProcessLineEdit : public CommandLine {
      /*!
       * \param[in] l : label
       * \param[in] c : command guess
       * \param[in] m : mode used to display the results
       * \param[in] p : parent
       */
      ProcessLineEdit(const QString &,
                      const QString &,
                      const QString &,
                      EditorWidget &);
      //! destructor
      ~ProcessLineEdit() override;

     protected:
      void treatUserInput() override;

      virtual void run(const QString &, const QString &, const QStringList &);
      //! major mode for the process output frame
      const QString mode;

     private:
      Q_OBJECT
    };  // end of ProcessLineEdit

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_PROCESSLINEEDIT_HXX */
