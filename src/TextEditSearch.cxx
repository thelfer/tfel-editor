/*!
 * \file  TextEditSearch.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include <QtCore/QDebug>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/TextEditSearch.hxx"

namespace tfel {

  namespace gui {

    TextEditSearchFilter::TextEditSearchFilter(TextEditBase& t,
                                               TextEditSearch& p)
        : TextEditKeyPressEventFilter(t), textEdit(t), s(p) {
      QObject::connect(&p, &TextEditSearch::destroyed, this,
                       &TextEditSearchFilter::lineEditDestroyed);
    }

    void TextEditSearchFilter::lineEditDestroyed() { this->deleteLater(); }

    bool TextEditSearchFilter::filterKeyPressEvent(QKeyEvent* const e) {
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
          ((k == Qt::Key_Up) || (k == Qt::Key_Down) || (k == Qt::Key_Left) ||
           (k == Qt::Key_Right) || (k == Qt::Key_Return))) {
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
    }  // end of TextEditSearchFilter::filterKeyPressEvent

    bool TextEditSearchFilter::isOk() const {
      return true;
    }  // end of TextEditSearch::isOk

    TextEditSearchFilter::~TextEditSearchFilter() = default;

    TextEditSearch::TextEditSearch(TextEditBase& t,
                                   EditorWidget& p,
                                   const QTextDocument::FindFlags f)
        : CommandLine("", p),
          textEdit(t),
          sf(new TextEditSearchFilter(t, *this)) {
      this->cursor = this->textEdit.textCursor();
      QObject::connect(this->sf, &TextEditSearchFilter::destroyed, this,
                       &TextEditSearch::searchFilterDestroyed);
      QObject::connect(this, &TextEditSearch::textChanged, this,
                       &TextEditSearch::search);
      this->textEdit.setKeyPressEventFilter(this->sf);
      this->setSearchOptions(f);
    }

    void TextEditSearch::search(const QString& s) {
      this->textEdit.setTextCursor(this->cursor);
      if (!this->textEdit.find(s, this->getSearchOptions())) {
        this->editor.displayInformativeMessage("failing search");
      }
    }  // end of TextEditSearch::search

    void TextEditSearch::searchFilterDestroyed() {
      this->sf = nullptr;
      this->editor.removeUserInput(this);
    }  // end of TextEditSearch::searchFilterDestroyed

    void TextEditSearch::findNext() {
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
        this->editor.displayInformativeMessage("failed search");
      }
    }  // end of TextEditSearch::updateCursor

    void TextEditSearch::treatUserInput() {}

    QTextDocument::FindFlags TextEditSearch::getSearchOptions() const {
      return this->opts;
    }  // end of TextEditSearch::getSearchOptions

    void TextEditSearch::setSearchOptions(const QTextDocument::FindFlags o) {
      this->opts = o;
      if (this->opts & QTextDocument::FindBackward) {
        this->setLabel("i-search backward :");
      } else {
        this->setLabel("i-search :");
      }
    }  // end of TextEditSearch::setSearchOptions

    TextEditSearch::~TextEditSearch() = default;

  }  // end of namespace gui
}  // end of namespace tfel
