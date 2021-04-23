/*!
 * \file  ProcessInteractionFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \date   01/08/2012
 */

#ifndef LIB_TFEL_GUI_PROCESSINTERACTIONFRAME_HXX
#define LIB_TFEL_GUI_PROCESSINTERACTIONFRAME_HXX

#include <QtCore/QProcess>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"

namespace tfel {

  namespace gui {

    /*!
     * \brief an helper class to interact with a QProcess
     * and displaying its outpout within a PlainText
     */
    struct TFEL_GUI_VISIBILITY_EXPORT ProcessInteractionFrame
        : public ProcessOutputFrame {
      /*!
       * \param[in] w : the editor widget
       * \param[in] b : the buffer
       */
      ProcessInteractionFrame(EditorWidget&, Buffer&);
      //! destructor
      ~ProcessInteractionFrame();

     private:
      Q_OBJECT
    };  // end of ProcessInteractionFrame

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_PROCESSINTERACTIONFRAME_HXX */
