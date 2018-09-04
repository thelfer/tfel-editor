/*!
 * \file  GnuplotMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/09/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/ProcessInteractionFrame.hxx"
#include "TFEL/GUI/GnuplotSyntaxHighlighter.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel{

  namespace gui{

  /*!
   * A major mode to handle the gnuplot utility
   */
  struct GnuplotMajorMode final : public MajorModeBase {
    GnuplotMajorMode(EditorWidget& w,
                     Buffer& b,
                     TextEditBase& t)
        : MajorModeBase(w, b, t, &t),
          go(new ProcessInteractionFrame(w, b)) {
      QFileInfo fn(t.getCompleteFileName());
      QDir d(fn.dir());
      auto& p = go->getProcess();
      if (d.exists()) {
        p.setWorkingDirectory(d.absolutePath());
      } else {
        p.setWorkingDirectory(QDir::current().absolutePath());
      }
      p.start("gnuplot", QStringList());
      b.attachSecondaryTask("* gnuplot *", go);
    }

    QString getName() const override {
      return "Gnuplot";
    }  // end of LicosMajorMode

    QString getDescription() const override {
      return "major mode dedicated to gnuplot";
    }  // end of getDescription

    void setSyntaxHighlighter(QTextDocument* d) override {
      new GnuplotSyntaxHighlighter(d);
    }  // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent*) override { return false; }

    bool keyPressEvent(QKeyEvent*) override { return false; }

    void format() override {}

    void comment() override {}

    bool handleShortCut(const int k1,
                        const Qt::KeyboardModifiers m,
                        const int k2) override {
      if ((k1 == Qt::Key_C) && (k2 == Qt::Key_L) &&
          (m == Qt::ControlModifier)) {
        auto c = this->textEdit.textCursor();
        c.select(QTextCursor::LineUnderCursor);
        const auto l = c.selectedText();
        auto& p = this->go->getProcess();
        p.write((l + "\n").toLatin1());
        return true;
      }
      return false;
    }  // end of MajorModeBase::handleShortCut

    ~GnuplotMajorMode() override = default;

    ProcessInteractionFrame* go;

  };  // end of GnuplotMajorMode

  static StandardMajorModeProxy<GnuplotMajorMode> proxy(
      "Gnuplot",
      QVector<QRegExp>() << QRegExp("^[\\w-\\.0-9]+\\.gp")
                         << QRegExp("^[\\w-\\.0-9]+\\.gnuplot"));

}  // end of namespace gui
}// end of namespace tfel
