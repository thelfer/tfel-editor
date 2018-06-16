/*!
 * \file   QEmacsWidget.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/06/2012
 */

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

#include "QEmacs/QEmacsCommand.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/QEmacsCommandLine.hxx"
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

  static QString getRealPath(const QString& f) {
    QFileInfo fi(f);
    if (fi.isSymLink()) {
      return fi.symLinkTarget();
    }
    return fi.absoluteFilePath();
  }  // end of getRealPath

  static void setQLineEditFont(QEmacsLineEdit* l) {
    auto f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 0);
  }  // end of setQLineEditFont

  static void setQEmacsCommandLineFont(QEmacsCommandLine* l) {
    auto f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 0);
  }  // end of setQEmacsCommandLineFont

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

  struct QEmacsWidget::ChangeBuffer : public QEmacsCommandLine {
    ChangeBuffer(QEmacsWidget& p,
                 const QStringList& bn,
                 const QString& d)
        : QEmacsCommandLine(
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

  struct QEmacsWidget::Command : public QEmacsCommandLine {
    Command(QEmacsWidget& p)
        : QEmacsCommandLine(QObject::tr("qemacs command:"), p) {
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
      const auto i = this->input->text();
      this->qemacs.removeUserInput(this);
      this->qemacs.launchCommand(i);
    }

  };  // end of struct QEmacsTextEdit::Command

  QEmacsWidget::QEmacsWidget(QWidget* const p)
      : QWidget(p),
        buffers(new QStackedWidget),
        minibuffer(new QStackedWidget),
        um(new QEmacsLineEdit(*this)),
        ui(0),
        eui(new QEmacsLineEdit(*this)),
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

  QStringList& QEmacsWidget::getRecentFiles() {
    static QStringList files;
    return files;
  }  // end of QEmacsWidget::getRecentFiles

  void QEmacsWidget::addToRecentFiles(const QString& f) {
    if (f.isEmpty()) {
      return;
    }
    auto& files = QEmacsWidget::getRecentFiles();
    files.append(f);
    if (files.size() > 100) {
      files.pop_front();
    }
  }  // end of QEmacsWidget::addToRecentFiles

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
    const auto af = getRealPath(f);
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      const auto bf = b->getMainFrame().getCompleteFileName();
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
    auto* b = this->createNewBuffer();
    this->buffers->addWidget(b);
    this->setCurrentBuffer(b);
    emit bufferAdded();
  }  // end of QEmacs::createEmptyBuffer()

  QEmacsBuffer* QEmacsWidget::createNewBuffer(const QString& f) {
    QSettings settings;
    QEmacsBuffer* b;
    if (f.isEmpty()) {
      b = new QEmacsBuffer(this->nid, *this);
      b->setSecondaryTasksOrientation(
          settings.value("secondary tasks orientation")
              .value<Qt::Orientation>());
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
      b->setSecondaryTasksOrientation(
          settings.value("secondary tasks orientation")
              .value<Qt::Orientation>());
      ++(this->nid);
      this->emitNewTreatedFile(b->getMainFrame().getCompleteFileName());
    }
    QObject::connect(b, &QEmacsBuffer::bufferNameChanged, this,
                     &QEmacsWidget::updateBufferName);
    QObject::connect(b, &QEmacsBuffer::newTreatedFile, this,
                     &QEmacsWidget::emitNewTreatedFile);
    QObject::connect(b, &QEmacsBuffer::mainFrameMajorModeChanged, this,
                     [this, b] {
                       if (&(this->getCurrentBuffer()) == b) {
                         emit currentBufferMajorModeChanged();
                       }
                     });
    QObject::connect(b, &QEmacsBuffer::updatedMenu, this, [this, b] {
      if (&(this->getCurrentBuffer()) == b) {
        emit updatedMenu();
      }
    });
    return b;
  }  // end of QEmacsWidget::createNewBuffer

  void QEmacsWidget::removeBuffer(QEmacsBuffer* const b) {
    const auto n = b->getBufferName();
    this->bHistory.removeAll(n);
    // removing the buffer in the secondary task manager, cleaning
    // lonely task
    SecondaryTaskManager::removeBuffer(b);
    this->buffers->removeWidget(b);
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

  void QEmacsWidget::nextBuffer() {
    if (this->buffers->count() == 1) {
      return;
    }
    const auto i = this->buffers->currentIndex();
    const auto n = i + 1 >= this->buffers->count() ? 0 : i + 1;
    this->buffers->setCurrentIndex(n);
  }  // end of QEmacsWidget::nextBuffer

  void QEmacsWidget::previousBuffer() {
    if (this->buffers->count() == 1) {
      return;
    }
    const auto i = this->buffers->currentIndex();
    const auto n = i == 0 ? this->buffers->count() - 1 : i - 1;
    this->buffers->setCurrentIndex(n);
  }  // end of QEmacsWidget::previousBuffer

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
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    QTimer::singleShot(1000, this, SLOT(resetUserInput()));
#else
    QTimer::singleShot(1000, this, &QEmacsWidget::resetUserInput);
#endif

  }  // end of QEmacsWidget::displayInformativeMessage

  void QEmacsWidget::setUserInput(QEmacsCommandLine* const l) {
    if (l == nullptr) {
      return;
    }
    if (std::find(this->ui.begin(), this->ui.end(), l) !=
        this->ui.end()) {
      return;
    }
    if (!this->ui.empty()) {
      if (this->ui.back()->isBlocking()) {
        this->displayInformativeMessage(
            QObject::tr("command attemted "
                        "to use minibuffer "
                        "while in minibuffer"));
        return;
      }
    }
    this->ui.push_back(l);
    setQEmacsCommandLineFont(this->ui.back());
    this->minibuffer->addWidget(this->ui.back());
    this->minibuffer->setCurrentWidget(this->ui.back());
    this->ui.back()->setFocus();
    if (this->ui.back()->isBlocking()) {
      this->buffers->setEnabled(false);
    }
    QObject::connect(
        this->ui.back(), &QEmacsCommandLine::finished, this,
        static_cast<void (QEmacsWidget::*)(QEmacsCommandLine*)>(
            &QEmacsWidget::removeUserInput));
    QObject::connect(
        this->ui.back(), &QEmacsCommandLine::destroyed, this,
        static_cast<void (QEmacsWidget::*)(QEmacsCommandLine*)>(
            &QEmacsWidget::removeUserInput));
  }

  void QEmacsWidget::removeUserInput(QEmacsCommandLine* p) {
    if (this->ui.empty()) {
      return;
    }
    if (std::find(this->ui.begin(), this->ui.end(), p) ==
        this->ui.end()) {
      return;
    }
    if (p->isBlocking()) {
      Q_ASSERT(p == this->ui.back());
      this->buffers->setEnabled(true);
    }
    this->minibuffer->removeWidget(p);
    this->ui.erase(std::find(this->ui.begin(), this->ui.end(), p));
    if (this->ui.empty()) {
      this->minibuffer->setCurrentWidget(this->eui);
    } else {
      this->minibuffer->setCurrentWidget(this->ui.back());
    }
    this->buffers->currentWidget()->setFocus();
    p->deleteLater();
  }  // end of QEmacsWidget::removeUserInput

  void QEmacsWidget::removeUserInput() {
    if (this->ui.empty()) {
      return;
    }
    this->removeUserInput(this->ui.back());
  }  // end of QEmacsWidget::removeUserInput

  void QEmacsWidget::removeUserInputs() {
    while (!this->ui.empty()) {
      this->removeUserInput(this->ui.back());
    }
  }  // end of QEmacsWidget::removeUserInputs

  bool QEmacsWidget::hasUserInput() const {
    return !this->ui.empty();
  }  // end of QEmacsWidget::hasUserInput

  void QEmacsWidget::focusUserInput() {
    if (!this->ui.empty()) {
      this->ui.back()->setFocus();
    }
  }  // end of QEmacsWidget::focusUserInput

  void QEmacsWidget::resetUserInput() {
    if (!this->ui.empty()) {
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

  std::vector<QIcon> QEmacsWidget::getBuffersIcons() const {
    std::vector<QIcon> icons;
    for (int i = 0; i != this->buffers->count(); ++i) {
      QEmacsBuffer const* b =
          qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        icons.push_back(b->getIcon());
      }
    }
    return icons;
  }

  std::vector<int> QEmacsWidget::getBuffersIds() const {
    std::vector<int> ids;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto const* b =
          qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        ids.push_back(b->getId());
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

  std::vector<QMenu*> QEmacsWidget::getCurrentBufferSpecificMenus() {
    if (this->buffers->count() == 0) {
      return std::vector<QMenu*>();
    }
    auto* b =
        qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    return b->getSpecificMenus();
  }  // end of QEmacsWidget::getCurrentBufferMenu()

  void QEmacsWidget::emitNewTreatedFile(const QString& f) {
    emit newTreatedFile(f);
  }  // end of QEmacsWidget::emitNewTreatedFile

  void QEmacsWidget::launchCommand() {
    if (!this->ui.empty()) {
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
    qc->deleteLater();
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

  void QEmacsWidget::print() {
    auto& t = this->getCurrentBuffer().getMainFrame();
    t.print();
  }  // end of QEmacsWidget::print

  void QEmacsWidget::setSecondaryTasksOrientation(
      const Qt::Orientation o) {
    QSettings settings;
    settings.setValue("secondary tasks orientation", o);
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* const b =
          qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      b->setSecondaryTasksOrientation(o);
    }
  }  // end of QEmacsWidget::setSecondaryTasksOrientation

  QEmacsWidget::~QEmacsWidget() {
    this->removeUserInputs();
    this->buffers->deleteLater();
    this->minibuffer->deleteLater();
  }

}  // end of namespace qemacs
