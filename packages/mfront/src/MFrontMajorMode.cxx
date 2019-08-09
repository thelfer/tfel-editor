/*!
 * \file   packages/mfront/src/MFrontMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/08/2012
 */

#include <map>
#include <stdexcept>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QStringListModel>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QAbstractItemView>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "MFront/AbstractDSL.hxx"
#include "MFront/AbstractBehaviourDSL.hxx"
#include "MFront/BehaviourDSLCommon.hxx"
#include "MFront/DSLFactory.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/MFrontInitializer.hxx"
#include "TFEL/GUI/MFrontSyntaxHighlighter.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandFactory.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontOptions.hxx"
#include "TFEL/GUI/MFrontOptionsDialog.hxx"
#include "TFEL/GUI/MFrontMajorMode.hxx"

namespace tfel{

  namespace gui{

  struct MFrontExamples {
    //! file implementing material properties, sorted by material
    std::map<QString, QStringList> properties;
    //! file implementing behaviours, sorted by material
    std::map<QString, QStringList> behaviours;
    //! file implementing models, sorted by material
    std::map<QString, QStringList> models;
  };  // end of struct MFrontExamples

  static MFrontExamples buildMFrontExamplesList(const char *const src) {
    auto r = MFrontExamples();
    const auto mfm = ::getenv(src);
    if (mfm == nullptr) {
      debug("variable", src, "is not defined");
      return r;
    }
#ifdef Q_OS_WIN
    const auto paths = QString(mfm).split(";", QString::SkipEmptyParts);
#else  /* Q_OS_WIN */
    const auto paths = QString(mfm).split(":", QString::SkipEmptyParts);
#endif /* Q_OS_WIN */
    for (const auto &path : paths) {
#ifdef Q_OS_WIN
      QDir d(path + "\\share\\mfm\\materials");
#else  /* Q_OS_WIN */
      QDir d(path + "/share/mfm/materials");
#endif /* Q_OS_WIN */
      if (!d.exists()) {
        continue;
      }
      for (const auto &md : d.entryInfoList()) {
        if (!md.isDir()) {
          continue;
        }
        const auto amp = md.absoluteFilePath();
        const auto m = QDir(amp).dirName();
        auto append = [&m, &amp](std::map<QString, QStringList> &mfs,
                                 const QString &n) {
          QDir sd(amp + QDir::separator() + n);
          if (!sd.exists()) {
            return;
          }
          for (const auto &f : sd.entryInfoList()) {
            if (f.suffix() == "mfront") {
              mfs[m].append(f.absoluteFilePath());
            }
          }
        };
        append(r.properties, "properties");
        append(r.behaviours, "behaviours");
        append(r.models, "models");
      }
    }
    return r;
  }  // end of buildMFrontExamplesList

  static const MFrontExamples &getMFrontGalleryExamples() {
    static const MFrontExamples e(
        buildMFrontExamplesList("MFRONTGALLERYHOME"));
    return e;
  }

  static const MFrontExamples &getMFrontMaterialsExamples() {
    static const MFrontExamples e(buildMFrontExamplesList("MFMHOME"));
    return e;
  }

  static QString getDSLName(EditorWidget &editor, TextEditBase &t) {
    using tfel::utilities::CxxTokenizer;
    try {
      CxxTokenizer tokenizer;
      tokenizer.parseString(t.document()->toPlainText().toStdString());
      tokenizer.stripComments();
      for (auto p = tokenizer.begin(); p != tokenizer.end(); ++p) {
        if ((p->value == "@DSL") || (p->value == "@Parser")) {
          ++p;
          if (p == tokenizer.end()) {
            return "DefaultDSL";
          }
          return QString::fromStdString(p->value);
        }
      }
    } catch (std::exception &e) {
      editor.displayInformativeMessage("getDSLName: " +
                                       QString(e.what()));
    } catch (...) {
      editor.displayInformativeMessage("getDSLName: unknown exception");
    }
    return "DefaultDSL";
  }  // end of getDSLName

  static void runMFront(EditorWidget &editor,
                        Buffer &b,
                        TextEditBase &t) {
    static const QString mpli(
        "/tfel/editor/packages/mfront/material-property-last-interface");
    static const QString bli(
        "/tfel/editor/packages/mfront/behaviour-last-interface");
    static const QString mli(
        "/tfel/editor/packages/mfront/model-last-interface");
    const auto n = t.getCompleteFileName();
    if (n.isEmpty()) {
      editor.displayInformativeMessage(
          QObject::tr("runMFront: no file name"));
      return;
    }
    QFileInfo fn(n);
    if ((!fn.exists()) || (!fn.isFile())) {
      editor.displayInformativeMessage(
          QObject::tr("invalid file name"));
      return;
    }
    // no structured binding yet
    const auto dsln_mkt = [&editor, &t] {
      auto &f = mfront::DSLFactory::getDSLFactory();
      const auto name = getDSLName(editor, t);
      const auto dsl =
          f.createNewDSL(name.toStdString())->getTargetType();
      if (dsl == mfront::AbstractDSL::MATERIALPROPERTYDSL) {
        return std::make_pair(name,
                              MFrontOptionsDialog::MATERIALPROPERTY);
      } else if (dsl == mfront::AbstractDSL::BEHAVIOURDSL) {
        return std::make_pair(name, MFrontOptionsDialog::BEHAVIOUR);
      }
      return std::make_pair(name, MFrontOptionsDialog::MODEL);
    }();
    const auto &mkt = dsln_mkt.second;
    QSettings s;
    MFrontOptions o;
    o.dsl = dsln_mkt.first;
    o.file = fn.absoluteFilePath();
    if (mkt == MFrontOptionsDialog::MATERIALPROPERTY) {
      if (s.contains(mpli)) {
        o.i = s.value(mpli).toString();
      }
    } else if (mkt == MFrontOptionsDialog::BEHAVIOUR) {
      if (s.contains(bli)) {
        o.i = s.value(bli).toString();
      }
    } else {
      if (s.contains(mli)) {
        o.i = s.value(mli).toString();
      }
    }
    MFrontOptionsDialog od(editor, o, mkt, &t);
    if (od.exec() == QDialog::Rejected) {
      return;
    }
    auto nf = new ProcessOutputFrame(editor, b);
    b.attachSecondaryTask(QObject::tr("MFront output"), nf);
    auto &p = nf->getProcess();
    p.setWorkingDirectory(fn.dir().absolutePath());
    auto args = QStringList{};
    args << "--no-gui";
    args << ("--verbose=" + o.vlvl);
    if (o.analysis_type == "Build") {
      args << ("--obuild=" + o.olvl);
    } else if (o.analysis_type == "Generate") {
      args << ("--omake=" + o.olvl);
    }
    if (!o.i.isEmpty()) {
      if (mkt == MFrontOptionsDialog::MATERIALPROPERTY) {
        s.setValue(mpli, o.i);
      } else if (mkt == MFrontOptionsDialog::BEHAVIOUR) {
        s.setValue(bli, o.i);
      } else {
        s.setValue(mli, o.i);
      }
      args << ("--interface=" + o.i);
    }
    auto optional_argument = [&args](const bool bvalue,
                                     const char *const opt) {
      if (bvalue) {
        args << opt;
      }
    };
    optional_argument(o.debug, "--debug");
    optional_argument(o.warning, "--warning");
    optional_argument(o.pedantic, "--pedantic");
    if (mkt == MFrontOptionsDialog::BEHAVIOUR) {
      optional_argument(o.profiling, "--@Profiling=true");
    }
    if (o.compare_to_numerical_jacobian) {
      if (!o.numerical_jacobian_criterion.isEmpty()) {
        args << "--@"
                "JacobianComparisonCriterion=" +
                    o.numerical_jacobian_criterion;
      }
      if (!o.numerical_jacobian_perturbation.isEmpty()) {
        args << "--@"
                "PerturbationValueForNumericalJacobianComputation=" +
                    o.numerical_jacobian_perturbation;
      }
      args << "--@CompareToNumericalJacobian=true";
    }
    args << fn.absoluteFilePath();
    auto *m = qobject_cast<ProcessOutputMajorModeBase *>(
        nf->setMajorMode("compilation-output"));
    if (m != nullptr) {
      m->setDirectory(p.workingDirectory());
      m->setCommand("mfront");
      m->setArguments(args);
      m->setMajorMode("compilation-output");
    }
    p.start("mfront", args);
    p.waitForStarted();
  }  // end of runMFront

  static void startMFront(EditorWidget &editor,
                          Buffer &b,
                          TextEditBase &t) {
    if (t.isModified()) {
      auto *input = t.getSaveInput();
      QObject::connect(input, &TextEditBase::SaveInput::finished,
                       [&editor, &b, &t] { runMFront(editor, b, t); });
      editor.setUserInput(input);
      return;
    }
    runMFront(editor, b, t);
  }  // end of  startMFront

  static void buildAnalyseFileMenu(
      QObject *o,
      QMenu *m,
      const mfront::BehaviourDescription &bd,
      const tfel::material::ModellingHypothesis::Hypothesis h,
      EditorWidget &w,
      Buffer &b,
      TextEditBase &t) {
    const auto &d = bd.getBehaviourData(h);
    if (!d.getCodeBlockNames().empty()) {
      auto *const vc = m->addMenu(QObject::tr("View code"));
      for (const auto &cb : d.getCodeBlockNames()) {
        auto *const vca = vc->addAction(QString::fromStdString(cb));
        QObject::connect(vca, &QAction::triggered, o, [&w, &b, &t, cb,
                                                       h] {
          auto &f = mfront::DSLFactory::getDSLFactory();
          try {
            auto *const cbb = new PlainTextEdit(w, b);
            auto dsl = f.createNewDSL(getDSLName(w, t).toStdString());
            dsl->analyseString(
                t.document()->toPlainText().toStdString());
            dsl->endsInputFileProcessing();
            const auto &nbd =
                dynamic_cast<mfront::AbstractBehaviourDSL &>(*dsl)
                    .getBehaviourDescription();
            const auto &nd = nbd.getBehaviourData(h);
            cbb->appendPlainText(
                QString::fromStdString(nd.getCodeBlock(cb).code));
            cbb->setMajorMode("C++");
            cbb->format();
            b.attachSecondaryTask(QString::fromStdString(cb), cbb);
            b.setSecondaryTaskIcon(cbb,
                                   QIcon::fromTheme("mfront-icon"));
          } catch (std::exception &e) {
            debug(e.what());
          } catch (...) {
          }
        });
      }
    }
  }  // end of buildAnalyseFileMenu

  MFrontMajorMode::MFrontMajorMode(EditorWidget &w,
                                   Buffer &b,
                                   TextEditBase &t)
      : CxxMajorMode(w, b, t) {
    MFrontInitializer::init();
    this->c = new QCompleter(this->getKeyWordsList(), &t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,
                     static_cast<void (QCompleter::*)(const QString &)>(
                         &QCompleter::activated),
                     &t, &TextEditBase::insertCompletion);
    // timer
    this->rt = new QTimer(this);
    connect(this->rt, &QTimer::timeout, [this] {
      auto dsl = getDSLName(this->editor,this->textEdit);
      if (dsl!=this->current_dsl){
        this->updateSyntaxHighlighterAndCompleter();
        this->buffer.updateMenu();
        this->current_dsl = dsl;
      }
    });
    this->rt->start(2000);
  }  // end of MFrontMajorMode::MFrontMajorMode

  QString MFrontMajorMode::getName() const {
      return "MFront";
  }  // end of MFrontMajorMode::getName

  QString MFrontMajorMode::getDescription() const {
      return "major mode dedicated to the MFront code generator";
  }  // end of CppMajorMode

  QMenu *MFrontMajorMode::getSpecificMenu() {
      auto online_ressource = [this](QMenu *const m, const QString &t,
                                     const QString &url) {
        auto *const a = m->addAction(t);
        QObject::connect(a, &QAction::triggered, [url] {
          QDesktopServices::openUrl(QUrl(url));
        });
        return a;
      };  // end of online_ressources
      auto online_ressource2 = [online_ressource](
          QMenu *const m, const QString &t, const QString &url,
          const QIcon i) {
        online_ressource(m, t, url)->setIcon(i);
      };  // end of online_ressources2
      auto *t = qobject_cast<QWidget *>(this->parent());
      if (t == nullptr) {
        return nullptr;
    }
    auto *const m = new QMenu(QObject::tr("MFront"), t);

    auto *const rm = m->addAction(QObject::tr("Run MFront"));
    rm->setIcon(this->getIcon());
    QObject::connect(rm, &QAction::triggered, this, [this] {
      startMFront(this->editor, this->buffer, this->textEdit);
    });
    // wizards
    auto *const w = m->addMenu(QObject::tr("Wizards"));
    w->setIcon(QIcon(":/tfel/editor/misc/wizard.png"));
    auto *const sebw = w->addAction(QObject::tr("Behaviour wizard"));
    sebw->setToolTip(
        QObject::tr("Create the basis of a behaviour implementation"));
    QObject::connect(sebw, &QAction::triggered, this,
                     &MFrontMajorMode::runMFrontBehaviourWizard);
    auto append_examples = [this, &m](const QString &label,
                                      const MFrontExamples &emfm) {
      if ((!emfm.properties.empty()) || (!emfm.behaviours.empty()) ||
          (!emfm.models.empty())) {
        auto *const em = m->addMenu(label);
        em->setIcon(this->getIcon());
        auto append = [this, &em](
            const QString &n,
            const std::map<QString, QStringList> &mfs) {
          if (mfs.empty()) {
            return;
          }
          auto *const sem = em->addMenu(n);
          for (const auto &mn : mfs) {
            auto *const sem2 = sem->addMenu(mn.first);
            for (const auto &f : mn.second) {
              QFileInfo fi(f);
              auto *const a = sem2->addAction(fi.fileName());
              a->setIcon(this->getIcon());
              QObject::connect(a, &QAction::triggered, this,
                               [f, this] { this->editor.openFile(f); });
            }
          }
        };
        append(QObject::tr("Properties"), emfm.properties);
        append(QObject::tr("Behaviours"), emfm.behaviours);
        append(QObject::tr("Models"), emfm.models);
      }
    };
    // explorer
    try {
      auto &f = mfront::DSLFactory::getDSLFactory();
      auto dsl = f.createNewDSL(
          getDSLName(this->editor, this->textEdit).toStdString());
      if (dsl->getTargetType() == mfront::AbstractDSL::BEHAVIOURDSL) {
        dsl->analyseString(
            this->textEdit.document()->toPlainText().toStdString());
        dsl->endsInputFileProcessing();
        auto *const a = m->addMenu(QObject::tr("Analyse File"));
        const auto &bd =
            dynamic_cast<mfront::AbstractBehaviourDSL &>(*dsl)
                .getBehaviourDescription();
        for (const auto h : bd.getDistinctModellingHypotheses()) {
          if (h == tfel::material::ModellingHypothesis::
                       UNDEFINEDHYPOTHESIS) {
            buildAnalyseFileMenu(this, a, bd, h, this->editor,
                                 this->buffer, this->textEdit);
          } else {
            auto *const ah = a->addMenu(QString::fromStdString(
                tfel::material::ModellingHypothesis::toString(h)));
            buildAnalyseFileMenu(this, ah, bd, h, this->editor,
                                 this->buffer, this->textEdit);
          }
        }
      }
    } catch (std::exception &e) {
      debug(e.what());
    } catch (...) {
    }
    // examples
    append_examples(QObject::tr("Open examples from MFrontGallery"),
                    getMFrontGalleryExamples());
    append_examples(QObject::tr("Open examples from MFrontMaterials"),
                    getMFrontMaterialsExamples());
    // online documentations
    auto *const d = m->addMenu(QObject::tr("Online Documentation"));
    d->setIcon(QIcon::fromTheme("help-browser"));
    online_ressource2(d, QObject::tr("MFront website"),
                      "http://tfel.sourceforge.net", this->getIcon());
    online_ressource2(d, QObject::tr("General documentation"),
                      "http://tfel.sourceforge.net/documentation.html",
                      this->getIcon());
    online_ressource2(d, QObject::tr("MFront gallery"),
                      "http://tfel.sourceforge.net/gallery.html",
                      this->getIcon());
    online_ressource2(d, QObject::tr("Frequently asked questions"),
                      "http://tfel.sourceforge.net/faq.html",
                      QIcon::fromTheme("help-faq"));
    online_ressource2(d, QObject::tr("Search on the website"),
                      "http://tfel.sourceforge.net/search.html",
                      QIcon::fromTheme("system-search"));
    // help menu
    auto *const h = m->addMenu(QObject::tr("Getting help"));
    h->setIcon(QIcon::fromTheme("dialog-question"));
    online_ressource(h, QObject::tr("Forum"),
                     "https://sourceforge.net/p/tfel/discussion/");
    online_ressource(h, QObject::tr("Report bugs"),
                     "https://sourceforge.net/p/tfel/tickets/");
    online_ressource(
        h, QObject::tr("Contact the authors"),
        "mailto:tfel-contact@cea.fr?subject=About MFront usage");
    return m;
  }  // end of MFrontMajorMode::getSpecificMenu

  QCompleter *MFrontMajorMode::getCompleter() {
    return this->c;
  }  // end of MFrontMajorMode::getCompleter

  QString MFrontMajorMode::getCompletionPrefix() {
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
  }  // end of MFrontMajorMode::getCompletionPrefix

  QIcon MFrontMajorMode::getIcon() const {
    static QIcon i(":/mfront/MFrontIcon.png");
    return i;
  }  // end of MFrontMajorMode::getIcon()

  void MFrontMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    if (this->highlighter != nullptr) {
      // the previous highlighter must be deleted, otherwise the
      // highlighters accumulate and memory is only freed when the
      // document is closed. This can seriously impact the
      // performance
      // of editor.
      this->highlighter->deleteLater();
    }
    this->highlighter = new MFrontSyntaxHighlighter(
        d, getDSLName(this->editor, this->textEdit));
  }  // end of MFrontMajorMode::setSyntaxHighlighter

  void MFrontMajorMode::updateSyntaxHighlighterAndCompleter() {
    this->rt->stop();
    if (!this->c->popup()->isVisible()) {
      this->setSyntaxHighlighter(this->textEdit.document());
      this->c->setModel(new QStringListModel(this->getKeyWordsList()));
    }
    this->rt->start(1000);
  }  // end of
     // MFrontMajorMode::updateSyntaxHighlighterAndCompleter

  void MFrontMajorMode::completeContextMenu(QMenu *const m,
                                            const QTextCursor &tc) {
    MajorModeBase::completeContextMenu(m, tc);
    const auto keys = this->getKeyWordsList();
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    b.select(QTextCursor::LineUnderCursor);
    QString l = b.selectedText();
    QRegExp r("^(@\\w+)");
    if (r.indexIn(l) >= 0) {
      QString k = r.cap(1);
      if (keys.indexOf(k) != -1) {
        auto ha = new QAction(QObject::tr("Help on %1").arg(k), m);
        ha->setIcon(QIcon::fromTheme("dialog-question"));
        const auto cactions = m->actions();
        if (cactions.isEmpty()) {
          m->addAction(ha);
        } else {
          m->insertSeparator(*(cactions.begin()));
          m->insertAction(*(cactions.begin()), ha);
        }
        QObject::connect(ha, &QAction::triggered, this, [k, this] {
          auto nf = new ProcessOutputFrame(this->editor, this->buffer);
          this->buffer.attachSecondaryTask(
              QObject::tr("help on '%1'").arg(k), nf);
          auto &p = nf->getProcess();
          if (p.state() != QProcess::Running) {
            const auto dsl = getDSLName(this->editor, this->textEdit);
            p.start("mfront", QStringList() << ("--help-keyword=" +
                                                dsl + ":" + k));
            p.waitForStarted();
            p.waitForFinished(1000);
          }
          nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
        });
      }
    }
  }  // end of MFrontMajorMode::completeContextMenu

  QStringList MFrontMajorMode::getKeyWordsList() {
    QStringList keys;
    try {
      auto &f = mfront::DSLFactory::getDSLFactory();
      const auto n = getDSLName(this->editor, this->textEdit);
      auto dsl = f.createNewParser(n.toStdString());
      std::vector<std::string> mkeys;
      dsl->getKeywordsList(mkeys);
      for (const auto &k : mkeys) {
        keys.append(QString::fromStdString(k));
      }
    } catch (std::exception &e) {
      this->report("MFrontMajorMode::getKeyWordList: " +
                   QString(e.what()));
    } catch (...) {
      this->report(
          "MFrontMajorMode::getKeyWordList: unknown exception");
    }
    return keys;
  }  // end of MFrontMajorMode::getKeyWordsList

  QString MFrontMajorMode::getLanguageName() const {
    return "mfront";
  }  // end of MFrontMajorMode::getLanguageName

  QString MFrontMajorMode::getDefaultCompilationCommand() const {
    return "mfront --obuild ";
  }

  void MFrontMajorMode::runMFrontBehaviourWizard() {
    MFrontBehaviourWizard w(this->editor, this->textEdit);
    if (w.exec() == QDialog::Accepted) {
      w.write();
    }
  }  // end of MFrontMajorMode::runMFrontBehaviourWizard

  void MFrontMajorMode::runCompilation() {
    startMFront(this->editor, this->buffer, this->textEdit);
  }  // end of MFrontMajorMode::runCompilation

  MFrontMajorMode::~MFrontMajorMode() = default;

  void runMFront(EditorWidget &editor) {
    auto &b = editor.getCurrentBuffer();
    auto &t = b.getMainFrame();
    startMFront(editor, b, t);
  }  // end of runMFront

  static StandardFunctionCommandProxy<runMFront> runMFrontProxy(
      "run-mfront");

  static StandardMajorModeProxy<MFrontMajorMode> proxy(
      "MFront",
      QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.mfront"),
      ":/mfront/MFrontIcon.png");

}  // end of namespace gui
}// end of namespace tfel
