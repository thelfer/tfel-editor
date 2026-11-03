/*!
 * \file   ShSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   5/01/2018
 */

#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/ShSyntaxHighlighter.hxx"

namespace tfel::gui {

  static QStringList buildShCommandsList() {
    QStringList commands;
    return commands;
  }  // end of buildShCommandsList

  ShSyntaxHighlighter::ShSyntaxHighlighter(QTextDocument* const t)
      : QSyntaxHighlighter(t) {
    // parsing options
    this->options.bKeepCommentBoundaries = true;
    this->options.treatPreprocessorDirectives = false;
    this->options.allowStrayBackSlash = true;
    this->options.allowStrayHashCharacter = true;
    this->options.treatStrings = true;
    this->options.charAsString = true;
    this->options.treatCComments = false;
    this->options.treatCxxComments = false;
    this->options.graveAccentAsSeparator = true;
    this->options.joinCxxTwoCharactersSeparators = false;
    this->options.dotAsSeparator = false;
    this->options.treatHashCharacterAsCommentDelimiter = true;
    // formating options
    this->commentFormat.setForeground(Qt::red);
    this->keyFormat.setForeground(Qt::darkBlue);
    this->bKeyFormat.setForeground(Qt::blue);
    this->commandFormat.setForeground(Qt::darkCyan);
    this->functionFormat.setForeground(Qt::darkMagenta);
    this->stringFormat.setForeground(Qt::darkGreen);
  }  // end of ShSyntaxHighlighter::ShSyntaxHighlighter

  const QStringList& ShSyntaxHighlighter::getKeywordsList() {
    static QStringList keys(buildShCommandsList());
    return keys;
  }  // end of ShSyntaxHighlighter::getKeywordsList

  const QStringList& ShSyntaxHighlighter::getBuildInKeywordsList() {
    static QStringList buildins;
    return buildins;
  }  // end of ShSyntaxHighlighter::getBuildInKeywordsList

  const QStringList& ShSyntaxHighlighter::getFunctionsList() {
    static QStringList functions;
    return functions;
  }  // end of ShSyntaxHighlighter::getFunctionsList

  const QStringList& ShSyntaxHighlighter::getCommandsList() {
    static QStringList commands;
    return commands;
  }  // end of ShSyntaxHighlighter::getCommandsList

  void ShSyntaxHighlighter::highlightBlock(const QString& l) {
    if (!this->lookupsInitialized) {
      // issue #65: classifying every token of every block by four linear
      // QStringList::contains() scans costs several ms per keystroke once the
      // bash mode fills the lists with the machine's command table
      // (10511 entries measured); the sets mirror the (virtual) lists and are
      // built once per highlighter, after the derived constructor has run
      const auto& keys = this->getKeywordsList();
      const auto& bkeys = this->getBuildInKeywordsList();
      const auto& fcts = this->getFunctionsList();
      const auto& cmds = this->getCommandsList();
      this->keysSet = QSet<QString>(keys.cbegin(), keys.cend());
      this->bkeysSet = QSet<QString>(bkeys.cbegin(), bkeys.cend());
      this->fctsSet = QSet<QString>(fcts.cbegin(), fcts.cend());
      this->cmdsSet = QSet<QString>(cmds.cbegin(), cmds.cend());
      this->lookupsInitialized = true;
    }
    auto tl = l.trimmed();
    if (tl.isEmpty()) {
      return;
    }
    tfel::utilities::CxxTokenizer tokens(this->options);
    try {
      tokens.parseString(l.toStdString());
    } catch (std::exception& e) {
      debug("ShSyntaxHighlighter::highlightBlock:", e.what());
      return;
    } catch (...) {
      debug("ShSyntaxHighlighter::highlightBlock: unknown exception");
      return;
    }
    if (tokens.empty()) {
      return;
    }
    for (const auto& t : tokens) {
      if (t.flag == tfel::utilities::Token::String) {
        this->setFormat(t.offset, t.value.size(), this->stringFormat);
      } else if (t.flag == tfel::utilities::Token::Comment) {
        this->setFormat(t.offset, t.value.size(), this->commentFormat);
      } else {
        const auto w = QString::fromStdString(t.value);
        if (this->keysSet.contains(w)) {
          this->setFormat(t.offset, t.value.size(), this->keyFormat);
        } else if (this->bkeysSet.contains(w)) {
          this->setFormat(t.offset, t.value.size(), this->bKeyFormat);
        } else if (this->fctsSet.contains(w)) {
          this->setFormat(t.offset, t.value.size(), this->functionFormat);
        } else if (this->cmdsSet.contains(w)) {
          this->setFormat(t.offset, t.value.size(), this->commandFormat);
        }
      }
    }
  }  // end of ShSyntaxHighlighter::highlightBlock

  ShSyntaxHighlighter::~ShSyntaxHighlighter() = default;

}  // end of namespace tfel::gui
