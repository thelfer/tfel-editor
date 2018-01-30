/*! 
 * \file  QEmacsMajorModeChangeCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 sept. 2012
 */

#include"QEmacs/QEmacsCommand.hxx"
#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"
#include"QEmacs/QEmacsCommandFactory.hxx"

#include"QEmacs/QEmacsMajorModeChangeCommand.hxx"

namespace qemacs
{
  
  QEmacsMajorModeChangeCommand::QEmacsMajorModeChangeCommand(QEmacsWidget& p,
							     const QString& m)
    : QEmacsCommand(p),
      mode(m)
  {} // end of QEmacsMajorModeChangeCommand::QEmacsMajorModeChangeCommand
  
  void
  QEmacsMajorModeChangeCommand::execute()
  {
    auto *p = qobject_cast<QEmacsWidget *>(this->parent());
    if(p!=nullptr){
      QEmacsBuffer& b = p->getCurrentBuffer();
      QEmacsPlainTextEdit& t = b.getMainFrame();
      t.setMajorMode(this->mode);
    }
  } // end of QEmacsMajorModeChangeCommand::execute
    
  QEmacsMajorModeChangeCommand::~QEmacsMajorModeChangeCommand() = default;
   
  QEmacsMajorModeChangeCommandProxy::QEmacsMajorModeChangeCommandProxy(const QString& c,
								       const QString& m)
    :  name(c),
       mode(m)
  {} // end of QEmacsMajorModeChangeCommandProxy::QEmacsMajorModeChangeCommandProxy
  
  QString
  QEmacsMajorModeChangeCommandProxy::getName() const
  {
    return this->name;
  }
  
  QEmacsCommand*
  QEmacsMajorModeChangeCommandProxy::getQEmacsCommand(QEmacsWidget& t) const
  {
    return new QEmacsMajorModeChangeCommand(t,this->mode);
  }

  QEmacsMajorModeChangeCommandProxy::~QEmacsMajorModeChangeCommandProxy() = default;

} // end of namespace qemacs
