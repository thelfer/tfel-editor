/*!
 * \file  TextEditQueryReplace.cxx
 * \brief
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#include <QtCore/QSettings>

#include <QtGui/QTextDocument>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/SearchPattern.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/TextEditQueryReplace.hxx"
#include "TFEL/GUI/TextEditReplaceFilter.hxx"

namespace tfel::gui {

  TextEditQueryReplace::TextEditQueryReplace(TextEditBase& t,
                                             EditorWidget& p,
                                             const bool r)
      : CommandLine("", p), textEdit(t), regexp(r), stage(0) {
    const auto h = TextEditQueryReplace::getHistory(r);
    if (h.length() < 2) {
      this->setLabel((r) ? QObject::tr("query replace regexp :")
                         : QObject::tr("query replace :"));
    } else {
      const int hpos = h.length() - 2;
      this->setLabel((r) ? QObject::tr("query replace regexp (default %1 "
                                       "with %2) :")
                             .arg(h[hpos], h.back())
                         : QObject::tr("query replace (default %1 with %2) :")
                               .arg(h[hpos], h.back()));
    }
    this->input->setInputHistory(h);
  }  // end of TextEditQueryReplace::TextEditQueryReplace

  TextEditQueryReplace::TextEditQueryReplace(TextEditBase& t,
                                             EditorWidget& p,
                                             const QString& s1_,
                                             const bool r)
      : CommandLine((r) ? QObject::tr("query replace regexp %1 with :").arg(s1_)
                        : QObject::tr("query replace %1 with :").arg(s1_),
                    p),
        textEdit(t),
        s1(s1_),
        regexp(r),
        stage(1) {
    this->input->setInputHistory(TextEditQueryReplace::getHistory(r));
  }  // end of TextEditQueryReplace::TextEditQueryReplace

  TextEditQueryReplace::TextEditQueryReplace(TextEditBase& t,
                                             EditorWidget& p,
                                             const QString& s1_,
                                             const QString& s2_,
                                             const bool r)
      : CommandLine((r) ? QObject::tr("query replacing regexp %1 with %2 "
                                      "(type 'y', 'n' or '!')")
                            .arg(s1_, s2_)
                        : QObject::tr("query replacing %1 with %2 "
                                      "(type 'y', 'n' or '!')")
                              .arg(s1_, s2_),
                    p),
        textEdit(t),
        s1(s1_),
        regexp(r),
        stage(2) {
    auto* rf = new TextEditReplaceFilter(this->editor, this->textEdit, *this,
                                         SearchPattern(s1_, r, true, false, s2_));
    QObject::connect(rf, &TextEditReplaceFilter::destroyed, this,
                     &TextEditQueryReplace::userEditingFinished);
    if (!this->textEdit.setKeyPressEventFilter(rf)) {
      // the buffer holds no match, or a filter is already installed: the filter
      // says which, and taking it away ends this input, since the filter is what
      // the input is connected to
      rf->deleteLater();
    } else {
      this->input->setFocusPolicy(Qt::NoFocus);
      this->input->setReadOnly(true);
      this->textEdit.setFocus();
    }
  }  // end of TextEditQueryReplace::TextEditQueryReplace

  TextEditQueryReplace::~TextEditQueryReplace() = default;

  void TextEditQueryReplace::treatUserInput() {
    auto& t = this->textEdit;
    auto& q = this->editor;
    if (this->stage == 0) {
      const auto i = this->input->text();
      if (i.isEmpty()) {
        const auto h = TextEditQueryReplace::getHistory(this->regexp);
        if (h.length() < 2) {
          this->cancel();
          this->editor.displayInformativeMessage(QObject::tr("empty entry"));
          return;
        } else {
          const auto hpos = h.length() - 2;
          const auto i1 = h[hpos];
          const auto i2 = h.back();
          this->editor.removeUserInput(this);
          this->editor.setUserInput(new TextEditQueryReplace(t, q, i1, i2,
                                                             this->regexp));
          this->textEdit.setFocus();
          return;
        }
      } else {
        if (this->regexp) {
          /* an answer which is no expression is no question about a buffer:
             the next stage would answer it with a buffer holding no match and
             the reader would be left looking for the parenthesis they forgot in
             a message which says nothing of it */
          const SearchPattern answer(i, true, true, false);
          if (!answer.isValid()) {
            this->editor.displayInformativeMessage(answer.errorString());
            this->editor.removeUserInput(this);
            /* the question is asked again holding what was typed: the mistake
               is a character left out or one too many, and an empty field would
               have the reader write the whole expression again */
            auto* const again = new TextEditQueryReplace(t, q, this->regexp);
            again->input->setText(i);
            this->editor.setUserInput(again);
            return;
          }
        }
        TextEditQueryReplace::addToHistory(i, this->regexp);
        this->editor.removeUserInput(this);
        this->editor.setUserInput(
            new TextEditQueryReplace(t, q, i, this->regexp));
        return;
      }
    } else if (this->stage == 1) {
      const auto i1 = this->s1;
      const auto i2 = this->input->text();
      TextEditQueryReplace::addToHistory(i2, this->regexp);
      this->editor.removeUserInput(this);
      this->editor.setUserInput(
          new TextEditQueryReplace(t, q, i1, i2, this->regexp));
      this->textEdit.setFocus();
      return;
    }
  }  // end of TextEditQueryReplace::treatUserInput

  void TextEditQueryReplace::addToHistory(const QString& i, const bool r) {
    QSettings settings;
    const auto address = (r) ? QStringLiteral("user input/query replace "
                                              "regexp/history")
                             : QStringLiteral("user input/query replace/history");
    auto h = settings.value(address).toStringList();
    h.append(i);
    if (h.size() > 100) {
      h.pop_front();
    }
    settings.setValue(address, h);
  }  // end of TextEditQueryReplace::addToHistory

  QStringList TextEditQueryReplace::getHistory(const bool r) {
    QSettings settings;
    return settings
        .value((r) ? QStringLiteral("user input/query replace regexp/history")
                   : QStringLiteral("user input/query replace/history"))
        .toStringList();
  }  // end of TextEditQueryReplace::getHistory

}  // end of namespace tfel::gui
