/*!
 * \file  PythonMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief  26/09/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "QEmacs/Debug.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/QEmacsMajorMode.hxx"
#include "QEmacs/QEmacsMajorModeBase.hxx"
#include "QEmacs/CSyntaxHighlighterBase.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs {

  struct PythonSyntaxHighlighter : public CSyntaxHighlighterBase {
    explicit PythonSyntaxHighlighter(QTextDocument* p)
        : CSyntaxHighlighterBase(p) {
      for (const auto key :
           {"and",    "del",    "from",   "not",      "while",
            "as",     "elif",   "global", "or",       "with",
            "assert", "else",   "if",     "pass",     "yield",
            "break",  "except", "import", "print",    "class",
            "exec",   "in",     "raise",  "continue", "finally",
            "is",     "return", "def",    "for",      "lambda",
            "try"}) {
        HighlightingRule rule;
        rule.key = key;
        rule.format = this->keyFormat;
        highlightingRules.push_back(rule);
      }
      this->options.allowStrayHashCharacter = true;
      this->options.treatPreprocessorDirectives = false;
      this->options.treatCComments = false;
      this->options.treatCxxComments = false;
      this->options.charAsString = true;
      this->options.treatHashCharacterAsCommentDelimiter = true;
    }  // end of PythonSyntaxHighlighter
  };   // end of struct PythonSyntaxHighlighter

  /*!
   * \brief A major mode for the `Python` programming language
   */
  struct PythonMajorMode final : public QEmacsMajorModeBase {
    PythonMajorMode(QEmacsWidget& w,
                    QEmacsBuffer& b,
                    QEmacsTextEditBase& t)
        : QEmacsMajorModeBase(w, b, t, &t) {}

    QString getName() const override {
      return "Python";
    }  // end of LicosMajorMode

    QString getDescription() const override {
      return "major mode dedicated to the python programming language";
    }  // end of getDescription

    void setSyntaxHighlighter(QTextDocument* d) override {
      new PythonSyntaxHighlighter(d);
    }  // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent*) override { return false; }

    bool keyPressEvent(QKeyEvent*) override { return false; }

    void format() override {}

    void comment() override {}

    ~PythonMajorMode() override = default;

  };  // end of PythonMajorMode

  static StandardQEmacsMajorModeProxy<PythonMajorMode> proxy(
      "Python", QVector<QRegExp>() << QRegExp("^[\\w-]+\\.py$"));

}  // end of namespace qemacs
