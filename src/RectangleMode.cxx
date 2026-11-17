/*!
 * \file  RectangleMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   30/09/2012
 */

#include <QtCore/QObject>
#include <QtCore/QSettings>

#include <QtGui/QKeyEvent>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/RectangleEditing.hxx"
#include "TFEL/GUI/RectangleMode.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {

    /*!
     * \brief the input "Ctrl-X r t" uncovers, which asks the string written on
     * every line of the rectangle
     */
    struct InsertRectangle : public CommandLine {
      InsertRectangle(TextEditBase& t, EditorWidget& p)
          : CommandLine(QObject::tr("String rectangle"), p), textEdit(t) {
        static QString e("/rectangle-mode/insert/histroy");
        QSettings s;
        this->setInputHistorySettingAddress(e);
        const auto ch = s.value(e).toStringList();
        if (!ch.isEmpty()) {
          this->defaultEntry = ch.back();
          this->setLabel(QObject::tr("String rectangle (default %1)")
                             .arg(this->defaultEntry));
        }
      }  // end of InsertRectangle

      /*!
       * \brief writes the answer on every line of the rectangle the selection
       * covers, which is the \c string-rectangle command of Emacs
       *
       * \note an empty answer falls back on the last one, which is what the
       * label of the input offers, and a rectangle which can not be written,
       * because the buffer is read only, is reported by the command itself
       */
      void treatUserInput() override {
        QString t = this->input->text();
        if (t.isEmpty()) {
          t = this->defaultEntry;
        }
        RectangleEditing::insertString(this->textEdit, this->editor, t);
      }  // end of treatUserInput

     protected:
      TextEditBase& textEdit;

      QString defaultEntry;

    };  // end of InsertRectangle

  }  // end of anonymous namespace

  struct RectangleMode::RectangleModeLineEdit : CommandLine::CustomLineEdit {
    RectangleModeLineEdit(TextEditBase& t, EditorWidget& q, CommandLine& p)
        : CustomLineEdit(q, p), textEdit(t) {}  // end of RectangleModeLineEdit

    void keyPressEvent(QKeyEvent* ev) override {
      auto* qle = qobject_cast<CommandLine*>(this->parent());
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_G)) {
        if (qle != nullptr) {
          qle->cancel();
        }
        return;
      }
      const auto k = ev->key();
      if ((ev->modifiers() == Qt::NoModifier) &&
          ((k == Qt::Key_T) || (k == Qt::Key_K) || (k == Qt::Key_W) ||
           (k == Qt::Key_Y))) {
        this->setText(QString(QChar(k == Qt::Key_T    ? 't'
                                    : k == Qt::Key_K  ? 'k'
                                    : k == Qt::Key_W  ? 'w'
                                                      : 'y')));
        if (qle != nullptr) {
          qle->userEditingFinished();
        }
        return;
      }
      // every other key is edited as usual (Return, Backspace, the
      // ↑/↓ history, plain characters ...): the previous code neither
      // chained to CustomLineEdit::keyPressEvent nor guarded the call
      // to userEditingFinished(), so any stray key (space, z, an arrow)
      // ended the C-x r command with empty input
      CommandLine::CustomLineEdit::keyPressEvent(ev);
    }

   protected:
    TextEditBase& textEdit;
  };

  RectangleMode::RectangleMode(TextEditBase& t, EditorWidget& p)
      : CommandLine(QObject::tr("Ctrl-X r "), p, false), textEdit(t) {
    this->setLineEdit(new RectangleModeLineEdit(t, p, *this));
  }  // end of RectangleMode::RectangleMode

  bool RectangleMode::isBlocking() const {
    return true;
  }  // end of RectangleMode::isBlocking

  void RectangleMode::treatUserInput() {
    const QString m = this->input->text();
    TextEditBase& t = this->textEdit;
    EditorWidget& q = this->editor;
    if (m == "t") {
      this->editor.removeUserInput(this);
      this->editor.setUserInput(new InsertRectangle(t, q));
      return;
    } else if (m == "k") {
      RectangleEditing::killRectangle(t, q);
    } else if (m == "w") {
      RectangleEditing::copyRectangleAsKill(t, q);
    } else if (m == "y") {
      RectangleEditing::yankRectangle(t, q);
    }
  }  // end of RectangleMode::treatUserInput

}  // end of namespace tfel::gui
