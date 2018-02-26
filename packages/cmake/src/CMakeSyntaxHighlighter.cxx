/*!
 * \file   CMakeSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   5/01/2018
 */

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "QEmacs/CMakeSyntaxHighlighter.hxx"

namespace qemacs {

  static QStringList buildCMakeCommandsList() {
    QProcess p;
    p.start("cmake", QStringList() << "--help-command-list");
    if(!p.waitForFinished()){
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

  CMakeSyntaxHighlighter::CMakeSyntaxHighlighter(QTextDocument *const t)
      : QSyntaxHighlighter(t) {
    // parsing options
    this->options.treatPreprocessorDirectives = false;
    this->options.allowStrayBackSlash = true;
    this->options.allowStrayHashCharacter = true;
    this->options.treatStrings = true;
    this->options.treatComments = false;
    this->options.graveAccentAsSeparator = true;
    this->options.joinCxxTwoCharactersSeparators = false;
    this->options.dotAsSeparator = false;
    // formating options
    this->commandFormat1.setForeground(Qt::blue);
    this->commandFormat2.setForeground(Qt::darkBlue);
    this->commentFormat.setForeground(Qt::red);
    this->stringFormat.setForeground(Qt::darkGreen);
  }  // end of CMakeSyntaxHighlighter::CMakeSyntaxHighlighter

  void CMakeSyntaxHighlighter::highlightBlock(const QString & l) {
    const auto& cmds = CMakeSyntaxHighlighter::getCMakeCommandsList();
    auto tl = l.trimmed();
    if (tl.isEmpty()) {
      return;
    }
    if (tl[0] == '#') {
      this->setFormat(0, l.length(), this->commentFormat);
      return;
    }
    tfel::utilities::CxxTokenizer tokens(this->options);
    try{
      tokens.parseString(l.toStdString());
    } catch(...){
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
      }
    }
  }  // end of CMakeSyntaxHighlighter::highlightBlock

  CMakeSyntaxHighlighter::~CMakeSyntaxHighlighter() = default;
  
} // end of namespace qemacs
