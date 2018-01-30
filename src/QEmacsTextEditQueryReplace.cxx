/*! 
 * \file  QEmacsTextEditQueryReplace.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 06 août 2012
 */

#include<QtCore/QSettings>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsTextEditReplaceFilter.hxx"
#include"QEmacs/QEmacsTextEditQueryReplace.hxx"

namespace qemacs
{

  QEmacsTextEditQueryReplace::QEmacsTextEditQueryReplace(QEmacsTextEditBase& t,
							 QEmacsWidget& p)
    : QEmacsLineEdit("",p),
      textEdit(t),
      stage(0)
  {
    const auto h = QEmacsTextEditQueryReplace::getHistory();
    if(h.length()<2){
      this->setLabel(QObject::tr("query replace :"));
    } else {
      int hpos = h.length()-2;
      this->setLabel(QObject::tr("query replace (default %1 with %2) :")
		     .arg(h[hpos]).arg(h.back()));
    }
    this->input->setInputHistory(h);
  }

  QEmacsTextEditQueryReplace::QEmacsTextEditQueryReplace(QEmacsTextEditBase& t,
							 QEmacsWidget& p,
							 const QString& s1_)
    : QEmacsLineEdit(QObject::tr("query replace %1 with :").arg(s1_),p),
      textEdit(t),
      s1(s1_),
      stage(1)
  {
    this->input->setInputHistory(QEmacsTextEditQueryReplace::getHistory());
  }
  
  QEmacsTextEditQueryReplace::QEmacsTextEditQueryReplace(QEmacsTextEditBase& t,
							 QEmacsWidget& p,
							 const QString& s1_,
							 const QString& s2_)
    : QEmacsLineEdit(QObject::tr("query replacing %1 with %2 "
				 "(type 'y', 'n' or '!')")
		     .arg(s1_).arg(s2_),p),
      textEdit(t),
      stage(2)
  {
    QEmacsTextEditReplaceFilter *rf;
    auto tc = this->textEdit.textCursor();
    rf = new QEmacsTextEditReplaceFilter(this->qemacs,this->textEdit,
					 *this,s1_,s2_);
    QObject::connect(rf,&QEmacsTextEditReplaceFilter::destroyed,
		     this,&QEmacsTextEditQueryReplace::userEditingFinished);
    if(!this->textEdit.setKeyPressEventFilter(rf)){
      rf->deleteLater();
    } else {
      this->input->setFocusPolicy(Qt::NoFocus);
      this->input->setReadOnly(true);
      this->textEdit.setFocus();
    }
  }

  QEmacsTextEditQueryReplace::~QEmacsTextEditQueryReplace() = default;

  void QEmacsTextEditQueryReplace::treatUserInput()
  {
    auto& t = this->textEdit;
    auto& q = this->qemacs;
    if(this->stage==0){
      QString i = this->input->text();
      if(i.isEmpty()){
	const auto h = QEmacsTextEditQueryReplace::getHistory();
	if(h.length()<2){
	  this->cancel();
	  this->qemacs.displayInformativeMessage(QObject::tr("empty entry"));
	  return;
	} else {
	  int hpos = h.length()-2;
	  QString i1 = h[hpos];
	  QString i2 = h.back();
	  QEmacsTextEditQueryReplace::addToHistory(i2);
	  this->qemacs.removeUserInput(this);
	  this->qemacs.setUserInput(new QEmacsTextEditQueryReplace(t,q,i1,i2));
	  this->textEdit.setFocus();
	  return;
	}
      } else {
	QEmacsTextEditQueryReplace::addToHistory(i);
	this->qemacs.removeUserInput(this);
	this->qemacs.setUserInput(new QEmacsTextEditQueryReplace(t,q,i));
	return;
      }
    } else if(this->stage==1){
      const auto i1 = this->s1;
      const auto i2 = this->input->text();
      QEmacsTextEditQueryReplace::addToHistory(i2);
      this->qemacs.removeUserInput(this);
      this->qemacs.setUserInput(new QEmacsTextEditQueryReplace(t,q,i1,i2));
      this->textEdit.setFocus();
      return;
    }
  } // end of treatUserInput

  void
  QEmacsTextEditQueryReplace::addToHistory(const QString& i)
  {
    QSettings settings;
    QStringList h = settings.value("user input/query replace/history").toStringList();
    h.append(i);
    if(h.size()>100){
      h.pop_front();
    }
    settings.setValue("user input/query replace/history",h);
  }

  QStringList
  QEmacsTextEditQueryReplace::getHistory()
  {
    QSettings settings;
    return settings.value("user input/query replace/history").toStringList();
  } // end of QEmacsTextEditQueryReplace::getHistory

} // end of namespace qemacs


