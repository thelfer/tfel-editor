/*!
 * \file  MTestMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/08/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtGui/QTextCursor>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"
#include "QEmacs/QEmacsStandardFunctionCommand.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/MTestStudyOptions.hxx"
#include "QEmacs/MTestSyntaxHighlighter.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/ImportBehaviourWizard.hxx"
#include "QEmacs/ImportMFMBehaviourWizard.hxx"
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
      auto* input = t.getSaveInput();
      QObject::connect(input, &QEmacsTextEditBase::SaveInput::finished,
                       [&qemacs, &b, &t, scheme] {
                         runMTest(qemacs, b, t, scheme);
                       });
      qemacs.setUserInput(input);
      return;
    }
    runMTest(qemacs, b, t, scheme);
  }  // end of  startMTest

  static void insertBehaviour(QEmacsTextEditBase& textEdit,
                              const BehaviourDescription& bd) {
    auto b = bd.generate();
    if (!b) {
      return;
    }
    auto tc = textEdit.textCursor();
    tc.beginEditBlock();
    tc.insertText("@ModellingHypothesis '" + bd.hypothesis + "';\n");
    tc.insertText("@Behaviour<" + bd.minterface + "> '" + bd.library +
                  "' '" + bd.getFunction() + "';\n\n");
    if (!b->getMaterialPropertiesNames().empty()) {
      tc.insertText("// material properties\n");
      for (const auto& m : b->getMaterialPropertiesNames()) {
        const auto mp = QString::fromStdString(m);
        const auto pmpd = std::find_if(
            bd.material_properties.begin(),
            bd.material_properties.end(),
            [&mp](const MaterialPropertyDescription& mpd) {
              if (mpd.is<ConstantMaterialPropertyDescription>()) {
                const auto& cmpd =
                    mpd.get<ConstantMaterialPropertyDescription>();
                return mp == cmpd.name;
              } else if (mpd.is<CastemMaterialPropertyDescription>()) {
                const auto& cmpd =
                    mpd.get<CastemMaterialPropertyDescription>();
                return mp == cmpd.name;
              }
              return false;
            });
        if (pmpd != bd.material_properties.end()) {
          if (pmpd->is<ConstantMaterialPropertyDescription>()) {
            const auto& cmpd =
                pmpd->get<ConstantMaterialPropertyDescription>();
            tc.insertText("@MaterialProperty<constant> '" + mp + "' " +
                          QString::number(cmpd.value) + ";\n");
          } else if (pmpd->is<CastemMaterialPropertyDescription>()) {
            const auto& cmpd =
                pmpd->get<CastemMaterialPropertyDescription>();
            tc.insertText("@MaterialProperty<castem> '" + mp + "' " +
                          "'" + cmpd.library + "' '" + cmpd.function +
                          "';\n");
          } else {
            tc.insertText("@MaterialProperty<...> '" + mp + "' ...;\n");
          }
        } else {
          tc.insertText("@MaterialProperty<...> '" + mp + "' ...;\n");
        }
      }
      tc.insertText("\n");
    }
    if (!b->getParametersNames().empty()) {
      tc.insertText("// parameters\n");
      for (const auto& p : b->getParametersNames()) {
        const auto v = b->getRealParameterDefaultValue(p);
        tc.insertText("// @Parameter' " + QString::fromStdString(p) +
                      "' " + QString::number(v) + ";\n");
      }
      tc.insertText("\n");
    }
    if (!b->getIntegerParametersNames().empty()) {
      tc.insertText("// integer parameters\n");
      for (const auto& p : b->getIntegerParametersNames()) {
        const auto v = b->getIntegerParameterDefaultValue(p);
        tc.insertText("// @IntegerParameter' " +
                      QString::fromStdString(p) + "' " +
                      QString::number(v) + ";\n");
      }
      tc.insertText("\n");
    }
    if (!b->getUnsignedShortParametersNames().empty()) {
      tc.insertText("// UnsignedShort parameters\n");
      for (const auto& p : b->getUnsignedShortParametersNames()) {
        const auto v = b->getUnsignedShortParameterDefaultValue(p);
        tc.insertText("// @UnsignedIntegerParameter' " +
                      QString::fromStdString(p) + "' " +
                      QString::number(v) + ";\n");
      }
      tc.insertText("\n");
    }
    if (!b->getInternalStateVariablesNames().empty()) {
      tc.insertText("// internal state variable initialisations\n");
      for (const auto& iv : b->getInternalStateVariablesNames()) {
        tc.insertText("// @InternalStateVariable '" +
                      QString::fromStdString(iv) + "';\n");
      }
      tc.insertText("\n");
    }
    if (!b->getExternalStateVariablesNames().empty()) {
      tc.insertText("// external state variable\n");
      for (const auto& ev : b->getExternalStateVariablesNames()) {
        tc.insertText("// @ExternalStateVariable '" +
                      QString::fromStdString(ev) + "' ... ;\n");
      }
      tc.insertText("\n");
    }
    tc.endEditBlock();
    textEdit.setTextCursor(tc);
  }  // end of insertBehaviour

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
      startMTest(this->qemacs, this->buffer, this->textEdit,
                 this->getScheme());
      return true;
    }
    return CxxMajorMode::keyPressEvent(e);
  }  // end of MTestMajorMode::keyPressEvent

  QMenu* MTestMajorMode::getSpecificMenu() {
    auto* t = qobject_cast<QWidget*>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    auto* m = new QMenu(this->getName(), t);
    // creating actions
    auto* const ra = m->addAction(QObject::tr("Run mtest"));
    QObject::connect(ra, &QAction::triggered, this, [this] {
      startMTest(qemacs, this->buffer, this->textEdit,
                 this->getScheme());
    });
    auto* const tpa =
        m->addAction(QObject::tr("Plot results using TPlot"));
    QObject::connect(tpa, &QAction::triggered, this,
                     &MTestMajorMode::showResults);
    m->addSeparator();
    auto* const iba = m->addAction(QObject::tr("Import Behaviour"));
    QObject::connect(iba, &QAction::triggered, this,
                     &MTestMajorMode::showImportBehaviourWizard);
    auto* const imfmba =
        m->addAction(QObject::tr("Import MFM behaviour"));
    QObject::connect(imfmba, &QAction::triggered, this,
                     &MTestMajorMode::showImportMFMBehaviourWizard);
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
        auto* const ha =
            new QAction(QObject::tr("Help on %1").arg(k), m);
        const auto cactions = m->actions();
        if (cactions.isEmpty()) {
          m->addAction(ha);
        } else {
          m->insertSeparator(*(cactions.begin()));
          m->insertAction(*(cactions.begin()), ha);
        }
        QObject::connect(ha, &QAction::triggered, this, [k, this] {
          auto nf = new ProcessOutputFrame(this->qemacs, this->buffer);
          this->buffer.attachSecondaryTask(
              QObject::tr("help on '%1'").arg(k), nf);
          auto& p = nf->getProcess();
          if (p.state() != QProcess::Running) {
            p.start("mtest", QStringList()
                                 << ("--scheme=" + this->getScheme())
                                 << ("--help-keyword=" + k));
            p.waitForStarted();
            p.waitForFinished(1000);
          }
          nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
        });
      }
    }
  }

  const std::vector<std::string>& MTestMajorMode::getKeyWordsList()
      const {
    return MTestSyntaxHighlighter::getMTestKeys();
  }

  QString MTestMajorMode::getScheme() const { return "mtest"; }

  QCompleter* MTestMajorMode::getCompleter() {
    return this->c;
  }  // end of MTestMajorMode::getCompleter

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
  }  // end of MTestMajorMode::getCompletionPrefix

  void MTestMajorMode::showImportBehaviourWizard() {
    ImportBehaviourWizard w(this->textEdit);
    if (w.exec() == QDialog::Accepted) {
      insertBehaviour(this->textEdit, w.getSelectedBehaviour());
    }
  }  // end of MTestMajorMode::showImportBehaviourWizard

  void MTestMajorMode::showImportMFMBehaviourWizard() {
    using tfel::material::ModellingHypothesis;
    using tfel::system::ExternalLibraryManager;
    ImportMFMBehaviourWizard w(this->textEdit);
    if (w.exec() == QDialog::Accepted) {
      insertBehaviour(this->textEdit, w.getSelectedBehaviour());
    }
  }  // end of MTestMajorMode::showImportMFMBehaviourWizard

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

  void runMTest(QEmacsWidget& qemacs) {
    auto& b = qemacs.getCurrentBuffer();
    auto& t = b.getMainFrame();
    startMTest(qemacs, b, t, "mtest");
  }  // end of runMTest

  void runPTest(QEmacsWidget& qemacs) {
    auto& b = qemacs.getCurrentBuffer();
    auto& t = b.getMainFrame();
    startMTest(qemacs, b, t, "ptest");
  }  // end of runPTest

  static QEmacsStandardFunctionCommandProxy<runMTest> runMTestProxy(
      "run-mtest");
  static QEmacsStandardFunctionCommandProxy<runPTest> runPTestProxy(
      "run-ptest");

  static StandardQEmacsMajorModeProxy<MTestMajorMode> proxy(
      "MTest", QVector<QRegExp>() << QRegExp("^[\\w-]+\\.mtest"));

}  // end of namespace qemacs
