/*!
 * \file  MFrontMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/08/2012
 */

#include <stdexcept>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QStringListModel>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtWidgets/QAbstractItemView>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "MFront/AbstractDSL.hxx"
#include "MFront/DSLFactory.hxx"
#include "QEmacs/MFrontInitializer.hxx"
#include "QEmacs/MFrontSyntaxHighlighter.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"
#include "QEmacs/QEmacsStandardFunctionCommand.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/MFrontOptions.hxx"
#include "QEmacs/MFrontMajorMode.hxx"

namespace qemacs {

  static QString getDSLName(QEmacsWidget &qemacs,
                            QEmacsTextEditBase &t) {
    using tfel::utilities::CxxTokenizer;
    try {
      CxxTokenizer tokenizer;
      tokenizer.parseString(t.document()->toPlainText().toStdString());
      tokenizer.stripComments();
      for (auto p = tokenizer.begin(); p != tokenizer.end(); ++p) {
        if((p->value == "@DSL") || (p->value == "@Parser")) {
          ++p;
          if(p == tokenizer.end()) { return "DefaultDSL"; }
          return QString::fromStdString(p->value);
        }
      }
    } catch(std::exception &e) {
      qemacs.displayInformativeMessage("getDSLName: " + QString(e.what()));
    } catch(...) {
      qemacs.displayInformativeMessage("getDSLName: unknown exception");
    }
    return "DefaultDSL";
  } // end of getDSLName

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
    auto& p = nf->getProcess();
    p.setWorkingDirectory(fn.dir().absolutePath());
    auto arg = QStringList{};
    arg << ("--verbose=" + o.vlvl);
    if (o.analysis_type == "Build") {
      arg << ("--obuild=" + o.vlvl);
    } else if (o.analysis_type == "Generate") {
      arg << ("--omake=" + o.vlvl);
    }
    if (!o.interface.isEmpty()) {
      arg << ("--interface=" + o.interface);
    }
    auto optional_argument = [&arg](const bool bvalue,
                                    const char *const opt) {
      if (bvalue) {
        arg << opt;
      }
    };
    optional_argument(o.debug, "--debug");
    optional_argument(o.warning, "--warning");
    optional_argument(o.pedantic, "--pedantic");
    arg << fn.absoluteFilePath();
    qDebug() << arg;
    p.start("mfront", arg);
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
  } // end of  startMFront

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
  } // end of MFrontMajorMode::MFrontMajorMode

  QString MFrontMajorMode::getName() const {
    return "MFront";
  } // end of MFrontMajorMode::getName

  QString MFrontMajorMode::getDescription() const {
    return "major mode dedicated to the MFront code generator";
  } // end of CppMajorMode

  QMenu *MFrontMajorMode::getSpecificMenu() {
    auto *t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    QMenu *m(new QMenu(QObject::tr("MFront"), t));
    m->addAction(this->rm);
    return m;
  } // end of

  QCompleter *MFrontMajorMode::getCompleter() {
    return this->c;
  } // end of MFrontMajorMode::getCompleter

  QString MFrontMajorMode::getCompletionPrefix() {
    auto tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    if(!tc.atBlockStart()) {
      tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
      if(tc.selectedText() == "@") {
        return '@' + this->textEdit.getCurrentWord();
      }
    }
    return CxxMajorMode::getCompletionPrefix();
  } // end of MFrontMajorMode::getCompletionPrefix

  QIcon MFrontMajorMode::getIcon() const {
    static QIcon i(":/mfront/MFrontIcon.png");
    return i;
  } // end of MFrontMajorMode::getIcon()

  void MFrontMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    new MFrontSyntaxHighlighter(
        d, getDSLName(this->qemacs, this->textEdit));
  } // end of MFrontMajorMode::setSyntaxHighlighter

  void MFrontMajorMode::updateSyntaxHighlighterAndCompleter() {
    this->rt->stop();
    if(!this->c->popup()->isVisible()) {
      this->setSyntaxHighlighter(this->textEdit.document());
      this->c->setModel(new QStringListModel(this->getKeyWordsList()));
    }
    this->rt->start(500);
  } // end of MFrontMajorMode::updateSyntaxHighlighterAndCompleter

  void MFrontMajorMode::completeContextMenu(QMenu *const m,
                                            const QTextCursor &tc) {
    QEmacsMajorModeBase::completeContextMenu(m, tc);
    const auto keys = this->getKeyWordsList();
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    b.select(QTextCursor::LineUnderCursor);
    QString l = b.selectedText();
    QRegExp r("^(@\\w+)");
    if(r.indexIn(l) >= 0) {
      QString k = r.cap(1);
      if(keys.indexOf(k) != -1) {
        delete this->ha;
        this->ha = new QAction(QObject::tr("Help on %1").arg(k), this);
        this->ha->setData(k);
        const auto cactions = m->actions();
        if(cactions.isEmpty()) {
          m->addAction(this->ha);
        } else {
          m->insertSeparator(*(cactions.begin()));
          m->insertAction(*(cactions.begin()), this->ha);
        }
        QObject::connect(m, &QMenu::triggered, this,
                         &MFrontMajorMode::actionTriggered);
      }
    }
  } // end of MFrontMajorMode::completeContextMenu

  void MFrontMajorMode::actionTriggered(QAction *a) {
    if(a == this->ha) {
      const auto k = this->ha->data().toString();
      auto nf = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.attachSecondaryTask(QObject::tr("help on '%1'").arg(k), nf);
      auto &p = nf->getProcess();
      if(p.state() != QProcess::Running) {
        const auto dsl = getDSLName(this->qemacs, this->textEdit);
        p.start("mfront", QStringList()
                              << ("--help-keyword=" + dsl + ":" + k));
        p.waitForStarted();
        p.waitForFinished(1000);
      }
      nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    }
  } // end of MFrontMajorMode::actionTriggered

  QStringList MFrontMajorMode::getKeyWordsList() {
    QStringList keys;
    try {
      auto &f = mfront::DSLFactory::getDSLFactory();
      const auto n = getDSLName(this->qemacs,this->textEdit);
      auto dsl = f.createNewParser(n.toStdString());
      std::vector<std::string> mkeys;
      dsl->getKeywordsList(mkeys);
      for(const auto &k : mkeys) {
        keys.append(QString::fromStdString(k));
      }
    } catch(std::exception &e) {
      this->report("MFrontMajorMode::getKeyWordList: " + QString(e.what()));
    } catch(...) {
      this->report("MFrontMajorMode::getKeyWordList: unknown exception");
    }
    return keys;
  } // end of MFrontMajorMode::getKeyWordsList

  QString MFrontMajorMode::getLanguageName() const{
    return "mfront";
  } // end of MFrontMajorMode::getLanguageName

  QString MFrontMajorMode::getDefaultCompilationCommand() const {
    return "mfront --obuild ";
  }

  MFrontMajorMode::~MFrontMajorMode() = default;

  static void runMFront(QEmacsWidget &qemacs) {
    auto& b = qemacs.getCurrentBuffer();
    auto &t = b.getMainFrame();
    startMFront(qemacs, b, t);
  }  // end of runMFront

  static QEmacsStandardFunctionCommandProxy<runMFront> runMFrontProxy("run-mfront");

  static StandardQEmacsMajorModeProxy<MFrontMajorMode>
  proxy("MFront",
	QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.mfront"));

} // end of namespace qemacs
