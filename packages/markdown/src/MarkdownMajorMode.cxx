/*!
 * \file  MarkdownMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 août 2012
 */

#include <stdexcept>
#include <QtCore/QDebug>
#include <QtCore/QStringListModel>
#include <QtCore/QTimer>
#include <QtWidgets/QAbstractItemView>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "QEmacs/MarkdownSyntaxHighlighter.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/MarkdownMajorMode.hxx"

namespace qemacs {

  MarkdownMajorMode::MarkdownMajorMode(QEmacsWidget &w,
                                       QEmacsBuffer &b,
                                       QEmacsTextEditBase &t)
      : CompiledLanguageMajorModeBase(w, b, t) {
    this->rm = new QAction(QObject::tr("Run pandoc"), this);
  } // end of MarkdownMajorMode::MarkdownMajorMode

  QString MarkdownMajorMode::getName() const {
    return "Markdown";
  } // end of MarkdownMajorMode::getName

  QString MarkdownMajorMode::getDescription() const {
    return "major mode dedicated to the Markdown language";
  } // end of CppMajorMode

  void MarkdownMajorMode::format()
  {} // end of MarkdownMajorMode::format()
  
  QMenu *MarkdownMajorMode::getSpecificMenu() {
    auto *t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    QMenu *m(new QMenu(QObject::tr("Markdown"), t));
    m->addAction(this->rm);
    return m;
  } // end of

  QCompleter *MarkdownMajorMode::getCompleter() {
    return nullptr;
  } // end of MarkdownMajorMode::getCompleter

  QString MarkdownMajorMode::getCompletionPrefix() {
    return "";
  } // end of MarkdownMajorMode::getCompletionPrefix

  QIcon MarkdownMajorMode::getIcon() const {
    return QIcon::fromTheme("x-office-document");
  } // end of MarkdownMajorMode::getIcon()

  void MarkdownMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    new MarkdownSyntaxHighlighter(*this,d);
  } // end of MarkdownMajorMode::setSyntaxHighlighter

  void MarkdownMajorMode::completeContextMenu(QMenu *const m,
                                              const QTextCursor &tc) {
  }  // end of MarkdownMajorMode::completeContextMenu

  void MarkdownMajorMode::actionTriggered(QAction *) {
  } // end of MarkdownMajorMode::actionTriggered

  void MarkdownMajorMode::runPandoc() {
    if (this->textEdit.isModified()) {
      auto *input = this->textEdit.getSaveInput();
      QObject::connect(input, &QEmacsTextEditBase::SaveInput::finished, this,
                       &MarkdownMajorMode::startPandoc);
      this->qemacs.setUserInput(input);
      return;
    }
    this->startPandoc();
  } // end of MarkdownMajorMode::runPandoc()

  QString MarkdownMajorMode::getLanguageName() const{
    return "markdown";
  } // end of MarkdownMajorMode::getLanguageName

  QString MarkdownMajorMode::getDefaultCompilationCommand() const {
    return "pandoc ";
  } // end of MarkdownMajorMode::getDefaultCompilationCommand

  void MarkdownMajorMode::startPandoc(){
    const auto n = this->textEdit.getCompleteFileName();
    if (n.isEmpty()) {
      this->report(QObject::tr("MarkdownMajorMode::startPandoc: no file name"));
      return;
    }
    //     PandocAnalysisOptions o;
    //     MarkdownAnalysisOptionsDialog od(o, &(this->textEdit));
    //     if (od.exec() == QDialog::Rejected) { return; }
    //     const auto &af = QFileInfo(n).absoluteFilePath();
    //     auto *s = new MarkdownOutputFrame(this->qemacs, this->buffer, af, o);
    //     QObject::connect(s, &MarkdownOutputFrame::finished, this,
    //                      &MarkdownMajorMode::AnalysisFinished);
    //     this->buffer.addSlave(QObject::tr("Markdown Output"), s);
  } // end of MarkdownMajorMode::startPandoc

  SpellChecker& MarkdownMajorMode::getSpellChecker() {
    return this->spellChecker;
  } // end of MarkdownMajorMode::getSpellChecker

  MarkdownMajorMode::~MarkdownMajorMode() = default;

  static StandardQEmacsMajorModeProxy<MarkdownMajorMode> proxy(
      "Markdown",
      QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.md"));

} // end of namespace qemacs
