/*!
 * \file  MakefileMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QSettings>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/MakefileSyntaxHighlighter.hxx"
#include "TFEL/GUI/MakefileMajorMode.hxx"

namespace tfel {

  namespace gui {

    MakefileMajorMode::MakefileMajorMode(EditorWidget& w,
                                         Buffer& b,
                                         TextEditBase& t)
        : MajorModeBase(w, b, t, &t) {
    }  // end of MakefileMajorMode::MakefileMajorMode

    QString MakefileMajorMode::getName() const {
      return "Makefile";
    }  // end of MakefileMajorMode::MakefileMajorMode

    QString MakefileMajorMode::getDescription() const {
      return "major mode dedicated to Makefile";
    }  // end of MakefileMajorMode::MakefileMajorMode

    bool MakefileMajorMode::keyPressEvent(QKeyEvent* const e) {
      const auto k = e->key();
      const auto m = e->modifiers();
      if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
        this->runCompilation();
        return true;
      }
      return false;
    }  // end of MakefileMajorMode::keyPressEvent

    QString MakefileMajorMode::getCommentSyntax() {
      return "#";
    }  // end of MakefileMajorMode::getCommentSyntax

    void MakefileMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
      new MakefileSyntaxHighlighter(d);
    }  // end of MakefileMajorMode::setSyntaxHighlighter

    void MakefileMajorMode::format() {}

    void MakefileMajorMode::runCompilation() {
      QSettings s;
      const auto ch = s.value("make/compilation/history").toStringList();
      const auto d = [&ch]() -> QString {
        if (ch.isEmpty()) {
          return "make -k";
        }
        return ch.back();
      }();
      auto* l = new ShellProcessLineEdit("compilation command :", d,
                                         "compilation-output", this->editor);
      l->setInputHistorySettingAddress("make/compilation/history");
      this->editor.setUserInput(l);
    }  // end of MakefileMajorMode::runCompilation

    MakefileMajorMode::~MakefileMajorMode() = default;

    static StandardMajorModeProxy<MakefileMajorMode> proxy(
        "Makefile",
        QVector<QRegExp>() << QRegExp("^Makefile$")
                           << QRegExp("^Makefile\\.am$"));

  }  // end of namespace gui
}  // end of namespace tfel
