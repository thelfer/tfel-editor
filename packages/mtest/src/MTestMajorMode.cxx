/*!
 * \file  MTestMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/08/2012
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QTextCursor>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"
#include "QEmacs/QEmacsStandardFunctionCommand.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/MTestStudyOptions.hxx"
#include "QEmacs/MTestSyntaxHighlighter.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

#include "QEmacs/ImportBehaviour.hxx"
#include "QEmacs/MTestMajorMode.hxx"

namespace qemacs {

  static void runMTest(QEmacsWidget& qemacs,
                       QEmacsBuffer& b,
                       QEmacsTextEditBase& t,
                       const QString& scheme) {
    const auto n = t.getCompleteFileName();
    if (n.isEmpty()) {
      qemacs.displayInformativeMessage(QObject::tr("no file name"));
      return;
    }
    MTestStudyOptions o;
    MTestStudyOptionsDialog od(o, &t);
    if (od.exec() == QDialog::Rejected) {
      return;
    }
    QFileInfo fn(n);
    if ((!fn.exists()) || (!fn.isFile())) {
      qemacs.displayInformativeMessage(
          QObject::tr("invalid file name"));
      return;
    }
    auto nf = new ProcessOutputFrame(qemacs, b);
    b.attachSecondaryTask(QObject::tr("MTest output"), nf);
    auto& p = nf->getProcess();
    p.setWorkingDirectory(fn.dir().absolutePath());
    auto arg = QStringList{};
    arg << ("--scheme=" + scheme) << ("--verbose=" + o.vlvl);
    if (!o.res) {
      arg << "--result-file-output=no";
    }
    if (o.xml) {
      arg << "--xml-output=true";
    }
    arg << fn.absoluteFilePath();
    p.start("mtest", arg);
    p.waitForStarted();
  }  // end of runMTest

  static void startMTest(QEmacsWidget& qemacs,
                         QEmacsBuffer& b,
                         QEmacsTextEditBase& t,
                         const QString& scheme) {
    if (t.isModified()) {
      auto *input = t.getSaveInput();
      QObject::connect(input, &QEmacsTextEditBase::SaveInput::finished,
                       [&qemacs, &b, &t, scheme] {
                         runMTest(qemacs, b, t, scheme);
                       });
      qemacs.setUserInput(input);
      return;
    }
    runMTest(qemacs, b, t, scheme);
  } // end of  startMTest

  MTestMajorMode::MTestMajorMode(QEmacsWidget& w,
                                 QEmacsBuffer& b,
                                 QEmacsTextEditBase& t)
      : CxxMajorMode(w, b, t) {
    QStringList keys;
    for (const auto& k : this->getKeyWordsList()) {
      keys << QString::fromStdString(k);
    }
    this->c = new QCompleter(keys, &t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,
                     static_cast<void (QCompleter::*)(const QString&)>(
                         &QCompleter::activated),
                     &t, &QEmacsTextEditBase::insertCompletion);
    // creating actions
    this->ra = new QAction(QObject::tr("Run mtest"), this);
    QObject::connect(this->ra, &QAction::triggered, this, [this] {
      startMTest(this->qemacs, this->buffer, this->textEdit,
                 this->getScheme());
    });
    this->iba = new QAction(QObject::tr("Import Behaviour"), this);
    QObject::connect(this->iba, &QAction::triggered, this,
                     &MTestMajorMode::showImportBehaviourWizard);
    this->tpa =
        new QAction(QObject::tr("Plot results using TPlot"), this);
    QObject::connect(this->tpa, &QAction::triggered, this,
                     &MTestMajorMode::showResults);
  }  // end of MTestMajorMode::MTestMajorMode

  QString MTestMajorMode::getName() const {
    return "MTest";
  }  // end of MTestMajorMode::getName

  QString MTestMajorMode::getDescription() const {
    return "major mode dedicated to the mtest code";
  }  // end of CppMajorMode

  bool MTestMajorMode::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      startMTest(this->qemacs,this->buffer,this->textEdit,this->getScheme());
      return true;
    }
    return false;
  }  // end of MTestMajorMode::keyPressEvent

  QMenu* MTestMajorMode::getSpecificMenu() {
    auto* t = qobject_cast<QWidget*>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    auto* m = new QMenu(this->getName(), t);
    m->addAction(this->ra);
    m->addAction(this->tpa);
    m->addSeparator();
    m->addAction(this->iba);
    m->addSeparator();
    auto* km = m->addMenu(QObject::tr("Keywords"));
    auto keys = this->getKeyWordsList();
    std::sort(keys.begin(), keys.end());
    QChar ks;
    for (const auto& k : keys) {
      const auto qk = QString::fromStdString(k);
      if (qk.size() >= 2) {
        if (ks != qk[1]) {
          km->addSeparator();
        }
        ks = qk[1];
        auto* ka = new QAction(qk, this);
        ka->setData(qk);
        km->addAction(ka);
      }
    }
    QObject::connect(km, &QMenu::triggered, this,
                     &MTestMajorMode::insertKeyword);
    QObject::connect(km, &QMenu::triggered, this,
                     &MTestMajorMode::insertKeyword);
    return m;
  }

  void MTestMajorMode::insertKeyword(QAction* a) {
    const auto k = a->data().toString();
    if (k.isEmpty()) {
      return;
    }
    auto tc = this->textEdit.textCursor();
    tc.insertText(k + ' ');
    this->textEdit.setTextCursor(tc);
  }

  void MTestMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    new MTestSyntaxHighlighter(d);
  }  // end of MTestMajorMode::setSyntaxHighlighter

  void MTestMajorMode::completeContextMenu(QMenu* const m,
                                           const QTextCursor& tc) {
    const auto& keys = this->getKeyWordsList();
    QEmacsMajorModeBase::completeContextMenu(m, tc);
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    b.select(QTextCursor::LineUnderCursor);
    const auto l = b.selectedText();
    QRegExp r("^(@\\w+)");
    if (r.indexIn(l) >= 0) {
      const auto k = r.cap(1);
      if (std::find(keys.begin(), keys.end(), k.toStdString()) !=
          keys.end()) {
        delete this->ha;
        this->ha = new QAction(QObject::tr("Help on %1").arg(k), this);
        this->ha->setData(k);
        const auto cactions = m->actions();
        if (cactions.isEmpty()) {
          m->addAction(this->ha);
        } else {
          m->insertSeparator(*(cactions.begin()));
          m->insertAction(*(cactions.begin()), this->ha);
        }
        QObject::connect(m, &QMenu::triggered, this,
                         &MTestMajorMode::actionTriggered);
      }
    }
  }

  const std::vector<std::string>& MTestMajorMode::getKeyWordsList()
      const {
    return MTestSyntaxHighlighter::getMTestKeys();
  }

  QString MTestMajorMode::getScheme() const { return "mtest"; }

  void MTestMajorMode::actionTriggered(QAction* a) {
    if (a == this->ha) {
      const auto k = this->ha->data().toString();
      auto nf = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.attachSecondaryTask(QObject::tr("help on '%1'").arg(k), nf);
      auto& p = nf->getProcess();
      if (p.state() != QProcess::Running) {
        p.start("mtest", QStringList()
                             << ("--scheme=" + this->getScheme())
                             << ("--help-keyword=" + k));
        p.waitForStarted();
        p.waitForFinished(1000);
      }
      nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    }
  }

  QCompleter* MTestMajorMode::getCompleter() {
    return this->c;
  }  // end of getCompleter

  QString MTestMajorMode::getCompletionPrefix() {
    auto tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    if (!tc.atBlockStart()) {
      tc.movePosition(QTextCursor::PreviousCharacter,
                      QTextCursor::KeepAnchor);
      if (tc.selectedText() == "@") {
        return '@' + this->textEdit.getCurrentWord();
      }
    }
    return CxxMajorMode::getCompletionPrefix();
  }

  void MTestMajorMode::showImportBehaviourWizard() {
    ImportBehaviour w(this->textEdit);
    if (w.exec() == QDialog::Accepted) {
    }
  }  // end of MTestMajorMode::showImportBehaviourWizard

  void MTestMajorMode::showResults() {
    const auto n = this->textEdit.getCompleteFileName();
    if (n.isEmpty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("no file name"));
      return;
    };
    const auto f = "mtest:" + n.mid(0, n.lastIndexOf('.')) + ".res";
    if (!QProcess::startDetached("tplot", QStringList() << f)) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("launching tplot failed"));
    }
  }  // end of MTestMajorMode::showResults

  MTestMajorMode::~MTestMajorMode() = default;

  static void runMTest(QEmacsWidget &qemacs) {
    auto& b = qemacs.getCurrentBuffer();
    auto &t = b.getMainFrame();
    startMTest(qemacs, b, t, "mtest");
  }  // end of runMTest

  static void runPTest(QEmacsWidget &qemacs) {
    auto& b = qemacs.getCurrentBuffer();
    auto &t = b.getMainFrame();
    startMTest(qemacs, b, t, "ptest");
  }  // end of runPTest

  static QEmacsStandardFunctionCommandProxy<runMTest> runMTestProxy(
      "run-mtest");
  static QEmacsStandardFunctionCommandProxy<runPTest> runPTestProxy(
      "run-ptest");

  static StandardQEmacsMajorModeProxy<MTestMajorMode> proxy(
      "MTest", QVector<QRegExp>() << QRegExp("^[\\w-]+\\.mtest"));

}  // end of namespace qemacs
