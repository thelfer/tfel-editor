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

  // issue #65: callCompgen runs on the GUI thread, inside the first highlight
  // pass of any bash buffer, so its wait must stay bounded: a bash blocking on
  // its startup files (NFS-mounted home, .bashrc reading a terminal) would
  // otherwise freeze the editor with no message and no escape. Measured
  // durations on this machine: compgen -c 58 ms, -k 3 ms, -b 7 ms,
  // -A function 2 ms; 2 s leaves a wide margin.
  static const int compgen_timeout = 2000;

  static QStringList callCompgen(const QString& a) {
    QProcess p;
    p.start("/usr/bin/env", QStringList() << "bash"
                                          << "-c"
                                          << "compgen " + a);
    if (!p.waitForFinished(compgen_timeout)) {
      qWarning() << "BashSyntaxHighlighter: 'compgen" << a
                 << "' did not finish within" << compgen_timeout
                 << "ms: the list stays empty";
      p.kill();
      p.waitForFinished(500);
      return {};
    }
    if (p.exitStatus() != QProcess::NormalExit) {
      qWarning() << "BashSyntaxHighlighter: 'compgen" << a
                 << "' did not exit normally: the list stays empty";
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
