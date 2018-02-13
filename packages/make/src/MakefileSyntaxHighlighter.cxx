/*!
 * \file   MakefileSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   5/01/2018
 */

#include <QtCore/QDebug>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "QEmacs/MakefileSyntaxHighlighter.hxx"

namespace qemacs {

  MakefileSyntaxHighlighter::MakefileSyntaxHighlighter(
      QTextDocument *const t)
      : QSyntaxHighlighter(t) {
    // parsing options
    this->options.treatPreprocessorDirectives = false;
    this->options.allowStrayBackSlash = true;
    this->options.allowStrayHashCharacter = true;
    this->options.treatStrings = false;
    this->options.treatComments = false;
    this->options.graveAccentAsSeparator = true;
    this->options.joinCxxTwoCharactersSeparators = true;
    this->options.dotAsSeparator = false;
    // formating options
    this->targetFormat.setForeground(Qt::darkGray);
    this->commentFormat.setForeground(Qt::red);
  }  // end of MakefileSyntaxHighlighter::MakefileSyntaxHighlighter

  void MakefileSyntaxHighlighter::highlightBlock(const QString &l) {
    auto format = [this](const tfel::utilities::Token& t) {
      this->setFormat(t.offset, t.value.size(), this->targetFormat);
    };
    if (l.isEmpty()) {
      return;
    }
    if (l[0] == '#') {
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
    //     for (const auto& t : tokens) {
    //       qDebug() << QString::fromStdString(t.value);
    //     }
    if ((tokens.size() > 1) && (tokens[1].value == ":")) {
      format(tokens[0]);
    } else if ((tokens.size() > 2) && (tokens[0].value == ".") &&
               (tokens[2].value == ":")) {
      format(tokens[0]);
      format(tokens[1]);
    }
  } // end of MakefileSyntaxHighlighter::highlightBlock

  MakefileSyntaxHighlighter::~MakefileSyntaxHighlighter() = default;
  
} // end of namespace qemacs
