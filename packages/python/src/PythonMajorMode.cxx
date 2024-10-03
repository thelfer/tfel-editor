/*!
 * \file  PythonMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \brief  26/09/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/CSyntaxHighlighterBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel::gui {

  struct PythonSyntaxHighlighter : public CSyntaxHighlighterBase {
    explicit PythonSyntaxHighlighter(QTextDocument* p)
        : CSyntaxHighlighterBase(p) {
      for (const auto key :
           {"and",     "del",    "from",   "not",   "while",  "as",
            "elif",    "global", "or",     "with",  "assert", "else",
            "if",      "pass",   "yield",  "break", "except", "import",
            "print",   "class",  "exec",   "in",    "raise",  "continue",
            "finally", "is",     "return", "def",   "for",    "lambda",
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
  struct PythonMajorMode final : public MajorModeBase {
    PythonMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
        : MajorModeBase(w, b, t, &t) {}

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

  static StandardMajorModeProxy<PythonMajorMode> proxy(
      "Python",
      QVector<QRegularExpression>() << QRegularExpression("^[\\w-]+\\.py$"));

}  // end of namespace tfel::gui
