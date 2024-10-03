/*!
 * \file  ProcessInteractionFrame.hxx
 * \brief
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#ifndef LIB_TFEL_GUI_PROCESSINTERACTIONFRAME_HXX
#define LIB_TFEL_GUI_PROCESSINTERACTIONFRAME_HXX

#include <QtCore/QProcess>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"

namespace tfel::gui {

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

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PROCESSINTERACTIONFRAME_HXX */
