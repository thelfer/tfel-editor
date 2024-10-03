/*!
 * \file  PlainTextEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   19/08/2012
 */

#include <fstream>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringListModel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QListView>
#include <QtWidgets/QDialogButtonBox>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include "TFEL/System/System.hxx"
#include "TFEL/GUI/utf8/utf8.h"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/DirectoryMajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"

namespace tfel::gui {

  PlainTextEdit::PlainTextEdit(EditorWidget& g, Buffer& b)
      : TextEditBase(g, b), e(new QPlainTextEdit(this)) {
    this->initialize(this->e);
#pragma message("warning")
    auto* hl = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl, this->e);
    //    this->e->setTabStopWidth(40);
    this->e->setWordWrapMode(QTextOption::WrapAnywhere);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setLayout(hl);
    QObject::connect(this->e->document(), &QTextDocument::modificationChanged,
                     [this](const bool c) { emit modificationChanged(c); });
  }  // end of PlainTextEdit()

  PlainTextEdit::PlainTextEdit(const QString& f, EditorWidget& g, Buffer& b)
      : TextEditBase(g, b), e(new QPlainTextEdit(this)) {
    this->initialize(this->e);
    auto* hl = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl, this->e);
    //    this->e->setTabStopWidth(40);
    this->e->setWordWrapMode(QTextOption::WrapAnywhere);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->e->setContextMenuPolicy(Qt::NoContextMenu);
    this->setLayout(hl);
    // reading the file
    QFileInfo fn(f);
    if ((fn.exists()) && (fn.isDir())) {
      this->setMajorMode(
          new DirectoryMajorMode(f, this->editor, this->buffer, *this));
      this->allowMajorModeChange = false;
    } else if ((fn.exists()) && (!fn.isDir())) {
      QFile file(f);
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QObject::tr("::openFile"),
                              QObject::tr("opening file '%1' failed").arg(f));
        return;
      }
      // check if utf8
      QTextStream in(&file);
      in.setAutoDetectUnicode(true);
      in.setEncoding(QStringConverter::Utf8);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      this->setPlainText(in.readAll());
      QApplication::restoreOverrideCursor();
      //      this->document()->setModified(!u);
      this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
      // setting file name
      this->setFileName(f);
      // see if we saved the file position
      QSettings s;
      if (s.contains("positions in files")) {
        auto pl =
            s.value("positions in files").value<QMap<QString, QVariant>>();
        if (pl.contains(this->getCompleteFileName())) {
          this->gotoLine(pl[this->getCompleteFileName()].toInt());
        }
      }
    } else {
      if (!f.isEmpty()) {
        this->readTemplateFile(f);
      }
      this->setFileName(f);
    }
  }  // end of PlainTextEdit()

  QAbstractScrollArea* PlainTextEdit::widget() { return this->e; }

  void PlainTextEdit::print() {
    QPrinter printer;
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
      this->e->print(&printer);
    }
  }  // end of print

  void PlainTextEdit::undo() { this->e->undo(); }  // end of undo

  void PlainTextEdit::redo() { this->e->redo(); }  // end of redo

  void PlainTextEdit::cut() { this->e->cut(); }

  void PlainTextEdit::paste() { this->e->paste(); }

  void PlainTextEdit::selectAll() { this->e->selectAll(); }

  void PlainTextEdit::copy() { this->e->copy(); }

  void PlainTextEdit::clear() { this->e->clear(); }

  bool PlainTextEdit::isReadOnly() { return this->e->isReadOnly(); }

  void PlainTextEdit::appendPlainText(const QString& t) {
    return this->e->appendPlainText(t);
  }

  void PlainTextEdit::insertPlainText(const QString& t) {
    return this->e->insertPlainText(t);
  }

  void PlainTextEdit::setPlainText(const QString& t) {
    return this->e->setPlainText(t);
  }

  void PlainTextEdit::appendHtml(const QString& t) {
    return this->e->appendHtml(t);
  }

  void PlainTextEdit::insertHtml(const QString& t) {
    return this->e->textCursor().insertHtml(t);
  }

  void PlainTextEdit::setHtml(const QString& t) {
    this->clear();
    this->insertHtml(t);
  }

  QTextCursor PlainTextEdit::cursorForPosition(const QPoint& p) const {
    return this->e->cursorForPosition(p);
  }

  QRect PlainTextEdit::cursorRect(const QTextCursor& tc) const {
    return this->e->cursorRect(tc);
  }

  void PlainTextEdit::setUndoRedoEnabled(bool b) {
    return this->e->setUndoRedoEnabled(b);
  }

  void PlainTextEdit::setReadOnly(bool b) { this->e->setReadOnly(b); }

  void PlainTextEdit::moveCursor(QTextCursor::MoveOperation mo,
                                 QTextCursor::MoveMode mm) {
    return this->e->moveCursor(mo, mm);
  }

  QTextDocument* PlainTextEdit::document() const {
    return this->e->document();
  }  // end of document

  void PlainTextEdit::setExtraSelections(
      const QList<QTextEdit::ExtraSelection>& l) {
    return this->e->setExtraSelections(l);
  }

  QTextCursor PlainTextEdit::textCursor() const {
    return this->e->textCursor();
  }

  QList<QTextEdit::ExtraSelection> PlainTextEdit::extraSelections() const {
    return this->e->extraSelections();
  }  // end of extraSelections

  QRect PlainTextEdit::cursorRect() const { return this->e->cursorRect(); }

  bool PlainTextEdit::find(const QString& t, QTextDocument::FindFlags f) {
    return this->e->find(t, f);
  }

  bool PlainTextEdit::isUndoRedoEnabled() const {
    return this->e->isUndoRedoEnabled();
  }

  void PlainTextEdit::setTextCursor(const QTextCursor& tc) {
    return this->e->setTextCursor(tc);
  }

  void PlainTextEdit::centerCursor() {
    return this->e->centerCursor();
  }  // end of centerCursor

  void PlainTextEdit::setTextInteractionFlags(Qt::TextInteractionFlags f) {
    this->e->setTextInteractionFlags(f);
  }

  void PlainTextEdit::readTemplateFile(const QString& f) {
    // reading the template file associated with the file
    QFileInfo i(f);
    const auto ext = i.completeSuffix();
    const auto homePath =
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
    QFile t_file(homePath + "/.templates/TEMPLATE." + ext + ".tpl");
    if (!t_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;
    }
#ifdef Q_OS_UNIX
    QString user = getUserName();
    QString login = getLoginName();
#endif
    QTime t = QTime::currentTime();
    QString time = QString::number(t.hour()) + ":" +
                   QString::number(t.minute()) + QString::number(t.second());

    QDate d = QDate::currentDate();
    QString date = d.toString("dd/MM/yyyy");
    QString iso_date = d.toString(Qt::ISODate);
    QString vc_date = iso_date + " " + time;

    QString file = i.fileName();
    QString file_sans = i.baseName();
    QString file_upcase = file_sans.toUpper();
    QString dir = i.dir().absolutePath();

    QString file_raw;
    QString file_num;
    QRegularExpression expr("(.*)(\\d.*)");

    const auto match = expr.match(file_sans);
    if (match.hasMatch()) {
      file_raw = match.captured(1);
      file_num = match.captured(2);
    } else {
      file_raw = file_sans;
    }
    QTextStream in(&t_file);
    while (!in.atEnd()) {
      QString l = in.readLine();
#ifdef Q_OS_UNIX
      l.replace("(>>>AUTHOR<<<)", user);
      l.replace("(>>>USER_NAME<<<)", login);
      l.replace("(>>>LOGIN_NAME<<<)", login);
#endif
      l.replace("(>>>FILE<<<)", file);
      l.replace("(>>>FILE_RAW<<<)", file_raw);
      l.replace("(>>>FILE_UPCASE<<<)", file_upcase);
      l.replace("(>>>FILE_EXT<<<)", ext);
      l.replace("(>>>FILE_SANS<<<)", file_sans);
      l.replace("(>>>DIR<<<)", dir);
      l.replace("(>>>TIME<<<)", time);
      l.replace("(>>>DATE<<<)", date);
      l.replace("(>>>ISO_DATE<<<)", iso_date);
      l.replace("(>>>VC_DATE<<<)", vc_date);
      l.replace("(>>>COMMENT<<<)", "");
      this->insertPlainText(l + "\n");
    }
    this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    if (this->find("(>>>POINT<<<)")) {
      QTextCursor c = this->textCursor();
      c.removeSelectedText();
      while (this->find("(>>>POINT<<<)")) {
        c = this->textCursor();
        c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        c.removeSelectedText();
      }
    } else {
      this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    }
    this->document()->setModified(false);
  }  // end of readTemplateFile

  PlainTextEdit::~PlainTextEdit() {
    const auto& f = this->getCompleteFileName();
    QSettings s;
    QMap<QString, QVariant> pl;
    if (s.contains("positions in files")) {
      pl = s.value("positions in files").value<QMap<QString, QVariant>>();
    }
    const auto& c = this->textCursor();
    pl[f] = c.blockNumber() + 1;
    s.setValue("positions in files", pl);
  }  // end of ~PlainTextEdit

}  // end of namespace tfel::gui
