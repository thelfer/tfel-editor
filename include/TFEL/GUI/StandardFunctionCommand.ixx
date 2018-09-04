/*!
 * \file   StandardFunctionCommand.ixx
 * \brief    
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_TFEL_GUI_STANDARDFUNCTIONCOMMAND_IXX
#define LIB_TFEL_GUI_STANDARDFUNCTIONCOMMAND_IXX

namespace tfel{

  namespace gui{

  template <void (*f)(EditorWidget&)>
  StandardFunctionCommandProxy<
      f>::StandardFunctionCommandProxy(const QString& n,const bool b)
      : name(n) {
    if (b) {
      auto& qcf = CommandFactory::getCommandFactory();
      qcf.addCommand(
          std::make_shared<StandardFunctionCommandProxy>(n,false));
    }
  }  // end of
     // StandardFunctionCommandProxy::StandardFunctionCommandProxy

  template <void (*f)(EditorWidget&)>
  QString StandardFunctionCommandProxy<f>::getName() const {
    return this->name;
  } // end of StandardFunctionCommandProxy::getName

  template <void (*f)(EditorWidget&)>
  Command* StandardFunctionCommandProxy<f>::getCommand(
      EditorWidget& w) const {
    return new StandardFunctionCommand(w,f);
  }  // end of StandardFunctionCommandProxy::getCommand

  template <void (*f)(EditorWidget&)>
  StandardFunctionCommandProxy<
      f>::~StandardFunctionCommandProxy() = default;

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_STANDARDFUNCTIONCOMMAND_IXX */
