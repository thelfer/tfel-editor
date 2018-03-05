/*!
 * \file   QEmacsProcessLineEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#include <QtCore/QDir>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/Utilities/StringAlgorithms.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/ProcessOutputMajorModeBase.hxx"
#include "QEmacs/QEmacsProcessLineEdit.hxx"

namespace qemacs {

  QEmacsProcessLineEdit::QEmacsProcessLineEdit(const QString& l,
                                               const QString& c,
                                               const QString& m,
                                               QEmacsWidget& p)
      : QEmacsLineEdit(l, p), mode(m) {
    if (!c.isEmpty()) {
      this->input->setText(c);
    }
  }  // end of QEmacsProcessLineEdit::QEmacsProcessLineEdit

  void QEmacsProcessLineEdit::treatUserInput() {
    const auto c = this->input->text();
    auto args = c.split(" ", QString::SkipEmptyParts);
    if (args.empty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("empty command"));
      return;
    }
    const auto c0 = args.front();
    args.pop_front();
    this->run(c0, c0, args);
    return;
  }

  void QEmacsProcessLineEdit::run(const QString& on,
                                  const QString& c,
                                  const QStringList& args) {
    using namespace tfel::utilities;
    if (c.isEmpty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("empty command"));
      return;
    }
    auto& b = this->qemacs.getCurrentBuffer();
    auto* po = new ProcessOutputFrame(this->qemacs, b);
    const auto dn = [&b] {
      QDir d(b.getMainFrame().getDirectory());
      if(d.exists()){
        return d.absolutePath();
      }
      return QDir::current().absolutePath();
    }();
    auto nargs = QStringList{};
    const auto sdn = dn.toStdString();
    for(const auto& a :args){
      CxxTokenizerOptions opt;
      opt.shallMergeStrings = false;
      opt.allowStrayHashCharacter = true;
      opt.allowStrayBackSlash = true;
      opt.treatPreprocessorDirectives = false;
      opt.treatNumbers = false;
      opt.treatCComments = false;
      opt.treatCxxComments = false;
      opt.joinCxxTwoCharactersSeparators = false;
      opt.graveAccentAsSeparator = true;
      opt.charAsString = true;
      CxxTokenizer tokenizer(opt);
      try{
        tokenizer.parseString(a.toStdString());
        auto ppos = CxxTokenizer::size_type{};
        auto narg = std::string{};
        for (const auto& t : tokenizer) {
          auto value = t.value;
          if (t.flag == Token::Standard) {
            value = replace_all(value, ".", sdn);
            value = replace_all(value, "$(cmd)", sdn);
            value = replace_all(value, "%cmd%", sdn);
          }
          narg += std::string(' ',t.offset - ppos) + value;
          ppos = t.offset + value.size();
        }
        nargs << QString::fromStdString(narg);
      } catch(...){
        nargs << a;
      }
    }
    auto& p = po->getProcess();
    p.setWorkingDirectory(dn);
    if (!this->mode.isEmpty()) {
      auto* m = qobject_cast<ProcessOutputMajorModeBase*>(
          po->setMajorMode(this->mode));
      if (m != nullptr) {
        m->setDirectory(p.workingDirectory());
        m->setCommand(c);
        m->setArguments(args);
        m->setMajorMode(this->mode);
      }
    }
    p.start(c, args);
    b.attachSecondaryTask("*" + on + "* ouput", po);
    b.setSecondaryTaskIcon(po,QIcon::fromTheme("system-run"));
  }  // end of QEmacsProcessLineEdit::run

  QEmacsProcessLineEdit::~QEmacsProcessLineEdit() = default;

}  // end of namespace qemacs
