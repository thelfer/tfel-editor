/*!
 * \file  LicosMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#include <string>
#include <stdexcept>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWizard>

#include "TFEL/Raise.hxx"
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/GUI/LicosInsertMaterialPropertyDialog.hxx"
#include "TFEL/GUI/LicosMainBlock.hxx"
#include "TFEL/GUI/LicosMajorMode.hxx"
#include "TFEL/GUI/LicosMaterialWizard.hxx"
#include "TFEL/GUI/LicosOutputFrame.hxx"
#include "TFEL/GUI/LicosStudyOptions.hxx"
#include "TFEL/GUI/LicosSyntaxHighlighter.hxx"
#include "TFEL/GUI/LicosTokenizer.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Utilities.hxx"

namespace tfel::gui {

  struct LicosMajorMode::LicosInsertBlock : public CommandLine {
    LicosInsertBlock(EditorWidget &p, TextEditBase &t)
        : CommandLine(QObject::tr("insert block :"), p), textEdit(t) {
      QStringList blocks;
      for (const auto &b : LicosSyntaxHighlighter::getBlocks()) {
        blocks << QString::fromStdString(b);
      }
      auto *const co = new QCompleter(blocks, &p);
      co->setWidget(this->input);
      co->setCompletionMode(QCompleter::InlineCompletion);
      this->setInputHistorySettingAddress("licos/insertblock/history");
      this->input->setCompleter(co, false);
    }

    ~LicosInsertBlock() override = default;

   protected:
    void treatUserInput() override {
      const QString &b = this->input->text();
      if (!b.isEmpty()) {
        QTextCursor tc = this->textEdit.textCursor();
        QString s;
        if (!tc.hasSelection()) {
          tc.select(QTextCursor::LineUnderCursor);
        }
        s = tc.selectedText().trimmed();
        tc.removeSelectedText();
        tc.insertText(b + ' ');
        int p = tc.position();
        tc.movePosition(QTextCursor::EndOfBlock);
        if (!s.isEmpty()) {
          tc.insertText("\n" + s);
          tc.movePosition(QTextCursor::EndOfBlock);
        }
        tc.insertText("\nEndOf" + b);
        tc.movePosition(QTextCursor::PreviousBlock);
        tc.setPosition(p);
        this->textEdit.setTextCursor(tc);
      }
    }

    TextEditBase &textEdit;

  };  // end of struct TextEdit::LicosInsertBlock

  QStringList LicosMajorMode::buildLicosExtensionsSuffix() {
    QStringList e;
    e << "ple"
      << "txt"
      << "data"
      << "dat"
      << "py"
      << "mfront"
      << "ps";
    return e;
  }  // end of LicosMajorMode::buildLicosExtensionsSuffix

  const QStringList &LicosMajorMode::getLicosExtensionsSuffix() {
    static QStringList e(LicosMajorMode::buildLicosExtensionsSuffix());
    return e;
  }  // end of LicosMajorMode::buildLicosExtensionsSuffix

  bool LicosMajorMode::checkFile(QString &r, const QString &f) {
    QFileInfo fi;
    fi.setFile(f);
    r.clear();
    if ((fi.exists()) && (fi.isFile())) {
      if (!fi.isReadable()) {
        this->editor.displayInformativeMessage(
            QObject::tr("file %1 is not readable").arg(fi.absoluteFilePath()));
        return false;
      }
      r = fi.absoluteFilePath();
      return true;
    }
    return false;
  }

  QString LicosMajorMode::getLicosFile(const QString &f) {
    using namespace std;
    if (f.isEmpty()) {
      return "";
    }
    const char *userFiles = nullptr;
    const QString lpath = this->getLicosPath();
    QString r;
    QString d =
        QFileInfo(this->textEdit.getCompleteFileName()).dir().absolutePath();
    if (checkFile(r, d + QDir::separator() + f)) {
      return r;
    }
    if (checkFile(r, d + QDir::separator() + "inputs/" + f)) {
      return r;
    }
    if (checkFile(r, d + QDir::separator() + "scripts/" + f)) {
      return r;
    }
    if (checkFile(r, d + QDir::separator() + "data/" + f)) {
      return r;
    }
    if (checkFile(r, f)) {
      return r;
    }
    if (checkFile(r, "inputs/" + f)) {
      return r;
    }
    if (checkFile(r, "scripts/" + f)) {
      return r;
    }
    if (checkFile(r, "data/" + f)) {
      return r;
    }
    userFiles = ::getenv("LICOS_USER_FILES");
    if (userFiles != nullptr) {
      QStringList paths =
          QString(userFiles).split(":", QString::SkipEmptyParts);
      QStringList::const_iterator p = paths.begin();
      QStringList::const_iterator pe = paths.end();
      while (p != pe) {
        QString file = *p + "/" + f;
        if (checkFile(r, file)) {
          return r;
        }
        if (checkFile(r, *p + "/inputs/" + f)) {
          return r;
        }
        if (checkFile(r, *p + "/scripts/" + file)) {
          return r;
        }
        if (checkFile(r, *p + "/data/" + file)) {
          return r;
        }
        ++p;
      }
    }
    if (lpath.isEmpty()) {
      const QString root = lpath + "/share/licos/";
      if (checkFile(r, root + f)) {
        return r;
      }
      if (checkFile(r, root + "inputs/" + f)) {
        return r;
      }
      if (checkFile(r, root + "scripts/" + f)) {
        return r;
      }
      if (checkFile(r, root + "data/" + f)) {
        return r;
      }
    }
    this->editor.displayInformativeMessage(
        QObject::tr("file %1 not found").arg(f));
    return "";
  }  // end of LicosMajorMode::getLicosFile

  QString LicosMajorMode::getFileInPath(const QString &f, const QString &path) {
    QStringList paths = QString(path).split(":", QString::SkipEmptyParts);
    QStringList::const_iterator p = paths.begin();
    QStringList::const_iterator pe = paths.end();
    QString r;
    QString d =
        QFileInfo(this->textEdit.getCompleteFileName()).dir().absolutePath();
    if (f.isEmpty()) {
      return r;
    }
    if (checkFile(r, d + QDir::separator() + f)) {
      return r;
    }
    if (checkFile(r, f)) {
      return r;
    }
    while (p != pe) {
      const QString file = *p + "/" + f;
      if (checkFile(r, file)) {
        return r;
      }
      ++p;
    }
    return "";
  }

  struct LicosMajorModeRessourceLoader {
    LicosMajorModeRessourceLoader() : l(getLoader()) {}

   private:
    struct Loader {
      Loader() { Q_INIT_RESOURCE(LicosModeResources); }
    };
    static Loader &getLoader() {
      static Loader l;
      return l;
    }
    Loader l;
  };

  LicosMajorMode::LicosMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
      : MajorModeBase(w, b, t, &t), rlib("'(" + fileNameRegExp() + "\\.so)'") {
    QStringList keys;
    for (const auto &k : LicosSyntaxHighlighter::getKeys()) {
      keys << QString::fromStdString(k);
    }
    for (const auto &bl : LicosSyntaxHighlighter::getBlocks()) {
      const auto bln = QString::fromStdString(bl);
      keys << bln << "EndOf" + bln;
    }
    this->c = new QCompleter(keys, &t);
    this->c->setWidget(&t);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,
                     static_cast<void (QCompleter::*)(const QString &)>(
                         &QCompleter::activated),
                     &t, &TextEditBase::insertCompletion);
    // QWidget *vp = t.widget()->viewport();
    // vp->setCursor(Qt::PointingHandCursor);
    t.setMouseTracking(true);
    // regular expression
    rlib.setMinimal(true);
    // actions
    this->createActions();
  }  // end of LicosMajorMode::LicosMajorMode

  QString LicosMajorMode::getName() const {
    return "Licos";
  }  // end of LicosMajorMode::getName

  QString LicosMajorMode::getDescription() const {
    return "major mode dedicated to the licos input file";
  }  // end of LicosMajorMode::LicosMajorMode

  void LicosMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    new LicosSyntaxHighlighter(d);
  }  // end of LicosMajorMode::setSyntaxHighlighter

  LicosMajorMode::~LicosMajorMode() {
    delete this->c;
  }  // end of LicosMajorMode::~LicosMajorMode()

  QCompleter *LicosMajorMode::getCompleter() {
    return this->c;
  }  // end of LicosMajorMode::getCompleter

  void LicosMajorMode::completeCurrentWord(TextEditBase &t, const QString &w) {
    MajorModeBase::completeCurrentWord(t, w);
    const auto &pblocks = LicosSyntaxHighlighter::getBlocks();
    if (std::find(pblocks.begin(), pblocks.end(), w.toStdString()) !=
        pblocks.end()) {
      QTextCursor tc = t.textCursor();
      int p = tc.position();
      tc.movePosition(QTextCursor::EndOfBlock);
      tc.insertText("\nEndOf" + w + "\n");
      tc.movePosition(QTextCursor::PreviousBlock);
      tc.setPosition(p);
      t.setTextCursor(tc);
    }
  }  // end of LicosMajorModeBase::completeCurrentWord

  void LicosMajorMode::addMaterialProperties() {
    LicosInsertMaterialPropertyDialog d(this->textEdit);
    d.exec();
  }  // end of LicosMajorMode::addMaterialProperties

  void LicosMajorMode::createActions() {
    //     this->msrca = new QAction(QObject::tr("Open mfront source"),
    //     this);
    //     // documentations
    //     const QString &p1 = this->getLicosPath();
    //     const QString &p2 = this->getMFrontMaterialsPath();
    //     if (!p1.isEmpty()) {
    //       QFileInfo fi;
    //       fi.setFile(p1 + QDir::separator()
    //                  +
    //                  "share/doc/licos/description/description.pdf");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->ddoca = new QAction(QObject::tr("licos's
    //         description"), this);
    //         this->ddoca->setData(p1 + QDir::separator()
    //                              +
    //                              "share/doc/licos/description/description.pdf");
    //         this->ddoca->setIcon(QIcon(":/LicosPDFIcon.png"));
    //         this->ddoca->setIconVisibleInMenu(true);
    //       }
    //       fi.setFile(p1 + QDir::separator()
    //                  + "share/doc/licos/user-guide/user-guide.pdf");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->udoca = new QAction(QObject::tr("licos's user
    //         guide"), this);
    //         this->udoca->setData(p1 + QDir::separator()
    //                              +
    //                              "share/doc/licos/user-guide/user-guide.pdf");
    //         this->udoca->setIcon(QIcon(":/LicosPDFIcon.png"));
    //         this->udoca->setIconVisibleInMenu(true);
    //       }
    //       fi.setFile(p1 + QDir::separator()
    //                  +
    //                  "share/doc/licos/tests/TestCaseDocumentation.pdf");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->tdoca =
    //             new QAction(QObject::tr("licos's tests
    //             documentation"), this);
    //         this->tdoca->setData(
    //             p1 + QDir::separator()
    //             + "share/doc/licos/tests/TestCaseDocumentation.pdf");
    //         this->tdoca->setIcon(QIcon(":/LicosPDFIcon.png"));
    //         this->tdoca->setIconVisibleInMenu(true);
    //       }
    //       fi.setFile(p1 + QDir::separator()
    //                  +
    //                  "share/doc/licos/release-notes-1.0/licos-1.0.pdf");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->l10doca =
    //             new QAction(QObject::tr("licos's release note 1.0"),
    //             this);
    //         this->l10doca->setData(
    //             p1 + QDir::separator()
    //             + "share/doc/licos/release-notes-1.0/licos-1.0.pdf");
    //         this->l10doca->setIcon(QIcon(":/LicosPDFIcon.png"));
    //         this->l10doca->setIconVisibleInMenu(true);
    //       }
    //       fi.setFile(p1 + QDir::separator()
    //                  +
    //                  "share/doc/licos/release-notes-1.1/licos-1.1.pdf");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->l11doca =
    //             new QAction(QObject::tr("licos's release note 1.1"),
    //             this);
    //         this->l11doca->setData(
    //             p1 + QDir::separator()
    //             + "share/doc/licos/release-notes-1.1/licos-1.1.pdf");
    //         this->l11doca->setIcon(QIcon(":/LicosPDFIcon.png"));
    //         this->l11doca->setIconVisibleInMenu(true);
    //       }
    //       fi.setFile(p1 + QDir::separator() +
    //       "share/doc/licos/html/index.html");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->idoca =
    //             new QAction(QObject::tr("licos's doxygen
    //             documentation"), this);
    //         this->idoca->setData(p1 + QDir::separator()
    //                              +
    //                              "share/doc/licos/html/index.html");
    //         // this->ddoca->setIcon(QIcon(":/LicosHTMLIcon.png"));
    //         // this->ddoca->setIconVisibleInMenu(true);
    //       }
    //       // tutorials
    //       std::map<QString, QString> tutorials = {
    //           {QObject::tr("Coupling schemes"),
    //           "licos-coupling.pdf"},
    //           {QObject::tr("Postprocessings"),
    //           "licos-postprocessings.pdf"},
    //           {QObject::tr("Gaz Management"), "licos-gas.pdf"},
    //           {QObject::tr("Basic principles"),
    //           "licos-principes.pdf"},
    //           {QObject::tr("Introduction"),
    //           "licos-introduction.pdf"},
    //           {QObject::tr("Python extensions"), "licos-python.pdf"},
    //           {QObject::tr("Loadings"), "licos-loadings.pdf"},
    //           {QObject::tr("Heat transfer and mechanics"),
    //            "licos-thermomechanicalmodel.pdf"},
    //           {QObject::tr("Meshing"), "licos-mesh.pdf"},
    //           {QObject::tr("Time"), "licos-times.pdf"},
    //           {QObject::tr("Misc"), "licos-misc.pdf"},
    //           {QObject::tr("User guide"), "licos-userguide.pdf"},
    //           {QObject::tr("Multiple systems"),
    //           "licos-multitranches.pdf"},
    //           {QObject::tr("Workspace"), "licos-workspace.pdf"}};
    //       for (const auto& t : tutorials) {
    //         fi.setFile(p1 + QDir::separator() +
    //                    "share/doc/licos/tutorial/" + t.second);
    //         if ((fi.exists()) && (fi.isReadable())) {
    //           this->tudoca.push_back(new QAction(t.first, this));
    //           this->tudoca.back()->setData(p1 + QDir::separator() +
    //                                        "share/doc/licos/tutorial/"
    //                                        +
    //                                        t.second);
    //           this->tudoca.back()->setIcon(QIcon(":/LicosPDFIcon.png"));
    //           this->tudoca.back()->setIconVisibleInMenu(true);
    //         }
    //       }
    //     }
    //     if (!p2.isEmpty()) {
    //       QFileInfo fi;
    //       fi.setFile(p2 + QDir::separator()
    //                  +
    //                  "share/doc/mfrontmaterials/description/mfm.pdf");
    //       if ((fi.exists()) && (fi.isReadable())) {
    //         this->mdoca =
    //             new QAction(QObject::tr("MFrontMaterials's
    //             description"), this);
    //         this->mdoca->setData(p2 + QDir::separator()
    //                              +
    //                              "mfrontmaterials/description/mfm.pdf");
    //         this->mdoca->setIcon(QIcon(":/LicosPDFIcon.png"));
    //         this->mdoca->setIconVisibleInMenu(true);
    //       }
    //     }
  }  // end of   LicosMajorMode::createActions()

  void LicosMajorMode::createSearchActions() {
    //     const QString &mpath = this->getMFrontMaterialsPath();
    //     const QString &lpath = this->getLicosStudiesPath();
    //     QString l1 = QObject::tr("Search in *.ple from Licos");
    //     QString l2 = QObject::tr("Search in *.mfront from Licos");
    //     if (!mpath.isEmpty()) {
    //       l1 += "+MFrontMaterials";
    //       l2 += "+MFrontMaterials";
    //     }
    //     if (!lpath.isEmpty()) {
    //       l1 += "+LicosStudies";
    //       l2 += "+LicosStudies";
    //     }
    //     delete this->lsil;
    //     delete this->lsim;
    //     delete this->lsis;
    //     delete this->lsia;
    //     delete this->msil;
    //     delete this->msim;
    //     delete this->msis;
    //     delete this->msia;
    //     this->lsil = new QAction(QObject::tr("Search in *.ple from
    //     Licos"), this);
    //     this->lsim =
    //         new QAction(QObject::tr("Search in *.ple from
    //         MFrontMaterials"), this);
    //     this->lsis =
    //         new QAction(QObject::tr("Search in *.ple from
    //         LicosStudies"), this);
    //     this->lsia = new QAction(l1, this);
    //     this->msil =
    //         new QAction(QObject::tr("Search in *.mfront from Licos"),
    //         this);
    //     this->msim = new QAction(
    //         QObject::tr("Search in *.mfront from MFrontMaterials"),
    //         this);
    //     this->msis =
    //         new QAction(QObject::tr("Search in *.mfront from
    //         LicosStudies"), this);
    //     this->msia = new QAction(l2, this);
  }

  void LicosMajorMode::runLicos() {
    if (this->textEdit.isModified()) {
      auto *input = this->textEdit.getSaveInput();
      QObject::connect(input, &TextEditBase::SaveInput::finished, this,
                       &LicosMajorMode::startLicos);
      this->editor.setUserInput(input);
      return;
    }
    this->startLicos();
  }

  void LicosMajorMode::dryrunLicos() {}  // end of LicosMajorMode::dryrunLicos

  void LicosMajorMode::showMaterialWizard() {
    LicosMaterialWizard w(this->textEdit);
    if (w.exec() == QDialog::Accepted) {
      w.writeMaterial();
    }
  }  // end of LicosMajorMode::showMaterialWizard

  void LicosMajorMode::showThermalBehaviourWizard() {
    QWizard w;
    w.setWindowTitle(QObject::tr("Thermal behaviour wizard"));
    //    w.addPage();
    w.exec();
  }  // end of LicosMajorMode::showThermalBehaviourWizard

  void LicosMajorMode::showMechanicalBehaviourWizard() {
    QWizard w;
    w.setWindowTitle(QObject::tr("Mechanical behaviour wizard"));
    //    w.addPage();
    w.exec();
  }  // end of LicosMajorMode::showMechanicalBehaviourWizard

  void LicosMajorMode::startLicos() {
    QString n = this->textEdit.getCompleteFileName();
    if (n.isEmpty()) {
      this->report(QObject::tr("LicosMajorMode::startLicos: no file name"));
      return;
    }
    LicosStudyOptions o;
    LicosStudyOptionsDialog od(o, &(this->textEdit));
    if (od.exec() == QDialog::Rejected) {
      return;
    }
    const auto &af = QFileInfo(n).absoluteFilePath();
    auto *s = new LicosOutputFrame(this->editor, this->buffer, af, o);
    QObject::connect(s, &LicosOutputFrame::finished, this,
                     &LicosMajorMode::studyFinished);
    this->buffer.attachSecondaryTask(QObject::tr("Licos Output"), s);
  }  // end of LicosMajorMode::runLicos

  void LicosMajorMode::studyFinished(bool s, QString e) {
    if (s) {
      QMessageBox::information(&(this->textEdit), tr("Study succeeded"),
                               tr(""));
    } else {
      QMessageBox::critical(&(this->textEdit), tr("Study failed"), e);
      // look if an usefull information may be retrieved. Errors in
      // the input file have a special signature we try to catch.
      QStringList errors = e.split("\n");
      if (errors.isEmpty()) {
        return;
      }
      QListIterator<QString> pe(errors);
      pe.toBack();
      QRegExp r("^Parsing error at line (\\d+) in file '([\\w\\./]+)'.$");
      QString file;
      int line = -1;
      while (pe.hasPrevious()) {
        const QString &l = pe.previous();
        if (r.indexIn(l) >= 0) {
          file = r.cap(2);
          line = r.cap(1).toInt();
        }
      }
      if (!file.isEmpty()) {
        this->editor.openFile(file);
        this->editor.getCurrentBuffer().getMainFrame().gotoLine(line);
      }
    }
  }  // end of LicosMajorMode::studyFinished

  QMenu *LicosMajorMode::getSpecificMenu() {
    QWidget *t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    QMenu *m = new QMenu(QObject::tr("Licos"), t);
    auto *const ra = m->addAction(QObject::tr("Run Licos"));
    QObject::connect(ra, &QAction::triggered, this, &LicosMajorMode::runLicos);
    auto *const dra = m->addAction(QObject::tr("Dry-run Licos"));
    QObject::connect(dra, &QAction::triggered, this,
                     &LicosMajorMode::dryrunLicos);
    m->addSeparator();
    auto *const ampa = m->addAction(QObject::tr("Add material properties"));
    QObject::connect(ampa, &QAction::triggered, this,
                     &LicosMajorMode::addMaterialProperties);
    QMenu *im = m->addMenu(QObject::tr("Wizards"));
    auto *const mwa = im->addAction(QObject::tr("Material Wizard"));
    QObject::connect(mwa, &QAction::triggered, this,
                     &LicosMajorMode::showMaterialWizard);
    auto *const tbwa = im->addAction(QObject::tr("ThermalBehaviour Wizard"));
    QObject::connect(tbwa, &QAction::triggered, this,
                     &LicosMajorMode::showThermalBehaviourWizard);
    auto *const mbwa = im->addAction(QObject::tr("MechanicalBehaviour Wizard"));
    QObject::connect(mbwa, &QAction::triggered, this,
                     &LicosMajorMode::showMechanicalBehaviourWizard);
    //     if ((this->ddoca != nullptr) || (this->udoca != nullptr) ||
    //         (this->tdoca != nullptr) || (this->l10doca != nullptr) ||
    //         (this->l11doca != nullptr) || (this->idoca != nullptr) ||
    //         (this->mdoca != nullptr) || (!this->tudoca.isEmpty())) {
    //       QMenu *dm = m->addMenu(QObject::tr("Documentations"));
    //       if (this->ddoca != nullptr) {
    //         dm->addAction(this->ddoca);
    //       }
    //       if (this->udoca != nullptr) {
    //         dm->addAction(this->udoca);
    //       }
    //       if (this->tdoca != nullptr) {
    //         dm->addAction(this->tdoca);
    //       }
    //       if (this->l10doca != nullptr) {
    //         dm->addAction(this->l10doca);
    //       }
    //       if (this->l11doca != nullptr) {
    //         dm->addAction(this->l11doca);
    //       }
    //       if (this->idoca != nullptr) {
    //         dm->addAction(this->idoca);
    //       }
    //       if (this->mdoca != nullptr) {
    //         dm->addAction(this->mdoca);
    //       }
    //       if (!this->tudoca.isEmpty()) {
    //         QMenu *tm = dm->addMenu(QObject::tr("Tutorial"));
    //         QVector<QAction *>::const_iterator pt;
    //         for (pt = this->tudoca.begin(); pt != this->tudoca.end();
    //              ++pt) {
    //           tm->addAction(*pt);
    //         }
    //       }
    //       QObject::connect(dm, &QMenu::triggered, this,
    //                        &LicosMajorMode::openDocumentation);
    //     }
    return m;
  }  // end of LicosMajorMode::getSpecificMenu

  void LicosMajorMode::openDocumentation(QAction *a) {
    if (a == nullptr) {
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(a->data().toString()));
  }  // end of LicosMajorMode::openDocumentation

  QIcon LicosMajorMode::getIcon() const {
    static QIcon i(":/LicosIcon.png");
    return i;
  }  // end of LicosMajorMode::getIcon()

  void LicosMajorMode::actionTriggered(QAction *a) {
    //     if (a == nullptr) { return; }
    //     // open in tplot
    //     if (a == this->ta1) {
    //       QString data = this->ta1->data().toString();
    //       if (!data.isEmpty()) {
    //         if (!QProcess::startDetached("tplot", QStringList() <<
    //         data)) {
    //           this->editor.displayInformativeMessage(
    //               QObject::tr("launching tplot failed"));
    //         }
    //       }
    //     }
    //     if (a == this->ta2) {
    //       QString data = this->ta2->data().toString();
    //       if (!data.isEmpty()) {
    //         if (!QProcess::startDetached("tplot", QStringList()
    //                                                   << ("lc:" +
    //                                                   data))) {
    //           this->editor.displayInformativeMessage(
    //               QObject::tr("launching tplot failed"));
    //         }
    //       }
    //     }
    //     // analyse using mfma
    //     if (a == this->mfma) {
    //       this->analyseUsingMFM(this->mfma->data().toString());
    //     }
    //     // open mfront source file
    //     if (a == this->msrca) {
    //       this->openMFrontSource(this->msrca->data().toString());
    //     }
    //     // licos file
    //     if (a == this->lsil) {
    //       const QString &w = this->getLicosPath();
    //       this->search(this->lsil->data().toString(), w, "ple");
    //     }
    //     if (a == this->lsim) {
    //       const QString &w = this->getMFrontMaterialsPath();
    //       this->search(this->lsim->data().toString(), w, "ple");
    //     }
    //     if (a == this->lsis) {
    //       const QString &w = this->getLicosStudiesPath();
    //       this->search(this->lsis->data().toString(), w, "ple");
    //     }
    //     if (a == this->lsia) {
    //       const QString &w1 = this->getLicosPath();
    //       const QString &w2 = this->getMFrontMaterialsPath();
    //       const QString &w3 = this->getLicosStudiesPath();
    //       this->search(this->lsia->data().toString(), w1, "ple");
    //       if (w2 != w1) { this->search(this->lsia->data().toString(),
    //       w2, "ple"); }
    //       if (w3 != w1) { this->search(this->lsia->data().toString(),
    //       w3, "ple"); }
    //     }
    //     // mfront files
    //     if (a == this->msil) {
    //       const QString &w = this->getLicosPath();
    //       this->search(this->msil->data().toString(), w, "mfront");
    //     }
    //     if (a == this->msim) {
    //       const QString &w = this->getMFrontMaterialsPath();
    //       this->search(this->msim->data().toString(), w, "mfront");
    //     }
    //     if (a == this->msis) {
    //       const QString &w = this->getLicosStudiesPath();
    //       this->search(this->msis->data().toString(), w, "mfront");
    //     }
    //     if (a == this->msia) {
    //       const QString &w1 = this->getLicosPath();
    //       const QString &w2 = this->getMFrontMaterialsPath();
    //       const QString &w3 = this->getLicosStudiesPath();
    //       this->search(this->msia->data().toString(), w1, "mfront");
    //       if ((!w2.isEmpty()) && (w2 != w1)) {
    //         this->search(this->msia->data().toString(), w2,
    //         "mfront");
    //       }
    //       if ((!w3.isEmpty()) && (w3 != w1)) {
    //         this->search(this->msia->data().toString(), w3,
    //         "mfront");
    //       }
    //     }
  }

  void LicosMajorMode::openMFrontSource(const QString &f) {
    QFileInfo fi(f);
    if (fi.exists()) {
      if (fi.isReadable()) {
        this->editor.openFile(f);
      } else {
        this->editor.displayInformativeMessage(
            QObject::tr("file '%1' is not readable").arg(f));
        return;
      }
    }
    if (fi.isAbsolute()) {
      this->editor.displayInformativeMessage(
          QObject::tr("file '%1' is not found").arg(f));
      return;
    }
    fi.setFile("mfront/properties/" + f);
    if ((fi.exists()) && (fi.isReadable())) {
      this->editor.openFile(f);
      return;
    }
    fi.setFile("mfront/behaviours/" + f);
    if ((fi.exists()) && (fi.isReadable())) {
      this->editor.openFile(f);
      return;
    }
    fi.setFile("mfront/models/" + f);
    if ((fi.exists()) && (fi.isReadable())) {
      this->editor.openFile(f);
      return;
    }
    const QString &mpath = this->getMFrontMaterialsPath();
    if (!mpath.isEmpty()) {
      QStringList mfiles = this->findFiles(mpath, f);
      QStringList::const_iterator p;
      for (p = mfiles.begin(); p != mfiles.end(); ++p) {
        this->editor.openFile(*p);
      }
    }
    this->editor.displayInformativeMessage(
        QObject::tr("file '%1' not found").arg(f));
  }  // end of LicosMajorMode::openMFrontSource

  void LicosMajorMode::analyseUsingMFM(const QString &l) {
    const char *libpath = ::getenv("LD_LIBRARY_PATH");
    if (libpath == nullptr) {
      this->editor.displayInformativeMessage(
          QObject::tr("LD_LIBRARY_PATH not set"));
      return;
    }
    const QString lib = this->getFileInPath(l, libpath);
    if (lib.isEmpty()) {
      this->editor.displayInformativeMessage(
          QObject::tr("library %1 not found").arg(l));
      return;
    }
    auto &b = this->editor.getCurrentBuffer();
    auto *po = new ProcessOutputFrame(this->editor, b);
    auto &p = po->getProcess();
    QStringList args;
    args << lib;
    p.start("mfm", args);
    b.attachSecondaryTask("*mfm results for " + l + "*", po);
  }  // end of LicosMajorMode::analyseUsingMFM

  void LicosMajorMode::search(const QString &s,
                              const QString &w,
                              const QString &ext) {
    if (w.isEmpty()) {
      return;
    }
    const auto shell = ::getenv("SHELL");
    if (shell == nullptr) {
      this->editor.displayInformativeMessage(QObject::tr("no shell defined"));
    }
    auto &b = this->editor.getCurrentBuffer();
    auto *po = new ProcessOutputFrame(this->editor, b);
    po->setMajorMode("grep output");
    auto &p = po->getProcess();
    p.setWorkingDirectory(w);
    QStringList args;
    args << "-c"
         << "grep -nH -e " + s + " $(find . -name \"*." + ext + "\")";
    p.start(shell, args);
    b.attachSecondaryTask("*search for " + s + "*", po);
  }  // end of LicosMajorMode::search

  void LicosMajorMode::createAnalyseUsingMFMAction(const QString &l) {
    //     this->mfma = new QAction(QObject::tr("Analyse using mfm"),
    //     this);
    //     this->mfma->setData(l);
  }  // end of LicosMajorMode::createAnalyseUsingMFMAction

  void LicosMajorMode::completeContextMenu(QMenu *const m,
                                           const QTextCursor &tc) {
    // using namespace std;
    // using namespace tfel::system;
    // using ELM = ExternalLibraryManager;
    // auto bConnect = false; // connect
    MajorModeBase::completeContextMenu(m, tc);
    //     // looking for a word
    //     QTextCursor mtc(tc);
    //     mtc.select(QTextCursor::WordUnderCursor);
    //     const auto w = mtc.selectedText();
    //     if (!w.isEmpty()) {
    //       const auto &cactions = m->actions();
    //       const auto &lpath = this->getLicosPath();
    //       const auto &mpath = this->getMFrontMaterialsPath();
    //       const auto &spath = this->getLicosStudiesPath();
    //       if (!lpath.isEmpty()) {
    //         this->createSearchActions();
    //         this->lsil->setData(w);
    //         this->msil->setData(w);
    //         if (((lpath == mpath) && (lpath == spath))
    //             || ((mpath.isEmpty()) && (spath.isEmpty()))) {
    //           if (cactions.isEmpty()) {
    //             m->addAction(this->lsil);
    //             m->addAction(this->msil);
    //           } else {
    //             m->insertAction(*(cactions.begin()), this->lsil);
    //             m->insertAction(*(cactions.begin()), this->msil);
    //             m->insertSeparator(*(cactions.begin()));
    //           }
    //           bConnect = true;
    //         } else {
    //           // search menu for licos files
    //           auto *lsm = m->addMenu(QObject::tr("Search in Licos
    //           Input Files"));
    //           auto *msm = m->addMenu(QObject::tr("Search in MFront
    //           Input Files"));
    //           lsm->addAction(this->lsil);
    //           msm->addAction(this->msil);
    //           if (!mpath.isEmpty()) {
    //             this->lsim->setData(w);
    //             this->msim->setData(w);
    //             lsm->addAction(this->lsim);
    //             msm->addAction(this->msim);
    //           }
    //           if (!spath.isEmpty()) {
    //             this->lsis->setData(w);
    //             this->msis->setData(w);
    //             lsm->addAction(this->lsis);
    //             msm->addAction(this->msis);
    //           }
    //           this->lsia->setData(w);
    //           this->msia->setData(w);
    //           lsm->addAction(this->lsia);
    //           msm->addAction(this->msia);
    //           if (cactions.isEmpty()) {
    //             m->insertMenu(nullptr, lsm);
    //             m->insertMenu(nullptr, msm);
    //           } else {
    //             m->insertMenu(*(cactions.begin()), lsm);
    //             m->insertMenu(*(cactions.begin()), msm);
    //             m->insertSeparator(*(cactions.begin()));
    //           }
    //           QObject::connect(lsm, &QMenu::triggered, this,
    //                            &LicosMajorMode::actionTriggered);
    //           QObject::connect(msm, &QMenu::triggered, this,
    //                            &LicosMajorMode::actionTriggered);
    //         }
    //       }
    //     }
    //     // special treatment for txt files
    //     QString data;
    //     QStringList exts;
    //     exts << "txt"
    //          << "data"
    //          << "dat";
    //     data = this->getLicosFile(this->getFileNameUnderCursor(tc,
    //     exts));
    //     if (!data.isEmpty()) {
    //       QTextCursor b(tc);
    //       b.movePosition(QTextCursor::StartOfBlock,
    //       QTextCursor::MoveAnchor);
    //       b.select(QTextCursor::WordUnderCursor);
    //       if ((b.selectedText() == "Curve")
    //           || (b.selectedText() == "MultipleCurves")) {
    //         delete this->ta2;
    //         this->ta2 = new QAction(QObject::tr("Plot using tplot"),
    //         this);
    //         const QList<QAction *> &cactions = m->actions();
    //         this->ta2->setData(data);
    //         if (cactions.isEmpty()) {
    //           m->addAction(this->ta2);
    //         } else {
    //           m->insertAction(*(cactions.begin()), this->ta2);
    //           m->insertSeparator(*(cactions.begin()));
    //         }
    //         bConnect = true;
    //       } else {
    //         delete this->ta1;
    //         this->ta1 = new QAction(QObject::tr("Plot using tplot"),
    //         this);
    //         const QList<QAction *> &cactions = m->actions();
    //         this->ta1->setData(data);
    //         if (cactions.isEmpty()) {
    //           m->addAction(this->ta1);
    //         } else {
    //           m->insertAction(*(cactions.begin()), this->ta1);
    //           m->insertSeparator(*(cactions.begin()));
    //         }
    //         bConnect = true;
    //       }
    //     }
    //     // special treatment for libraries
    //     QString l = this->libraryNameUnderCursor(tc);
    //     if (!l.isEmpty()) {
    //       this->createAnalyseUsingMFMAction(l);
    //       const QList<QAction *> &cactions = m->actions();
    //       if (cactions.isEmpty()) {
    //         m->addAction(this->mfma);
    //       } else {
    //         m->insertAction(*(cactions.begin()), this->mfma);
    //         m->insertSeparator(*(cactions.begin()));
    //       }
    //       bConnect = true;
    //     }
    //     // special treatment for functions
    //     auto *ld = static_cast<LicosData *>(tc.block().userData());
    //     if (ld != nullptr) {
    //       const auto p = this->positionInCurrentBlock(tc);
    //       if ((p >= ld->pos) && (ld->pos + ld->function.size() >= p))
    //       {
    //         auto &elm = ELM::getExternalLibraryManager();
    //         QString src;
    //         auto lib = ld->library;
    //         QFileInfo lfi(lib);
    //         if (!lfi.isAbsolute()) { lib = lfi.fileName(); }
    //         try {
    //           src = QString::fromStdString(
    //               elm.getSource(lib.toStdString(),
    //               ld->function.toStdString()));
    //         } catch (exception &e) {
    //           this->editor.displayInformativeMessage(
    //               QObject::tr("getSource failed :
    //               '%1'").arg(e.what()));
    //         }
    //         if (!src.isEmpty()) {
    //           const auto &cactions = m->actions();
    //           this->msrca->setData(src);
    //           if (cactions.isEmpty()) {
    //             m->addAction(this->msrca);
    //           } else {
    //             m->insertAction(*(cactions.begin()), this->msrca);
    //             m->insertSeparator(*(cactions.begin()));
    //           }
    //           bConnect = true;
    //         }
    //       }
    //     }
    //     if (bConnect) {
    //       QObject::connect(m, &QMenu::triggered, this,
    //                        &LicosMajorMode::actionTriggered);
    //     }
  }

  bool LicosMajorMode::handleShortCut(const int k1,
                                      const Qt::KeyboardModifiers m,
                                      const int k2) {
    if ((k1 == Qt::Key_C) && (m == Qt::ControlModifier) && (k2 == Qt::Key_E)) {
      this->editor.setUserInput(
          new LicosInsertBlock(this->editor, this->textEdit));
      return true;
    }
    if ((k1 == Qt::Key_C) && (m == Qt::ControlModifier) && (k2 == Qt::Key_M)) {
      LicosMaterialWizard w(this->textEdit);
      if (w.exec() == QDialog::Accepted) {
        w.writeMaterial();
      }
    }
    return false;
  }  // end of LicosMajorMode::handleShortCut

  QString LicosMajorMode::getLicosPath() const {
    const char *w = ::getenv("LICOSHOME");
    if (w == nullptr) {
      this->editor.displayInformativeMessage(
          QObject::tr("no LICOSHOME "
                      "environment defined"));
      return "";
    }
    return w;
  }  // end of LicosMajorMode::getLicosPath()

  QString LicosMajorMode::getMFrontMaterialsPath() const {
    const char *w = getenv("MFRONTMATERIALSHOME");
    if (w == nullptr) {
      this->editor.displayInformativeMessage(
          QObject::tr("no MFRONTMATERIALSHOME "
                      "environment defined"));
      return "";
    }
    return w;
  }

  QString LicosMajorMode::getLicosStudiesPath() const {
    const char *w = getenv("LICOSSTUDIESHOME");
    if (w == nullptr) {
      this->editor.displayInformativeMessage(
          QObject::tr("no LICOSSTUDIESHOME "
                      "environment defined"));
      return "";
    }
    return w;
  }

  QString LicosMajorMode::getFileNameUnderCursor(const QTextCursor &tc,
                                                 const QString &e) {
    const QString l = tc.block().text();
    QRegExp i("'(" + fileNameRegExp() + "\\." + e + ")'");
    i.setMinimal(true);
    int index = i.indexIn(l);
    while (index >= 0) {
      int length = i.matchedLength();
      int p = this->positionInCurrentBlock(tc);
      if ((p > index) && (p < index + length)) {
        return i.cap(1);
      }
      index = i.indexIn(l, index + length);
    }
    return "";
  }

  QString LicosMajorMode::getFileNameUnderCursor(const QTextCursor &tc,
                                                 const QStringList &exts) {
    QStringList::const_iterator p;
    for (p = exts.begin(); p != exts.end(); ++p) {
      QString f = this->getFileNameUnderCursor(tc, *p);
      if (!f.isEmpty()) {
        return f;
      }
    }
    return "";
  }

  bool LicosMajorMode::import(const QTextCursor &tc) {
    const QStringList exts = this->getLicosExtensionsSuffix();
    foreach (const QString &e, exts) {
      QString f = this->getFileNameUnderCursor(tc, e);
      if (!f.isEmpty()) {
        if (e == "py") {
          const char *pypath = ::getenv("PYTHONPATH");
          if (pypath != nullptr) {
            const QString file = this->getFileInPath(f, pypath);
            if (!file.isEmpty()) {
              this->editor.openFile(file);
            }
          }
        } else if (e == "mfront") {
          this->openMFrontSource(f);
        } else if (e == "ps") {
          //	  QDesktopServices::openUrl(QUrl::fromLocalFile(f));
        } else {
          const QString file = this->getLicosFile(f);
          if (!file.isEmpty()) {
            this->editor.openFile(file);
          }
        }
        return true;
      }
    }
    return false;
  }

  QString LicosMajorMode::libraryNameUnderCursor(const QTextCursor &tc) {
    const QString l = tc.block().text();
    int index = this->rlib.indexIn(l);
    while (index >= 0) {
      int length = this->rlib.matchedLength();
      int p = this->positionInCurrentBlock(tc);
      if ((p > index) && (p < index + length)) {
        const QString f = this->rlib.cap(1);
        return f;
      }
      index = this->rlib.indexIn(l, index + length);
    }
    return "";
  }  // end of LicosMajorMode::libraryNameUnderCursor

  bool LicosMajorMode::mousePressEvent(QMouseEvent *const e) {
    if (e->buttons() == Qt::LeftButton) {
      if (this->import(this->textEdit.cursorForPosition(e->pos()))) {
        return true;
      }
    }
    return false;
  }  // end of LicosMajorMode::mousePressEvent

  bool LicosMajorMode::keyPressEvent(QKeyEvent *const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if (((m == Qt::ControlModifier) && (k == Qt::Key_M)) ||
        ((m == Qt::NoModifier) && (k == Qt::Key_Return))) {
      if (this->import(this->textEdit.textCursor())) {
        return true;
      }
    } else if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      this->runLicos();
      return true;
    }
    return false;
  }  // end of LicosMajorMode::keyPressEvent

  int LicosMajorMode::getIndentationIncrement(
      QVector<QSharedPointer<LicosBlock>> &blocks,
      LicosTokenizer &tokenizer,
      int &i,
      const QString &line,
      const unsigned short lineNumber) const {
    using namespace std;
    int pi = i;
    bool ao = false;  // array opened
    bool pf = false;  // position found
    try {
      if (tokenizer.getState() == LicosTokenizer::FAILED) {
        i = 0;
        pi = 0;
      } else {
        if (tokenizer.getState() == LicosTokenizer::ARRAYOPENED) {
          pi = tokenizer.getBeginningOfArrays().back() + i + 1;
          ao = true;
        } else {
          if (tokenizer.isCStyleCommentOpened()) {
            pi += 1;
            pf = true;
          }
        }
        tokenizer.parseString(line);
        tokenizer.stripComments();
        if (!ao) {
          auto p = tokenizer.begin();
          const auto pe = tokenizer.end();
          if (p != pe) {
            const auto w = QString::fromStdString(p->value);
            auto &b = *(blocks.back());
            if (w == b.blockEnd()) {
              if (w != "EndOfMain") {
                blocks.pop_back();
                i -= 2;
                if (!pf) {
                  pi -= 2;
                }
              }
            } else if (b.isSubBlock(w)) {
              blocks.append(b.getSubBlock(w));
              ++p;
              if (p != pe) {
                if (p->value == "<") {
                  QStringList args;
                  ++p;
                  while (p->value != ">") {
                    const auto cvalue = QString::fromStdString(p->value);
                    LicosTokenizer::checkNotEndOfLine("indent", p, pe);
                    if (!LicosTokenizer::isValidIdentifier(p->value)) {
                      bool convert;
                      cvalue.toInt(&convert);
                      if (!convert) {
                        tfel::raise(
                            "LicosMajorMode::getIndentationIncrement: "
                            "invalid argument '" +
                            p->value +
                            "').\n"
                            "Error at line '" +
                            std::to_string(lineNumber) + "'");
                      }
                    }
                    args << cvalue;
                    ++p;
                    LicosTokenizer::checkNotEndOfLine("indent", p, pe);
                    if (p->value != ">") {
                      LicosTokenizer::readSpecifiedToken("indent", ",", p, pe);
                      if (p->value == ",") {
                        string msg("indent : ");
                        msg += "unexpected token ','";
                        throw(runtime_error(msg));
                      }
                    }
                  }
                  blocks.back()->treatParameters(args);
                }
              }
              blocks.back()->treatArguments(p, pe);
              i += 2u;
            } else if ((w == "Global") && (blocks.size() == 1u)) {
              ++p;
              LicosTokenizer::checkNotEndOfLine("indent", p, pe);
              if (p->value == "Block") {
                ++p;
                blocks.append(b.getSubBlock("Block"));
                blocks.back()->treatArguments(p, pe);
                i += 2u;
              }
            } else if (!b.isSubKey(w)) {
              qDebug() << "error "
                       << " (" << lineNumber << ") : " << w << " is unsupported"
                       << endl;
            }
          }
        }
      }
    } catch (exception &e) {
      tokenizer.setFailedState(e.what());
      i = 0;
      return 0;
    }
    return pi;
  }  // end of LicosMajorMode::getIndentationIncrement

  int LicosMajorMode::getLineIndentation(
      QVector<QSharedPointer<LicosBlock>> &blocks,
      LicosTokenizer &tokenizer,
      const QTextCursor &tc_) const {
    // getting the line
    QTextCursor tc(tc_);
    QTextCursor cc(tc_);
    tc.clearSelection();
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    int i = 0;
    while (cc < tc) {
      this->getIndentationIncrement(blocks, tokenizer, i, cc.block().text(),
                                    cc.blockNumber() + 1);
      cc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    return i;
  }  // end of LicosMajorMode::getLineIndentation

  void LicosMajorMode::indentLine(const QTextCursor &tc_) {
    using namespace std;
    QTextCursor tc(tc_);
    // find the line indentation
    QVector<QSharedPointer<LicosBlock>> blocks;
    blocks.append(QSharedPointer<LicosBlock>(new LicosMainBlock));
    LicosTokenizer tokenizer;
    // current position in line
    int cp = tc.position();
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cp -= tc.position();
    int i = this->getLineIndentation(blocks, tokenizer, tc);
    tc.select(QTextCursor::LineUnderCursor);
    const auto line = tc.selectedText();
    // number of spaces at the beginning of the line
    int p;
    bool found = false;
    for (p = 0; (p != line.size()) && (!found);) {
      if (line[p].isSpace()) {
        ++p;
      } else {
        found = true;
      }
    }
    // remove spaces
    cp -= p;
    // get real line indentation
    int li = this->getIndentationIncrement(blocks, tokenizer, i, line,
                                           tc.blockNumber());
    if (li <= 0) {
      li = 0;
    }
    // remove selected text
    tc.removeSelectedText();
    tc.insertText(QString(li, ' ') + line.trimmed());
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    if (cp <= 0) {
      tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, li);
    } else {
      tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                      li + cp);
    }
    this->textEdit.setTextCursor(tc);
  }  // end of LicosMajorMode::indentLine

  void LicosMajorMode::indentRegion(const QTextCursor &tc) {
    if (!tc.hasSelection()) {
      return;
    }
    QTextCursor b;
    QTextCursor e;
    this->beginAndEndOfSelection(b, e, tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    e.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    // this is a paranoïd check
    if (b.atEnd()) {
      return;
    }
    if (b == e) {
      this->indentLine(tc);
      return;
    }
    // find the line indentation
    QVector<QSharedPointer<LicosBlock>> blocks;
    blocks.append(QSharedPointer<LicosBlock>(new LicosMainBlock));
    LicosTokenizer tokenizer;
    int i = this->getLineIndentation(blocks, tokenizer, b);
    while ((b <= e) && (!b.atEnd())) {
      b.select(QTextCursor::LineUnderCursor);
      QString line = b.selectedText().trimmed();
      b.removeSelectedText();
      int li = this->getIndentationIncrement(blocks, tokenizer, i, line,
                                             b.blockNumber() + 1);
      b.insertText(QString(li, ' ') + line);
      this->textEdit.setTextCursor(b);
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    this->textEdit.setTextCursor(b);
  }  // end of LicosMajorMode::indentRegion

  void LicosMajorMode::comment() {
    using tfel::utilities::Token;
    QTextCursor tc = this->textEdit.textCursor();
    if (tc.hasSelection()) {
      QTextCursor b;
      QTextCursor btmp;
      QTextCursor e;
      this->beginAndEndOfSelection(b, e, tc);
      if (b == e) {
        return;
      }
      // check if we shall comment and or uncomment the given region
      bool uncomment = false;
      btmp = b;
      b = tc;
      QTextCursor bl(tc);
      b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      bl.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      LicosTokenizer tokenizer;
      while (b < bl) {
        tokenizer.parseString(b.block().text());
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      // here we have the state of the beginning of the selection
      if (tokenizer.getState() == LicosTokenizer::FAILED) {
        this->editor.displayInformativeMessage(QObject::tr("parsing failed"));
        return;
      }
      LicosTokenizer::TokensContainer stokens;  // selected tokens
      if (!tokenizer.isCStyleCommentOpened()) {
        QVector<QString> lines = this->getSelectedLines(tc);
        unsigned short lNumber = 0u;
        uncomment = true;
        if (lines.isEmpty()) {
          uncomment = false;
        }
        for (const QString &line : lines) {
          tokenizer.parseString(line);
          if (tokenizer.getState() == LicosTokenizer::FAILED) {
            this->editor.displayInformativeMessage(
                QObject::tr("parsing failed"));
            return;
          }
          auto p = tokenizer.begin();
          const auto pe = tokenizer.end();
          if (p == pe) {
            stokens.push_back(Token("/**/", lNumber, 0, Token::Comment));
          }
          while ((p != pe) && (uncomment)) {
            if (p->flag != Token::Comment) {
              uncomment = false;
            } else {
              stokens.push_back(*p);
            }
            ++p;
          }
          ++lNumber;
        }
      }
      b = btmp;
      if (uncomment) {
        // uncomment
        auto p = stokens.begin();
        const auto pe = stokens.end();
        tc.beginEditBlock();
        tc.removeSelectedText();
        while (p != pe) {
          if (p == pe) {
            return;
          }
          const auto &w = QString::fromStdString(p->value);
          if (p != stokens.begin()) {
            auto p2 = p;
            --p2;
            if (p2->line != p->line) {
              tc.insertText("\n");
            } else {
              int s = p->offset - p2->offset - p2->value.size();
              if (s > 0) {
                tc.insertText(QString(s, ' '));
              }
            }
          }
          if ((w[0] == '/') && (w[1] == '/')) {
            // C++ comment
            if ((w.size() >= 3) && (w[2] == ' ')) {
              tc.insertText(w.mid(3, w.size() - 3));
            } else {
              tc.insertText(w.mid(2, w.size() - 2));
            }
          } else if ((w[0] == '/') && (w[1] == '*')) {
            // C comment
            tc.insertText(w.mid(2, w.size() - 4));
          }
          ++p;
        }
        tc.endEditBlock();
        this->textEdit.setTextCursor(tc);
      } else {
        // comment
        b.beginEditBlock();
        while ((b < e) && (!b.atEnd())) {
          int d = e.position() - b.position();
          if ((b.blockNumber() == e.blockNumber()) && (!e.atBlockStart())) {
            b.insertText("// ");
            b.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                           d);
            b.insertText("\n");
            break;
          } else {
            b.insertText("// ");
            e = b;
            e.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                           d);
          }
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
          b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        }
        b.endEditBlock();
        this->textEdit.setTextCursor(b);
      }
    } else {
      QTextCursor b(tc);
      QTextCursor bl(tc);
      b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      bl.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      LicosTokenizer tokenizer;
      while (b < bl) {
        tokenizer.parseString(b.block().text());
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      // tokenizer has the state at the beginning of the line
      QString l = b.block().text();
      if (tc.position() - bl.position() > 0) {
        tokenizer.parseString(l.mid(0, tc.position() - bl.position()));
      }
      if (tokenizer.getState() != LicosTokenizer::FAILED) {
        if (tokenizer.isCStyleCommentOpened()) {
          return;
        }
        if (tokenizer.begin() != tokenizer.end()) {
          const auto &t = *(tokenizer.end());
          if (t.flag == Token::Comment) {
            return;
          }
        }
      }
      // analysing the whole of the line
      if (tokenizer.getState() != LicosTokenizer::FAILED) {
        b = tc;
        tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        if (tc != b) {
          tokenizer.parseString(
              l.mid(b.position(), tc.position() - b.position()));
        }
      } else {
        tokenizer.reset();
        b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        while (b < bl) {
          QString cl = b.block().text();
          tokenizer.parseString(cl);
          if (tokenizer.getState() == LicosTokenizer::FAILED) {
            this->editor.displayInformativeMessage(
                QObject::tr("parsing failed : %1 (%2)")
                    .arg(tokenizer.getErrorMessage())
                    .arg(cl));
            return;
          }
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
        tokenizer.parseString(l);
        if (tokenizer.getState() == LicosTokenizer::FAILED) {
          this->editor.displayInformativeMessage(
              QObject::tr("parsing failed : %1 (%2)")
                  .arg(tokenizer.getErrorMessage())
                  .arg(l));
          return;
        }
        tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      }
      if (tokenizer.getState() == LicosTokenizer::FAILED) {
        this->editor.displayInformativeMessage(
            QObject::tr("parsing failed : %1")
                .arg(tokenizer.getErrorMessage()));
        return;
      }
      if (tokenizer.isCStyleCommentOpened()) {
        this->textEdit.setTextCursor(tc);
        return;
      }
      if (tokenizer.begin() != tokenizer.end()) {
        const Token &t = *(tokenizer.end());
        if (t.flag == Token::Comment) {
          this->textEdit.setTextCursor(tc);
          return;
        }
      }
      if (l.isEmpty()) {
        tc.insertText("// ");
      } else {
        if (l[l.size() - 1].isSpace()) {
          tc.insertText("// ");
        } else {
          tc.insertText(" // ");
        }
      }
      this->textEdit.setTextCursor(tc);
    }
  }  // end of LicosMajorMode::comment

  void LicosMajorMode::format() {}  // end of LicosMajorMode::format

  static StandardMajorModeProxy<LicosMajorMode> proxy(
      "licos",
      QVector<QRegExp>() << QRegExp("^" + fileNameRegExp() + "\\.ple"),
      ":/LicosIcon.png");

}  // end of namespace tfel::gui
