/*!
 * \file  MFrontMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/08/2012
 */

#include <map>
#include <stdexcept>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QStringListModel>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QAbstractItemView>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "MFront/AbstractDSL.hxx"
#include "MFront/DSLFactory.hxx"
#include "QEmacs/Debug.hxx"
#include "QEmacs/MFrontInitializer.hxx"
#include "QEmacs/MFrontSyntaxHighlighter.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/ProcessOutputMajorModeBase.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"
#include "QEmacs/QEmacsStandardFunctionCommand.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/MFrontOptions.hxx"
#include "QEmacs/MFrontMajorMode.hxx"

namespace qemacs {

  struct MFrontExamples {
    //! file implementing material properties, sorted by material
    std::map<QString, QStringList> properties;
    //! file implementing behaviours, sorted by material
    std::map<QString, QStringList> behaviours;
    //! file implementing models, sorted by material
    std::map<QString, QStringList> models;
  }; // end of struct MFrontExamples

  static MFrontExamples buildMFrontExamplesList(const char* const src) {
    auto r = MFrontExamples();
    const auto mfm = ::getenv(src);
    if (mfm == nullptr) {
      debug("variable",src,"is not defined");
      return r;
    }
#ifdef Q_OS_WIN
    const auto paths = QString(mfm).split(";", QString::SkipEmptyParts);
#else  /* Q_OS_WIN */
    const auto paths = QString(mfm).split(":", QString::SkipEmptyParts);
#endif /* Q_OS_WIN */
    for (const auto& path : paths) {
#ifdef Q_OS_WIN
      QDir d(path + "\\share\\mfm\\materials");
#else  /* Q_OS_WIN */
      QDir d(path + "/share/mfm/materials");
#endif /* Q_OS_WIN */
      if (!d.exists()) {
        continue;
      }
      for (const auto &md : d.entryInfoList()) {
        if(!md.isDir()){
          continue;
        }
        const auto amp = md.absoluteFilePath();
        const auto m = QDir(amp).dirName();
        auto append = [&m, &amp](std::map<QString, QStringList> &mfs,
                                 const QString &n) {
          QDir sd(amp + QDir::separator() + n);
          if(!sd.exists()){
            return;
          }
          for (const auto &f : sd.entryInfoList()) {
            if(f.suffix()=="mfront"){
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
  } // end of buildMFrontExamplesList

  static const MFrontExamples &getMFrontGalleryExamples() {
    static const MFrontExamples e(
        buildMFrontExamplesList("MFRONTGALLERYHOME"));
    return e;
  }

  static const MFrontExamples &getMFrontMaterialsExamples() {
    static const MFrontExamples e(
        buildMFrontExamplesList("MFMHOME"));
    return e;
  }

  static QString getDSLName(QEmacsWidget &qemacs,
                            QEmacsTextEditBase &t) {
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
      qemacs.displayInformativeMessage("getDSLName: " +
                                       QString(e.what()));
    } catch (...) {
      qemacs.displayInformativeMessage("getDSLName: unknown exception");
    }
    return "DefaultDSL";
  }  // end of getDSLName

  static void runMFront(QEmacsWidget &qemacs,
                        QEmacsBuffer &b,
                        QEmacsTextEditBase &t) {
    const auto n = t.getCompleteFileName();
    if (n.isEmpty()) {
      qemacs.displayInformativeMessage(
          QObject::tr("runMFront: no file name"));
      return;
    }
    auto mkt = [&qemacs, &t] {
      auto &f = mfront::DSLFactory::getDSLFactory();
      const auto name = getDSLName(qemacs, t);
      const auto dsl =
          f.createNewDSL(name.toStdString())->getTargetType();
      if (dsl == mfront::AbstractDSL::MATERIALPROPERTYDSL) {
        return MFrontOptionsDialog::MATERIALPROPERTY;
      } else if (dsl == mfront::AbstractDSL::BEHAVIOURDSL) {
        return MFrontOptionsDialog::BEHAVIOUR;
      }
      return MFrontOptionsDialog::MODEL;
    }();
    MFrontOptions o;
    MFrontOptionsDialog od(o, mkt, &t);
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
    b.attachSecondaryTask(QObject::tr("MFront output"), nf);
    auto &p = nf->getProcess();
    p.setWorkingDirectory(fn.dir().absolutePath());
    auto args = QStringList{};
    args << "--no-gui";
    args << ("--verbose=" + o.vlvl);
    if (o.analysis_type == "Build") {
      args << ("--obuild=" + o.vlvl);
    } else if (o.analysis_type == "Generate") {
      args << ("--omake=" + o.vlvl);
    }
    if (!o.i.isEmpty()) {
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

  static void startMFront(QEmacsWidget &qemacs,
                          QEmacsBuffer &b,
                          QEmacsTextEditBase &t) {
    if (t.isModified()) {
      auto *input = t.getSaveInput();
      QObject::connect(input, &QEmacsTextEditBase::SaveInput::finished,
                       [&qemacs, &b, &t] { runMFront(qemacs, b, t); });
      qemacs.setUserInput(input);
      return;
    }
    runMFront(qemacs, b, t);
  }  // end of  startMFront

  MFrontMajorMode::MFrontMajorMode(QEmacsWidget &w,
                                   QEmacsBuffer &b,
                                   QEmacsTextEditBase &t)
      : CxxMajorMode(w, b, t) {
    MFrontInitializer::init();
    this->c = new QCompleter(this->getKeyWordsList(), &t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,
                     static_cast<void (QCompleter::*)(const QString &)>(
                         &QCompleter::activated),
                     &t, &QEmacsTextEditBase::insertCompletion);
    // timer
    this->rt = new QTimer(this);
    connect(this->rt, &QTimer::timeout, this,
            &MFrontMajorMode::updateSyntaxHighlighterAndCompleter);
    this->rt->start(500);
    this->rm = new QAction(QObject::tr("Run MFront"), this);
    this->rm->setIcon(this->getIcon());
    QObject::connect(this->rm, &QAction::triggered, this, [this] {
      startMFront(this->qemacs, this->buffer, this->textEdit);
    });
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
      QObject::connect(a, &QAction::triggered,
                       [url] { QDesktopServices::openUrl(QUrl(url)); });
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
    m->addAction(this->rm);
    auto *const w = m->addMenu(QObject::tr("Wizards"));
    w->setIcon(QIcon(":/qemacs/misc/wizard.png"));
    auto *const sebw = w->addAction(QObject::tr("Implicit DSL wizard"));
    sebw->setToolTip(
        QObject::tr("Create the basis of a behaviour \n"
                    "implementation based on the Implicit DSL."));
    QObject::connect(sebw, &QAction::triggered, this,
                     &MFrontMajorMode::runImplicitDSLWizard);
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
                               [f, this] { this->qemacs.openFile(f); });
            }
          }
        };
        append(QObject::tr("Properties"), emfm.properties);
        append(QObject::tr("Behaviours"), emfm.behaviours);
        append(QObject::tr("Models"), emfm.models);
      }
    };
    append_examples(QObject::tr("Open examples from MFrontGallery"),
                    getMFrontGalleryExamples());
    append_examples(QObject::tr("Open examples from MFrontMaterials"),
                    getMFrontMaterialsExamples());
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
    new MFrontSyntaxHighlighter(
        d, getDSLName(this->qemacs, this->textEdit));
  }  // end of MFrontMajorMode::setSyntaxHighlighter

  void MFrontMajorMode::updateSyntaxHighlighterAndCompleter() {
    this->rt->stop();
    if (!this->c->popup()->isVisible()) {
      this->setSyntaxHighlighter(this->textEdit.document());
      this->c->setModel(new QStringListModel(this->getKeyWordsList()));
    }
    this->rt->start(500);
  }  // end of MFrontMajorMode::updateSyntaxHighlighterAndCompleter

  void MFrontMajorMode::completeContextMenu(QMenu *const m,
                                            const QTextCursor &tc) {
    QEmacsMajorModeBase::completeContextMenu(m, tc);
    const auto keys = this->getKeyWordsList();
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    b.select(QTextCursor::LineUnderCursor);
    QString l = b.selectedText();
    QRegExp r("^(@\\w+)");
    if (r.indexIn(l) >= 0) {
      QString k = r.cap(1);
      if (keys.indexOf(k) != -1) {
        delete this->ha;
        this->ha = new QAction(QObject::tr("Help on %1").arg(k), this);
        this->ha->setData(k);
        this->ha->setIcon(QIcon::fromTheme("dialog-question"));
        const auto cactions = m->actions();
        if (cactions.isEmpty()) {
          m->addAction(this->ha);
        } else {
          m->insertSeparator(*(cactions.begin()));
          m->insertAction(*(cactions.begin()), this->ha);
        }
        QObject::connect(m, &QMenu::triggered, this,
                         &MFrontMajorMode::actionTriggered);
      }
    }
  }  // end of MFrontMajorMode::completeContextMenu

  void MFrontMajorMode::actionTriggered(QAction *a) {
    if (a == this->ha) {
      const auto k = this->ha->data().toString();
      auto nf = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.attachSecondaryTask(
          QObject::tr("help on '%1'").arg(k), nf);
      auto &p = nf->getProcess();
      if (p.state() != QProcess::Running) {
        const auto dsl = getDSLName(this->qemacs, this->textEdit);
        p.start("mfront", QStringList()
                              << ("--help-keyword=" + dsl + ":" + k));
        p.waitForStarted();
        p.waitForFinished(1000);
      }
      nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    }
  }  // end of MFrontMajorMode::actionTriggered

  QStringList MFrontMajorMode::getKeyWordsList() {
    QStringList keys;
    try {
      auto &f = mfront::DSLFactory::getDSLFactory();
      const auto n = getDSLName(this->qemacs, this->textEdit);
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

  void MFrontMajorMode::runImplicitDSLWizard() {
    
  }  // end of MFrontMajorMode::runImplicitDSLWizard

  MFrontMajorMode::~MFrontMajorMode() = default;

  void runMFront(QEmacsWidget &qemacs) {
    auto &b = qemacs.getCurrentBuffer();
    auto &t = b.getMainFrame();
    startMFront(qemacs, b, t);
  }  // end of runMFront

  static QEmacsStandardFunctionCommandProxy<runMFront> runMFrontProxy(
      "run-mfront");

  static StandardQEmacsMajorModeProxy<MFrontMajorMode> proxy(
      "MFront",
      QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.mfront"),
      ":/mfront/MFrontIcon.png");

}  // end of namespace qemacs
