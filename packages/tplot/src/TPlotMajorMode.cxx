/*!
 * \file  TPlotMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/09/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <QtGui/QSyntaxHighlighter>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"

#include "QEmacs/QEmacsMajorMode.hxx"
#include "QEmacs/QEmacsMajorModeBase.hxx"
#include "QEmacs/GnuplotSyntaxHighlighter.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs {

  struct TPlotSyntaxHighlighter : public GnuplotSyntaxHighlighter {
    TPlotSyntaxHighlighter(QTextDocument* p)
        : GnuplotSyntaxHighlighter(p) {
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
  struct TPlotMajorMode : public QEmacsMajorModeBase {
    TPlotMajorMode(QEmacsWidget& w,
                   QEmacsBuffer& b,
                   QEmacsTextEditBase& t)
        : QEmacsMajorModeBase(w, b, t, &t) {}

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

  static StandardQEmacsMajorModeProxy<TPlotMajorMode> proxy(
      "TPlot",
      QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.tplot"));

}  // end of namespace qemacs
