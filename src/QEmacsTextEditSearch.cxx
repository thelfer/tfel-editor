/*!
 * \file  QEmacsTextEditSearch.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include <QtCore/QDebug>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/QEmacsTextEditSearch.hxx"

namespace qemacs {

  QEmacsTextEditSearchFilter::QEmacsTextEditSearchFilter(
      QEmacsTextEditBase& t, QEmacsTextEditSearch& p)
      : QEmacsTextEditKeyPressEventFilter(t), textEdit(t), s(p) {
    QObject::connect(&p, &QEmacsTextEditSearch::destroyed, this,
                     &QEmacsTextEditSearchFilter::lineEditDestroyed);
  }

  void QEmacsTextEditSearchFilter::lineEditDestroyed() { delete this; }

  bool QEmacsTextEditSearchFilter::filterKeyPressEvent(
      QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if ((m == Qt::NoModifier) && (k == Qt::Key_Escape)) {
      this->textEdit.removeKeyPressEventFilter();
      return true;
    }
    if (k == Qt::Key_Control) {
      return true;
    }
    if ((m == Qt::NoModifier) &&
        ((k == Qt::Key_Up) || (k == Qt::Key_Down) ||
         (k == Qt::Key_Left) || (k == Qt::Key_Right) ||
         (k == Qt::Key_Return))) {
      this->textEdit.removeKeyPressEventFilter();
      return false;
    }
    if ((m == Qt::ControlModifier) && (k == Qt::Key_S)) {
      auto o = this->s.getSearchOptions();
      if (o & QTextDocument::FindBackward) {
        o = o & ~QTextDocument::FindBackward;
        this->s.setSearchOptions(o);
        this->s.findNext();
      } else {
        this->s.findNext();
      }
      return true;
    }
    if ((m == Qt::ControlModifier) && (k == Qt::Key_R)) {
      auto o = this->s.getSearchOptions();
      if (o & QTextDocument::FindBackward) {
        this->s.findNext();
      } else {
        o = o | QTextDocument::FindBackward;
        this->s.setSearchOptions(o);
        this->s.findNext();
      }
      return true;
    }
    // if(!stdkey){
    //   this->textEdit.removeKeyPressEventFilter();
    //   return false;
    // }
    this->s.keyPressEvent(e);
    return true;
  }  // end of QEmacsTextEditSearchFilter::filterKeyPressEvent

  bool QEmacsTextEditSearchFilter::isOk() const {
    return true;
  }  // end of QEmacsTextEditSearch::isOk

  QEmacsTextEditSearchFilter::~QEmacsTextEditSearchFilter() = default;

  QEmacsTextEditSearch::QEmacsTextEditSearch(
      QEmacsTextEditBase& t,
      QEmacsWidget& p,
      const QTextDocument::FindFlags f)
      : QEmacsCommandLine("", p),
        textEdit(t),
        sf(new QEmacsTextEditSearchFilter(t, *this)) {
    this->cursor = this->textEdit.textCursor();
    QObject::connect(this->sf, &QEmacsTextEditSearchFilter::destroyed,
                     this,
                     &QEmacsTextEditSearch::searchFilterDestroyed);
    QObject::connect(this, &QEmacsTextEditSearch::textChanged, this,
                     &QEmacsTextEditSearch::search);
    this->textEdit.setKeyPressEventFilter(this->sf);
    this->setSearchOptions(f);
  }

  void QEmacsTextEditSearch::search(const QString& s) {
    this->textEdit.setTextCursor(this->cursor);
    if (!this->textEdit.find(s, this->getSearchOptions())) {
      this->qemacs.displayInformativeMessage("failing search");
    }
  }  // end of QEmacsTextEditSearch::search

  void QEmacsTextEditSearch::searchFilterDestroyed() {
    this->sf = nullptr;
    this->qemacs.removeUserInput(this);
  } // end of QEmacsTextEditSearch::searchFilterDestroyed

  void QEmacsTextEditSearch::findNext() {
    auto c = this->textEdit.textCursor();
    const auto p = c.selectionEnd();
    if (c.hasSelection()) {
      c.setPosition(p, QTextCursor::KeepAnchor);
    } else {
      c.setPosition(p, QTextCursor::MoveAnchor);
    }
    this->textEdit.setTextCursor(c);
    if (this->textEdit.find(this->input->text(), this->getSearchOptions())) {
      this->cursor = c;
    } else {
      this->qemacs.displayInformativeMessage("failed search");
    }
  }  // end of QEmacsTextEditSearch::updateCursor

  void QEmacsTextEditSearch::treatUserInput() {}

  QTextDocument::FindFlags QEmacsTextEditSearch::getSearchOptions() const {
    return this->opts;
  } // end of QEmacsTextEditSearch::getSearchOptions

  void QEmacsTextEditSearch::setSearchOptions(
      const QTextDocument::FindFlags o) {
    this->opts = o;
    if (this->opts&QTextDocument::FindBackward){
      this->setLabel("i-search backward :");
    } else {
      this->setLabel("i-search :");
    }
  } // end of QEmacsTextEditSearch::setSearchOptions

  QEmacsTextEditSearch::~QEmacsTextEditSearch() = default;

}  // end of namespace qemacs
