/*! 
 * \file  Command.hxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#ifndef LIB_TFEL_GUI_COMMAND_HXX
#define LIB_TFEL_GUI_COMMAND_HXX 

#include<QtCore/QObject>
#include"TFEL/GUI/Config.hxx"

namespace tfel{

  namespace gui{

  //! forward declaration
  struct EditorWidget;

  //! \brief abstract class for all editor commands
  struct TFEL_GUI_VISIBILITY_EXPORT Command : public QObject {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    Command(EditorWidget&);
    //! execute the command
    virtual void execute() = 0;
    //! destructor
    virtual ~Command();

   private:
    Q_OBJECT
  }; // end of class Command

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_COMMAND_HXX */
