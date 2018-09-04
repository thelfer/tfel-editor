/*!
 * \file  TextEditQueryReplace.cxx
 * \brief
 * \author Helfer Thomas
 * \date   06/08/2012
 */

#include <QtCore/QSettings>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/TextEditReplaceFilter.hxx"
#include "TFEL/GUI/TextEditQueryReplace.hxx"

namespace tfel{

  namespace gui{

  TextEditQueryReplace::TextEditQueryReplace(
      TextEditBase& t, EditorWidget& p)
      : CommandLine("", p), textEdit(t), stage(0) {
    const auto h = TextEditQueryReplace::getHistory();
    if (h.length() < 2) {
      this->setLabel(QObject::tr("query replace :"));
    } else {
      int hpos = h.length() - 2;
      this->setLabel(QObject::tr("query replace (default %1 with %2) :")
                         .arg(h[hpos], h.back()));
    }
    this->input->setInputHistory(h);
  }

  TextEditQueryReplace::TextEditQueryReplace(
      TextEditBase& t, EditorWidget& p, const QString& s1_)
      : CommandLine(QObject::tr("query replace %1 with :").arg(s1_),
                       p),
        textEdit(t),
        s1(s1_),
        stage(1) {
    this->input->setInputHistory(
        TextEditQueryReplace::getHistory());
  }

  TextEditQueryReplace::TextEditQueryReplace(
      TextEditBase& t,
      EditorWidget& p,
      const QString& s1_,
      const QString& s2_)
      : CommandLine(QObject::tr("query replacing %1 with %2 "
                                   "(type 'y', 'n' or '!')")
                           .arg(s1_, s2_),
                       p),
        textEdit(t),
        stage(2) {
    auto* rf = new TextEditReplaceFilter(
        this->editor, this->textEdit, *this, s1_, s2_);
    QObject::connect(rf, &TextEditReplaceFilter::destroyed, this,
                     &TextEditQueryReplace::userEditingFinished);
    if (!this->textEdit.setKeyPressEventFilter(rf)) {
      rf->deleteLater();
    } else {
      this->input->setFocusPolicy(Qt::NoFocus);
      this->input->setReadOnly(true);
      this->textEdit.setFocus();
    }
  }

  TextEditQueryReplace::~TextEditQueryReplace() = default;

  void TextEditQueryReplace::treatUserInput() {
    auto& t = this->textEdit;
    auto& q = this->editor;
    if (this->stage == 0) {
      const auto i = this->input->text();
      if (i.isEmpty()) {
        const auto h = TextEditQueryReplace::getHistory();
        if (h.length() < 2) {
          this->cancel();
          this->editor.displayInformativeMessage(
              QObject::tr("empty entry"));
          return;
        } else {
          const auto hpos = h.length() - 2;
          const auto i1 = h[hpos];
          const auto i2 = h.back();
          this->editor.removeUserInput(this);
          this->editor.setUserInput(
              new TextEditQueryReplace(t, q, i1, i2));
          this->textEdit.setFocus();
          return;
        }
      } else {
        TextEditQueryReplace::addToHistory(i);
        this->editor.removeUserInput(this);
        this->editor.setUserInput(
            new TextEditQueryReplace(t, q, i));
        return;
      }
    } else if (this->stage == 1) {
      const auto i1 = this->s1;
      const auto i2 = this->input->text();
      TextEditQueryReplace::addToHistory(i2);
      this->editor.removeUserInput(this);
      this->editor.setUserInput(
          new TextEditQueryReplace(t, q, i1, i2));
      this->textEdit.setFocus();
      return;
    }
  }  // end of treatUserInput

  void TextEditQueryReplace::addToHistory(const QString& i) {
    QSettings settings;
    auto h = settings.value("user input/query replace/history")
                 .toStringList();
    h.append(i);
    if (h.size() > 100) {
      h.pop_front();
    }
    settings.setValue("user input/query replace/history", h);
  }

  QStringList TextEditQueryReplace::getHistory() {
    QSettings settings;
    return settings.value("user input/query replace/history")
        .toStringList();
  }  // end of TextEditQueryReplace::getHistory

}  // end of namespace gui
}// end of namespace tfel
