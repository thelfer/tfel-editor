/*!
 * \file   QEmacsWidget.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/06/2012
 */

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QCloseEvent>

#include "QEmacs/QEmacsCommand.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/QEmacsPlainTextEdit.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsWidget.hxx"

struct QEmacsModeRessourceLoader {
  QEmacsModeRessourceLoader() : l(getLoader()) {}

 private:
  struct Loader {
    Loader() { Q_INIT_RESOURCE(QEmacsResources); }
  };
  static Loader& getLoader() {
    static Loader l;
    return l;
  }
  Loader l;
};

namespace qemacs {

  static void setQLineEditFont(QLineEdit* l) {
    QFont f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 0);
  }

  static void setQEmacsLineEditFont(QEmacsLineEdit* l) {
    QFont f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 0);
  }

  struct QEmacsWidget::OpenFile : public QEmacsFilePathUserInput {
    OpenFile(QEmacsWidget& p, const QString& path)
        : QEmacsFilePathUserInput(QObject::tr("open file :"), p) {
      QString npath(path);
      if (!npath.isEmpty()) {
        if (!npath.endsWith(QDir::separator())) {
          npath += QDir::separator();
        }
      }
      this->input->setText(npath);
    }

    ~OpenFile() override = default;

   protected:
    void treatUserInput() override {
      this->qemacs.openFile(this->input->text());
    }

  };  // end of struct QEmacsTextEdit::OpenFile

  struct QEmacsWidget::ChangeBuffer : public QEmacsLineEdit {
    ChangeBuffer(QEmacsWidget& p,
                 const QStringList& bn,
                 const QString& d)
        : QEmacsLineEdit(
              QObject::tr("switch to buffer") +
                  (!d.isEmpty() ? QObject::tr("(default '%1')").arg(d)
                                : "") +
                  QObject::tr(" :"),
              p),
          defaultBuffer(d) {
      QStringList b(bn);
      if (!this->defaultBuffer.isEmpty()) {
        b << "";  // add the emtpy string for default completion
      }
      auto* c = new QCompleter(bn, &p);
      c->setWidget(this->input);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(c, false);
    }

    ~ChangeBuffer() override = default;

   protected:
    void treatUserInput() override {
      const auto t = this->input->text();
      if (t.isEmpty()) {
        this->qemacs.changeBuffer(this->defaultBuffer);
      } else {
        this->qemacs.changeBuffer(t);
      }
    }

    QString defaultBuffer;

  };  // end of struct QEmacsTextEdit::ChangeBuffer

  struct QEmacsWidget::Command : public QEmacsLineEdit {
    Command(QEmacsWidget& p)
        : QEmacsLineEdit(QObject::tr("qemacs command :"), p) {
      auto& f = QEmacsCommandFactory::getQEmacsCommandFactory();
      auto* c = new QCompleter(f.getAvailableQEmacsCommandsNames(), &p);
      c->setWidget(this->input);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(c, false);
    }

    ~Command() override = default;

   protected:
    void treatUserInput() override {
      // the input must be kept before calling the removeUserInput
      // method (this causes the qemacswdget to delete this).
      QString i = this->input->text();
      this->qemacs.removeUserInput(this);
      this->qemacs.launchCommand(i);
    }

  };  // end of struct QEmacsTextEdit::Command

  QStringList& QEmacsWidget::getRecentFiles() {
    static QStringList files;
    return files;
  }  // end of QEmacsWidget::getRecentFiles

  void QEmacsWidget::addToRecentFiles(const QString& f) {
    if (f.isEmpty()) {
      return;
    }
    QStringList& files = QEmacsWidget::getRecentFiles();
    files.append(f);
    if (files.size() > 100) {
      files.pop_front();
    }
  }  // end of QEmacsWidget::addToRecentFiles

  QEmacsWidget::QEmacsWidget(QWidget* const p)
      : QWidget(p),
        buffers(new QStackedWidget),
        minibuffer(new QStackedWidget),
        um(new QLineEdit),
        ui(0),
        eui(new QLineEdit),
        nid(0) {
    QEmacsModeRessourceLoader loader;
    auto* vl = new QVBoxLayout;
    this->buffers->setFocusPolicy(Qt::NoFocus);
    this->setFocusPolicy(Qt::NoFocus);
    this->um->setReadOnly(true);
    this->eui->setReadOnly(true);
    setQLineEditFont(this->um);
    setQLineEditFont(this->eui);
    this->minibuffer->addWidget(this->um);
    this->minibuffer->addWidget(this->eui);
    this->minibuffer->setContentsMargins(0, 0, 0, 0);
    QSizePolicy s;
    s.setHorizontalPolicy(QSizePolicy::Minimum);
    s.setVerticalPolicy(QSizePolicy::Maximum);
    this->minibuffer->setSizePolicy(s);
    vl->setMargin(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(this->buffers);
    vl->addWidget(this->minibuffer);
    vl->setSpacing(0);
    this->setLayout(vl);
  }  // end of QEmacsWigdet::QEmacsWigdet

  void QEmacsWidget::changeMainFramesFont(const QFont& f) {
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      auto& t = b->getMainFrame();
      t.setFont(f);
    }
  }  // end of QEmacsWidget::changeMainFrameFont

  QEmacsBuffer* QEmacsWidget::getBufferVisitingFile(const QString& f) {
    QFileInfo fi(f);
    QString af;
    if (fi.isSymLink()) {
      af = fi.symLinkTarget();
    } else {
      af = fi.absoluteFilePath();
    }
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      QFileInfo bfi(b->getBufferRawName());
      QString bf;
      if (bfi.isSymLink()) {
        bf = bfi.symLinkTarget();
      } else {
        bf = bfi.absoluteFilePath();
      }
      if (bf == af) {
        return b;
      }
    }
    return nullptr;
  }  // end of QEmacsWidget::getBufferVisitingFile

  void QEmacsWidget::openFile(const QString& f) {
    if (f.isEmpty()) {
      return;
    }
    auto* b = this->createNewBuffer(f);
    if (this->buffers->indexOf(b) == -1) {
      this->buffers->addWidget(b);
      emit bufferAdded();
    }
    this->setCurrentBuffer(b);
  }  // end of QEmacs::openFile(const QString&)

  void QEmacsWidget::openFile() {
    auto& b = this->getCurrentBuffer();
    auto& t = b.getMainFrame();
    QFileInfo fn(t.getCompleteFileName());
    QDir d(fn.dir());
    QString path;
    if (d.exists()) {
      path = d.absolutePath();
    } else {
      path = QDir::current().absolutePath();
    }
    auto* l = new QEmacsWidget::OpenFile(*this, path);
    l->setInputHistorySettingAddress("recent files");
    this->setUserInput(l);
  }  // end of QEmacs::openFile()

  void QEmacsWidget::createEmptyBuffer() {
    QEmacsBuffer* b = this->createNewBuffer();
    this->buffers->addWidget(b);
    this->setCurrentBuffer(b);
    emit bufferAdded();
  }  // end of QEmacs::createEmptyBuffer()

  QEmacsBuffer* QEmacsWidget::createNewBuffer(const QString& f) {
    QEmacsBuffer* b;
    if (f.isEmpty()) {
      b = new QEmacsBuffer(this->nid, *this);
      ++(this->nid);
    } else {
      b = this->getBufferVisitingFile(f);
      if (b != nullptr) {
        this->displayInformativeMessage(
            QObject::tr("A buffer is already visiting file '%1'")
                .arg(f));
        return b;
      }
      b = new QEmacsBuffer(f, this->nid, *this);
      ++(this->nid);
      this->emitNewTreatedFile(b->getMainFrame().getCompleteFileName());
    }
    QObject::connect(b, &QEmacsBuffer::bufferNameChanged, this,
                     &QEmacsWidget::updateBufferName);
    QObject::connect(b, &QEmacsBuffer::newTreatedFile, this,
                     &QEmacsWidget::emitNewTreatedFile);
    return b;
  }  // end of QEmacsWidget::createNewBuffer

  void QEmacsWidget::removeBuffer(QEmacsBuffer* b) {
    const auto n = b->getBufferName();
    this->bHistory.removeAll(n);
    this->buffers->removeWidget(b);
    // removing the buffer in the secondaryTasks
    auto p = this->secondaryTasks.begin();
    while(p!=this->secondaryTasks.end()){
      const auto pbe = p->second.end();
      auto pb = std::find(p->second.begin(),pbe,b);
      if(pb!=pbe){
        p->second.erase(pb);
      }
      if (p->second.empty()) {
        p->first->deleteLater();
        p = this->secondaryTasks.erase(p);
      } else {
        ++p;
      }
    }
    // creating empty buffer is necessary
    if (this->buffers->count() == 0) {
      this->createEmptyBuffer();
    } else {
      auto cw = this->buffers->currentWidget();
      cw->setFocus();
      if (!this->bHistory.isEmpty()) {
        this->changeBuffer(this->bHistory.front());
      } else {
        auto* nb = qobject_cast<QEmacsBuffer*>(cw);
        if (nb != nullptr) {
          this->setCurrentBuffer(nb);
        }
      }
    }
    b->deleteLater();
    emit bufferRemoved();
  }  // end of QEmacsWidget::removeBuffer

  void QEmacsWidget::updateBufferName(QEmacsBuffer* b,
                                      const QString& o,
                                      const QString& n) {
    int p = this->buffers->indexOf(b);
    if (p == -1) {
      return;
    }
    for (int i = 0; i < this->bHistory.size(); ++i) {
      if (this->bHistory.at(i) == o) {
        this->bHistory[i] = n;
      }
    }
    emit bufferNameChanged();
  }

  void QEmacsWidget::saveAllBuffers() {
    int i;
    for (i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      auto& t = b->getMainFrame();
      if (t.document()->isModified()) {
        t.save();
      }
    }
  }  // end of QEmacsWidget::saveAllBuffers()

  bool QEmacsWidget::isOkToClose() {
    const auto unsaved = [this] {
      QStringList bl;
      for (int i = 0; i != this->buffers->count(); ++i) {
        auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
        if (b == nullptr) {
          continue;
        }
        if (!b->isOkToClose()) {
          const auto n = b->getBufferName();
          if (n.isEmpty()) {
            bl << "<unamed>";
          } else {
            bl << n;
          }
        }
      }
      return bl;
    }();
    if (!unsaved.empty()) {
      auto msg = QObject::tr(
          "At least one buffer has "
          "been modified.\n");
      int i = 0;
      for (const auto& f : unsaved) {
        msg += "- " + f + "\n";
        if (i > 5) {
          break;
        }
        ++i;
      };
      msg += QObject::tr("Do you want to save your changes?");
      const auto ret = QMessageBox::warning(
          this, QObject::tr("QEmacs"), msg, QMessageBox::SaveAll |
                                                QMessageBox::Discard |
                                                QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel) {
        return false;
      } else if (ret == QMessageBox::SaveAll) {
        this->saveAllBuffers();
      }
    }
    return true;
  }  // end of QEmacsWidget::isOkToClose

  void QEmacsWidget::closeEvent(QCloseEvent* e) {
    if (this->isOkToClose()) {
      // for(int i=0;i!=this->buffers->count();++i){
      // 	QEmacsBuffer *b =
      // qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      // 	QCloseEvent cev;
      // 	cev.setAccepted(true);
      // 	b->closeEvent(&cev);
      // 	if(!cev.isAccepted()){
      // 	  e->ignore();
      // 	}
      // }
      e->accept();
      emit closed();
    } else {
      e->ignore();
    }
  }  // end of QEmacsWidget::close

  void QEmacsWidget::displayInformativeMessage(const QString& m) {
    this->um->setText(m);
    this->minibuffer->setCurrentWidget(this->um);
    QTimer::singleShot(1000, this, &QEmacsWidget::resetUserInput);
  }  // end of QEmacsWidget::displayInformativeMessage

  void QEmacsWidget::setUserInput(QEmacsLineEdit* const l) {
    if (l == nullptr) {
      return;
    }
    if (this->ui.contains(l)) {
      return;
    }
    if (!this->ui.isEmpty()) {
      if (this->ui.back()->isBlocking()) {
        this->displayInformativeMessage(
            QObject::tr("command attemted "
                        "to use minibuffer "
                        "while in minibuffer"));
        return;
      }
    }
    this->ui.append(l);
    setQEmacsLineEditFont(this->ui.back());
    this->minibuffer->addWidget(this->ui.back());
    this->minibuffer->setCurrentWidget(this->ui.back());
    this->ui.back()->setFocus();
    if (this->ui.back()->isBlocking()) {
      this->buffers->setEnabled(false);
    }
    QObject::connect(
        this->ui.back(), &QEmacsLineEdit::finished, this,
        static_cast<void (QEmacsWidget::*)(QEmacsLineEdit*)>(
            &QEmacsWidget::removeUserInput));
    QObject::connect(
        this->ui.back(), &QEmacsLineEdit::destroyed, this,
        static_cast<void (QEmacsWidget::*)(QEmacsLineEdit*)>(
            &QEmacsWidget::removeUserInput));
  }

  void QEmacsWidget::removeUserInput(QEmacsLineEdit* p) {
    if (this->ui.isEmpty()) {
      return;
    }
    if (!this->ui.contains(p)) {
      return;
    }
    if (p->isBlocking()) {
      Q_ASSERT(p == this->ui.back());
      this->buffers->setEnabled(true);
    }
    this->minibuffer->removeWidget(p);
    this->ui.remove(this->ui.indexOf(p));
    if (this->ui.isEmpty()) {
      this->minibuffer->setCurrentWidget(this->eui);
    } else {
      this->minibuffer->setCurrentWidget(this->ui.back());
    }
    this->buffers->currentWidget()->setFocus();
    p->deleteLater();
  }  // end of QEmacsWidget::removeUserInput

  void QEmacsWidget::removeUserInput() {
    if (this->ui.isEmpty()) {
      return;
    }
    this->removeUserInput(this->ui.back());
  }  // end of QEmacsWidget::removeUserInput

  void QEmacsWidget::removeUserInputs() {
    while (!this->ui.isEmpty()) {
      this->removeUserInput(this->ui.back());
    }
  }  // end of QEmacsWidget::removeUserInputs

  bool QEmacsWidget::hasUserInput() const {
    return !this->ui.isEmpty();
  }

  void QEmacsWidget::focusUserInput() {
    if (!this->ui.isEmpty()) {
      this->ui.back()->setFocus();
    }
  }

  void QEmacsWidget::resetUserInput() {
    if (!this->ui.isEmpty()) {
      this->minibuffer->setCurrentWidget(this->ui.back());
    } else {
      this->minibuffer->setCurrentWidget(this->eui);
    }
    this->um->clear();
  }  // end of QEmacsWidget::resetPreviousUserInput

  const QStringList& QEmacsWidget::getKillRing() const {
    return this->killRing;
  }  // end of QEmacsWidget::getKillRing

  void QEmacsWidget::addToKillRing(const QString& t) {
    if (t.isEmpty()) {
      return;
    }
    if (this->killRing.size() == 100) {
      this->killRing.pop_front();
    }
    this->killRing << t;
  }  // end of QEmacsWidget::addToKillRing

  QStringList QEmacsWidget::getBuffersNames() const {
    QStringList n;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        const auto bn = b->getBufferName();
        if (bn.isEmpty()) {
          n.append("* unnamed buffer *");
        } else {
          n.append(bn);
        }
      }
    }
    return n;
  }  // end of QEmacsWidget::getBuffersNames

  QVector<QIcon> QEmacsWidget::getBuffersIcons() const {
    QVector<QIcon> icons;
    for (int i = 0; i != this->buffers->count(); ++i) {
      QEmacsBuffer const* b =
          qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        icons.append(b->getIcon());
      }
    }
    return icons;
  }

  QVector<int> QEmacsWidget::getBuffersIds() const {
    QVector<int> ids;
    for (int i = 0; i != this->buffers->count(); ++i) {
      QEmacsBuffer const* b =
          qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        ids.append(b->getId());
      }
    }
    return ids;
  }

  void QEmacsWidget::changeBuffer(const QString& n) {
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        if (b->getBufferName() == n) {
          this->setCurrentBuffer(b);
          return;
        }
      }
    }
  }  // end of QEmacsWidget::changeBuffer

  void QEmacsWidget::changeBuffer(const int id) {
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        if (b->getId() == id) {
          this->setCurrentBuffer(b);
          return;
        }
      }
    }
  }  // end of QEmacsWidget::changeBuffer

  QString QEmacsWidget::chooseBufferNameSuffix(QEmacsBuffer* b,
                                               const QString& f) {
    if (f.isEmpty()) {
      return "";
    }
    QFileInfo info(f);
    const auto n = info.fileName();
    QString s;
    int m = 0;
    bool found = true;
    while (found) {
      found = false;
      for (int i = 0; (i != this->buffers->count()) && (!found); ++i) {
        auto* ob =
            qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
        if (ob != nullptr) {
          if (ob != b) {
            found = ((n == ob->getBufferName()) &&
                     (s == ob->getBufferNameSuffix()));
          }
        }
      }
      if (found) {
        ++m;
        s = QString::number(m);
      }
    }
    return s;
  }  // end of QEmacsWidget::chooseBufferName

  void QEmacsWidget::changeBuffer() {
    auto* cb =
        qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    if (cb == nullptr) {
      return;
    }
    const auto& bn = cb->getBufferName();
    QStringList bnames;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        const auto& n = b->getBufferName();
        if (bn != n) {
          bnames.append(n);
        }
      }
    }
    if (bnames.empty()) {
      this->displayInformativeMessage(QObject::tr("no other buffer"));
      return;
    }
    if (bnames.size() == 1) {
      this->changeBuffer(bnames[0]);
      return;
    }
    if (this->bHistory.size() >= 2) {
      const auto nb = this->bHistory[1];
      this->setUserInput(new ChangeBuffer(*this, bnames, nb));
    } else {
      this->setUserInput(new ChangeBuffer(*this, bnames, ""));
    }
  }  // end of QEmacsWidget::changeBuffer

  void QEmacsWidget::setCurrentBuffer(QEmacsBuffer* const b) {
    if (b == nullptr) {
      return;
    }
    this->buffers->setCurrentWidget(b);
    b->refreshSecondaryTaskTabWidget();
    b->setFocus();
    const auto n = b->getBufferName();
    this->bHistory.removeAll(n);
    this->bHistory.push_front(n);
    emit currentBufferChanged();
  }  // end of QEmacsWidget::setCurrentBuffer

  struct QEmacsWidget::SaveCurrentBufferAndKill
      : public QEmacsYesOrNoUserInput {
    SaveCurrentBufferAndKill(QEmacsWidget& p)
        : QEmacsYesOrNoUserInput(
              QObject::tr(
                  "buffer '%1' has been modified; kill anyway ?")
                  .arg(p.getCurrentBuffer().getBufferName()),
              p),
          t(p) {}
    bool isBlocking() const override {
      return true;
    }  // end of isBlocking
   protected:
    void treatUserInput() override {
      if (this->input->text() == "y") {
        this->t.removeBuffer(&(this->buffer));
      }
    }  // end of treatUserInput
    QEmacsWidget& t;
  };

  QVector<QMenu*> QEmacsWidget::getCurrentBufferSpecificMenus() {
    if (this->buffers->count() == 0) {
      return QVector<QMenu*>();
    }
    auto* b =
        qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    return b->getSpecificMenus();
  }  // end of QEmacsWidget::getCurrentBufferMenu()

  void QEmacsWidget::emitNewTreatedFile(const QString& f) {
    emit newTreatedFile(f);
  }  // end of QEmacsWidget::emitNewTreatedFile

  void QEmacsWidget::launchCommand() {
    if (!this->ui.isEmpty()) {
      this->displayInformativeMessage(
          QObject::tr("command attemted "
                      "to use minibuffer "
                      "while in minibuffer"));
    }
    auto* l = new QEmacsWidget::Command(*this);
    l->setInputHistorySettingAddress("command/history");
    this->setUserInput(l);
  }  // end of QEmacsWidget::launchCommand

  void QEmacsWidget::launchCommand(const QString& c) {
    auto& f = QEmacsCommandFactory::getQEmacsCommandFactory();
    auto* qc = f.getQEmacsCommand(c, *this);
    if (qc == nullptr) {
      this->displayInformativeMessage(
          QObject::tr("unknown command '%1'").arg(c));
      return;
    }
    qc->execute();
    delete qc;
  }  // end of QEmacsWidget::launchCommand

  QEmacsBuffer& QEmacsWidget::getCurrentBuffer() {
    return *(
        qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget()));
  }  // end of QEmacsWidget::getCurrentBuffer

  void QEmacsWidget::closeBuffer(QEmacsBuffer& buffer, const bool b) {
    if (b) {
#pragma message("unimplemented")
      // if(!buffer.isOkToClose()){
      // 	this->setUserInput(new
      // QEmacsWidget::SaveBufferAndKill(buffer,*this));
      // } else {
      this->removeBuffer(&buffer);
      // }
    } else {
      this->removeBuffer(&buffer);
    }
  }  // end of QEmacsWidget::closeCurrentBuffer

  void QEmacsWidget::closeCurrentBuffer(const bool b) {
    auto* buffer =
        qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    if (buffer == nullptr) {
      return;
    }
    if (b) {
      if (!buffer->isOkToClose()) {
        this->setUserInput(
            new QEmacsWidget::SaveCurrentBufferAndKill(*this));
      } else {
        this->removeBuffer(buffer);
      }
    } else {
      this->removeBuffer(buffer);
    }
  }  // end of QEmacsWidget::closeCurrentBuffer

  void QEmacsWidget::saveCurrentBuffer() {
    auto* b =
        qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    if (b == nullptr) {
      return;
    }
    auto& t = b->getMainFrame();
    if (t.document()->isModified()) {
      t.save();
    }
  }  // end of QEmacsWidget::closeCurrentBuffer

  void QEmacsWidget::attachSecondaryTask(const QEmacsBuffer* b, QWidget* w) {
    if (w == nullptr) {
      return;
    }
    auto& bv = this->secondaryTasks[w];
    if(std::find(bv.begin(),bv.end(),w)==bv.end()){
      bv.push_back(b);
    }
  }  // end of attachSecondaryTask

  void QEmacsWidget::detachSecondaryTask(const QEmacsBuffer* b, QWidget* w) {
    if (w == nullptr) {
      return;
    }
    const auto p = this->secondaryTasks.find(w);
    if(p==this->secondaryTasks.end()){
      return;
    }
    const auto pbe = p->second.end();;
    const auto pb  = std::find(p->second.begin(), pbe, b);
    if (pb != pbe) {
      p->second.erase(pb);
    }
    if (p->second.empty()) {
      p->first->deleteLater();
      this->secondaryTasks.erase(p);
    }
  }  // end of detachSecondaryTask

  QEmacsWidget::~QEmacsWidget() {
    this->removeUserInputs();
    delete this->buffers;
    delete this->minibuffer;
  }

}  // end of namespace qemacs
