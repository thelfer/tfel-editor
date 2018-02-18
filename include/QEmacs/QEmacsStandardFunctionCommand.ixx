/*!
 * \file   QEmacsStandardFunctionCommand.ixx
 * \brief    
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_QEMACS_QEMACSSTANDARDFUNCTIONCOMMAND_IXX
#define LIB_QEMACS_QEMACSSTANDARDFUNCTIONCOMMAND_IXX

namespace qemacs {

  template <void (*f)(QEmacsWidget&)>
  QEmacsStandardFunctionCommandProxy<
      f>::QEmacsStandardFunctionCommandProxy(const QString& n,const bool b)
      : name(n) {
    if (b) {
      auto& qcf = QEmacsCommandFactory::getQEmacsCommandFactory();
      qcf.addQEmacsCommand(
          std::make_shared<QEmacsStandardFunctionCommandProxy>(n,false));
    }
  }  // end of
     // QEmacsStandardFunctionCommandProxy::QEmacsStandardFunctionCommandProxy

  template <void (*f)(QEmacsWidget&)>
  QString QEmacsStandardFunctionCommandProxy<f>::getName() const {
    return this->name;
  } // end of QEmacsStandardFunctionCommandProxy::getName

  template <void (*f)(QEmacsWidget&)>
  QEmacsCommand* QEmacsStandardFunctionCommandProxy<f>::getQEmacsCommand(
      QEmacsWidget& w) const {
    return new QEmacsStandardFunctionCommand(w,f);
  }  // end of QEmacsStandardFunctionCommandProxy::getQEmacsCommand

  template <void (*f)(QEmacsWidget&)>
  QEmacsStandardFunctionCommandProxy<
      f>::~QEmacsStandardFunctionCommandProxy() = default;

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSSTANDARDFUNCTIONCOMMAND_IXX */
