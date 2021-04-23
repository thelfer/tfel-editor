/*!
 * \file   CMakeSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   5/01/2018
 */

#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/CMakeSyntaxHighlighter.hxx"

namespace tfel {

  namespace gui {

    static QStringList buildCMakeCommandsList() {
      QProcess p;
      p.start("cmake", QStringList() << "--help-command-list");
      if (!p.waitForFinished()) {
        return {};
      }
      if (p.exitStatus() != QProcess::NormalExit) {
        return {};
      }
      QTextStream stream(p.readAllStandardOutput());
      QStringList commands;
      while (!stream.atEnd()) {
        commands << stream.readLine();
      }
      return commands;
    }  // end of buildCMakeCommandsList

    const QStringList& CMakeSyntaxHighlighter::getCMakeCommandsList() {
      static QStringList commands(buildCMakeCommandsList());
      return commands;
    }  // end of getCMakeCommandsLists

    CMakeSyntaxHighlighter::CMakeSyntaxHighlighter(QTextDocument* const t)
        : QSyntaxHighlighter(t) {
      // parsing options
      this->options.bKeepCommentBoundaries = true;
      this->options.treatPreprocessorDirectives = false;
      this->options.allowStrayBackSlash = true;
      this->options.allowStrayHashCharacter = true;
      this->options.treatStrings = true;
      this->options.treatCComments = false;
      this->options.treatCxxComments = false;
      this->options.graveAccentAsSeparator = true;
      this->options.joinCxxTwoCharactersSeparators = false;
      this->options.dotAsSeparator = false;
      this->options.treatHashCharacterAsCommentDelimiter = true;
      // formating options
      this->commandFormat1.setForeground(Qt::blue);
      this->commandFormat2.setForeground(Qt::darkBlue);
      this->commentFormat.setForeground(Qt::red);
      this->stringFormat.setForeground(Qt::darkGreen);
    }  // end of CMakeSyntaxHighlighter::CMakeSyntaxHighlighter

    void CMakeSyntaxHighlighter::highlightBlock(const QString& l) {
      const auto& cmds = CMakeSyntaxHighlighter::getCMakeCommandsList();
      auto tl = l.trimmed();
      if (tl.isEmpty()) {
        return;
      }
      tfel::utilities::CxxTokenizer tokens(this->options);
      try {
        tokens.parseString(l.toStdString());
      } catch (std::exception& e) {
        debug("CMakeSyntaxHighlighter::highlightBlock:", e.what());
        return;
      } catch (...) {
        debug("CMakeSyntaxHighlighter::highlightBlock: unknown exception");
        return;
      }
      if (tokens.empty()) {
        return;
      }
      if ((tokens.size() > 2) && (tokens[1].value == "(")) {
        const auto cmd = QString::fromStdString(tokens[0].value).toLower();
        if (cmds.contains(cmd)) {
          this->setFormat(tokens[0].offset, tokens[0].value.size(),
                          this->commandFormat1);
        } else {
          this->setFormat(tokens[0].offset, tokens[0].value.size(),
                          this->commandFormat2);
        }
      }
      for (const auto& t : tokens) {
        if (t.flag == tfel::utilities::Token::String) {
          this->setFormat(t.offset, t.value.size(), this->stringFormat);
        } else if (t.flag == tfel::utilities::Token::Comment) {
          this->setFormat(t.offset, t.value.size(), this->commentFormat);
        }
      }
    }  // end of CMakeSyntaxHighlighter::highlightBlock

    CMakeSyntaxHighlighter::~CMakeSyntaxHighlighter() = default;

  }  // end of namespace gui
}  // end of namespace tfel
