/*!
 * \file  MFrontMajorMode.cxx
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
#include "MFront/AbstractDSL.hxx"
#include "MFront/DSLFactory.hxx"
#include "MFront/InitDSLs.hxx"
#include "QEmacs/MFrontSyntaxHighlighter.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/MFrontMajorMode.hxx"

namespace qemacs {

  MFrontMajorMode::MFrontMajorMode(QEmacsWidget &w,
                                   QEmacsBuffer &b,
                                   QEmacsTextEditBase &t)
    : CxxMajorMode(w, b, t) {
    mfront::initDSLs();
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
    QObject::connect(this->rm, &QAction::triggered, this,
                     &MFrontMajorMode::runMFront);
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
    new MFrontSyntaxHighlighter(d, this->getDSLName());
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

  QString MFrontMajorMode::getDSLName() {
    using tfel::utilities::CxxTokenizer;
    try {
      CxxTokenizer t;
      t.parseString(this->textEdit.document()->toPlainText().toStdString());
      t.stripComments();
      for(auto p = t.begin(); p != t.end(); ++p) {
        if((p->value == "@DSL") || (p->value == "@Parser")) {
          ++p;
          if(p == t.end()) { return "DefaultDSL"; }
          return QString::fromStdString(p->value);
        }
      }
    } catch(std::exception &e) {
      qDebug() << e.what();
      this->report("MFrontMajorMode::getDSLName: " + QString(e.what()));
    } catch(...) {
      this->report("MFrontMajorMode::getDSLName: unknown exception");
    }
    return "DefaultDSL";
  } // end of MFrontMajorMode::getDSLName

  void MFrontMajorMode::actionTriggered(QAction *a) {
    if(a == this->ha) {
      const auto k = this->ha->data().toString();
      auto nf = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.addSlave(QObject::tr("help on '%1'").arg(k), nf);
      auto &p = nf->getProcess();
      if(p.state() != QProcess::Running) {
        p.start("mfront", QStringList() << ("--help-keyword="
                                            + this->getDSLName() + ":" + k));
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
      const auto n = this->getDSLName();
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

  void MFrontMajorMode::runMFront() {
    if (this->textEdit.isModified()) {
      auto *input = this->textEdit.getSaveInput();
      QObject::connect(input, &QEmacsTextEditBase::SaveInput::finished, this,
                       &MFrontMajorMode::startMFront);
      this->qemacs.setUserInput(input);
      return;
    }
    this->startMFront();
  } // end of MFrontMajorMode::runMFront()

  QString MFrontMajorMode::getLanguageName() const{
    return "mfront";
  } // end of MFrontMajorMode::getLanguageName

  QString MFrontMajorMode::getDefaultCompilationCommand() const {
    return "mfront --obuild ";
  }

  void MFrontMajorMode::startMFront(){
    const auto n = this->textEdit.getCompleteFileName();
    if (n.isEmpty()) {
      this->report(QObject::tr("MFrontMajorMode::startMFront: no file name"));
      return;
    }
    //     MFrontAnalysisOptions o;
    //     MFrontAnalysisOptionsDialog od(o, &(this->textEdit));
    //     if (od.exec() == QDialog::Rejected) { return; }
    //     const auto &af = QFileInfo(n).absoluteFilePath();
    //     auto *s = new MFrontOutputFrame(this->qemacs, this->buffer, af, o);
    //     QObject::connect(s, &MFrontOutputFrame::finished, this,
    //                      &MFrontMajorMode::AnalysisFinished);
    //     this->buffer.addSlave(QObject::tr("MFront Output"), s);
  } // end of startMFront

  MFrontMajorMode::~MFrontMajorMode() = default;

  static StandardQEmacsMajorModeProxy<MFrontMajorMode>
  proxy("MFront",
	QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.mfront"));

} // end of namespace qemacs
