/*!
 * \file  TPlotMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/09/2012
 */

#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/GnuplotSyntaxHighlighter.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel::gui {

  struct TPlotSyntaxHighlighter : public GnuplotSyntaxHighlighter {
    TPlotSyntaxHighlighter(QTextDocument* p) : GnuplotSyntaxHighlighter(p) {
      for (const auto& k : {"import"}) {
        HighlightingRule rule;
        rule.key = k;
        rule.format = this->keyFormat;
        highlightingRules.push_back(rule);
      }
    }  // end of TPlotSyntaxHighlighter

  };  // end of struct TPlotSyntaxHighlighter

  /*!
   * A major mode to handle the tplot utility
   */
  struct TPlotMajorMode : public MajorModeBase {
    TPlotMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
        : MajorModeBase(w, b, t, &t) {}

    QString getName() const override {
      return "tplot";
    }  // end of LicosMajorMode

    QString getDescription() const override {
      return "major mode dedicated to tplot";
    }  // end of getDescription

    void setSyntaxHighlighter(QTextDocument* d) override {
      new TPlotSyntaxHighlighter(d);
    }  // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent*) override { return false; }

    bool keyPressEvent(QKeyEvent*) override { return false; }

    void format() override {}

    void comment() override {}

    ~TPlotMajorMode() override = default;

  };  // end of TPlotMajorMode

  static StandardMajorModeProxy<TPlotMajorMode> proxy(
						      "TPlot", QVector<QRegularExpression>() << QRegularExpression("^" + fileNameRegExp() + "\\.tplot"));

}  // end of namespace tfel::gui
