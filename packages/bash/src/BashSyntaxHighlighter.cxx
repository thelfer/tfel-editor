/*!
 * \file   BashSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   5/01/2018
 */

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include "TFEL/GUI/BashSyntaxHighlighter.hxx"

namespace tfel::gui {

  static QStringList callCompgen(const QString& a) {
    QProcess p;
    p.start("/usr/bin/env", QStringList() << "bash"
                                          << "-c"
                                          << "compgen " + a);
    if (!p.waitForFinished()) {
      return {};
    }
    if (p.exitStatus() != QProcess::NormalExit) {
      return {};
    }
    QTextStream stream(p.readAllStandardOutput());
    QStringList r;
    while (!stream.atEnd()) {
      r << stream.readLine();
    }
    return r;
  }  // end of callCompgen

  BashSyntaxHighlighter::BashSyntaxHighlighter(QTextDocument* const t)
      : ShSyntaxHighlighter(t) {
  }  // end of BashSyntaxHighlighter::BashSyntaxHighlighter

  const QStringList& BashSyntaxHighlighter::getCommandsList() {
    static QStringList commands(callCompgen("-c"));
    return commands;
  }  // end of BashSyntaxHighlighter::getCommandsList()

  const QStringList& BashSyntaxHighlighter::getKeywordsList() {
    static QStringList keys(callCompgen("-k"));
    return keys;
  }  // end of BashSyntaxHighlighter::getKeywordsList

  const QStringList& BashSyntaxHighlighter::getBuildInKeywordsList() {
    static QStringList buildins(callCompgen("-b"));
    return buildins;
  }  // end of BashSyntaxHighlighter::getBuildInKeywordsList

  const QStringList& BashSyntaxHighlighter::getFunctionsList() {
    static QStringList functions(callCompgen("-A function"));
    return functions;
  }  // end of BashSyntaxHighlighter::getFunctionsList

  BashSyntaxHighlighter::~BashSyntaxHighlighter() = default;

}  // end of namespace tfel::gui
