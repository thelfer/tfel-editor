/*!
 * \file   EditorWidget.cxx
 * \brief
 * \author Thomas Helfer
 * \date   26/06/2012
 */

#include <iostream>

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

#include "TFEL/GUI/VerboseLevel.hxx"
#include "TFEL/GUI/Command.hxx"
#include "TFEL/GUI/CommandFactory.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"

struct ModeRessourceLoader {
  ModeRessourceLoader() : l(getLoader()) {}

 private:
  struct Loader {
    Loader() { Q_INIT_RESOURCE(Resources); }
  };
  static Loader& getLoader() {
    static Loader l;
    return l;
  }
  Loader l;
};

namespace tfel::gui {

  /*!
   * \brief a small function to convert a Qt::Orientation to an
   * integer.
   *
   * \note This is required on Debian Jessie as Qt::Orientation could
   * not be stored in a QVariant.
   */
  static int convert(const Qt::Orientation o) {
    return o == Qt::Horizontal ? 0 : 1;
  }  // end of convert

  /*!
   * \brief a small function to convert an integer to a
   * Qt::Orientation
   *
   * \note This is required on Debian Jessie as Qt::Orientation could
   * not be stored in a QVariant.
   */
  static Qt::Orientation convert(const int o) {
    return o == 0 ? Qt::Horizontal : Qt::Vertical;
  }  // end of convert

  static QString getRealPath(const QString& f) {
    QFileInfo fi(f);
    if (fi.isSymLink()) {
      return fi.symLinkTarget();
    }
    return fi.absoluteFilePath();
  }  // end of getRealPath

  static void setQLineEditFont(LineEdit* l) {
    auto f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 0);
  }  // end of setQLineEditFont

  static void setCommandLineFont(CommandLine* l) {
    auto f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 0);
  }  // end of setCommandLineFont

  struct EditorWidget::OpenFile : public FilePathUserInput {
    OpenFile(EditorWidget& p, const QString& path)
        : FilePathUserInput(QObject::tr("open file :"), p) {
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
      this->editor.openFile(this->input->text());
    }

  };  // end of struct TextEdit::OpenFile

  struct EditorWidget::ChangeBuffer : public CommandLine {
    /*!
     * \param[in] p: editor
     * \param[in] bn: list of editor widget
     * \param[in] d: default buffer
     */
    ChangeBuffer(EditorWidget& p, const QStringList& bn, const QString& d)
        : CommandLine(
              QObject::tr("switch to buffer") +
                  (!d.isEmpty() ? QObject::tr("(default '%1')").arg(d) : "") +
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
        this->editor.changeBuffer(this->defaultBuffer);
      } else {
        this->editor.changeBuffer(t);
      }
    }

    QString defaultBuffer;

  };  // end of struct TextEdit::ChangeBuffer

  struct EditorWidget::Command : public CommandLine {
    Command(EditorWidget& p) : CommandLine(QObject::tr("editor command:"), p) {
      auto& f = CommandFactory::getCommandFactory();
      auto* c = new QCompleter(f.getAvailableCommandsNames(), &p);
      c->setWidget(this->input);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(c, false);
    }

    ~Command() override = default;

   protected:
    void treatUserInput() override {
      // the input must be kept before calling the removeUserInput
      // method (this causes the editorwdget to delete this).
      const auto i = this->input->text();
      this->editor.removeUserInput(this);
      this->editor.launchCommand(i);
    }

  };  // end of struct TextEdit::Command

  EditorWidget::EditorWidget(QWidget* const p)
      : QWidget(p),
        buffers(new QStackedWidget),
        minibuffer(new QStackedWidget),
        um(new LineEdit(*this)),
        ui(0),
        eui(new LineEdit(*this)),
        nid(0) {
    ModeRessourceLoader loader;
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
    //    vl->setMargin(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(this->buffers);
    vl->addWidget(this->minibuffer);
    vl->setSpacing(0);
    this->setLayout(vl);
  }  // end of Wigdet::Wigdet

  QStringList& EditorWidget::getRecentFiles() {
    static QStringList files;
    return files;
  }  // end of EditorWidget::getRecentFiles

  void EditorWidget::addToRecentFiles(const QString& f) {
    if (f.isEmpty()) {
      return;
    }
    auto& files = EditorWidget::getRecentFiles();
    files.append(f);
    if (files.size() > 100) {
      files.pop_front();
    }
  }  // end of EditorWidget::addToRecentFiles

  void EditorWidget::changeMainFramesFont(const QFont& f) {
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      auto& t = b->getMainFrame();
      t.setFont(f);
    }
  }  // end of EditorWidget::changeMainFrameFont

  Buffer* EditorWidget::getBufferVisitingFile(const QString& f) {
    const auto af = getRealPath(f);
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      const auto bf = b->getMainFrame().getCompleteFileName();
      if (bf == af) {
        return b;
      }
    }
    return nullptr;
  }  // end of EditorWidget::getBufferVisitingFile

  void EditorWidget::openFile(const QString& f) {
    if (f.isEmpty()) {
      return;
    }
    auto* b = this->createNewBuffer(f);
    if (this->buffers->indexOf(b) == -1) {
      this->buffers->addWidget(b);
      emit bufferAdded();
    }
    this->setCurrentBuffer(b);
  }  // end of ::openFile(const QString&)

  void EditorWidget::openFile() {
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
    auto* l = new EditorWidget::OpenFile(*this, path);
    l->setInputHistorySettingAddress("recent files");
    this->setUserInput(l);
  }  // end of ::openFile()

  void EditorWidget::createEmptyBuffer() {
    auto* b = this->createNewBuffer();
    this->buffers->addWidget(b);
    this->setCurrentBuffer(b);
    emit bufferAdded();
  }  // end of ::createEmptyBuffer()

  Buffer* EditorWidget::createNewBuffer(const QString& f) {
    QSettings settings;
    Buffer* b;
    if (f.isEmpty()) {
      b = new Buffer(this->nid, *this);
      b->setSecondaryTasksOrientation(
          convert(settings.value("secondary tasks orientation").value<int>()));
      ++(this->nid);
    } else {
      b = this->getBufferVisitingFile(f);
      if (b != nullptr) {
        this->displayInformativeMessage(
            QObject::tr("A buffer is already visiting file '%1'").arg(f));
        return b;
      }
      b = new Buffer(f, this->nid, *this);
      b->setSecondaryTasksOrientation(
          convert(settings.value("secondary tasks orientation").value<int>()));
      ++(this->nid);
      this->emitNewTreatedFile(b->getMainFrame().getCompleteFileName());
    }
    QObject::connect(b, &Buffer::bufferNameChanged, this,
                     &EditorWidget::updateBufferName);
    QObject::connect(b, &Buffer::newTreatedFile, this,
                     &EditorWidget::emitNewTreatedFile);
    QObject::connect(b, &Buffer::mainFrameMajorModeChanged, this, [this, b] {
      if (&(this->getCurrentBuffer()) == b) {
        emit currentBufferMajorModeChanged();
      }
    });
    QObject::connect(b, &Buffer::updatedMenu, this, [this, b] {
      if (&(this->getCurrentBuffer()) == b) {
        emit updatedMenu();
      }
    });
    return b;
  }  // end of EditorWidget::createNewBuffer

  void EditorWidget::removeBuffer(Buffer* const b) {
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
        auto* nb = qobject_cast<Buffer*>(cw);
        if (nb != nullptr) {
          this->setCurrentBuffer(nb);
        }
      }
    }
    b->deleteLater();
    emit bufferRemoved();
  }  // end of EditorWidget::removeBuffer

  void EditorWidget::updateBufferName(Buffer* b,
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

  void EditorWidget::nextBuffer() {
    if (this->buffers->count() == 1) {
      return;
    }
    const auto i = this->buffers->currentIndex();
    const auto n = i + 1 >= this->buffers->count() ? 0 : i + 1;
    this->buffers->setCurrentIndex(n);
  }  // end of EditorWidget::nextBuffer

  void EditorWidget::previousBuffer() {
    if (this->buffers->count() == 1) {
      return;
    }
    const auto i = this->buffers->currentIndex();
    const auto n = i == 0 ? this->buffers->count() - 1 : i - 1;
    this->buffers->setCurrentIndex(n);
  }  // end of EditorWidget::previousBuffer

  void EditorWidget::saveAllBuffers() {
    int i;
    for (i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      auto& t = b->getMainFrame();
      if (t.document()->isModified()) {
        t.save();
      }
    }
  }  // end of EditorWidget::saveAllBuffers()

  bool EditorWidget::isOkToClose() {
    const auto unsaved = [this] {
      QStringList bl;
      for (int i = 0; i != this->buffers->count(); ++i) {
        auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
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
          this, QObject::tr(""), msg,
          QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel) {
        return false;
      } else if (ret == QMessageBox::SaveAll) {
        this->saveAllBuffers();
      }
    }
    return true;
  }  // end of EditorWidget::isOkToClose

  void EditorWidget::closeEvent(QCloseEvent* e) {
    if (this->isOkToClose()) {
      // for(int i=0;i!=this->buffers->count();++i){
      // 	Buffer *b =
      // qobject_cast<Buffer*>(this->buffers->widget(i));
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
  }  // end of EditorWidget::close

  void EditorWidget::displayInformativeMessage(const QString& m) {
    this->um->setText(m);
    this->minibuffer->setCurrentWidget(this->um);
    const auto d = getVerboseMode() >= VERBOSE_DEBUG;
    const auto t = d ? 10000 : 2000;
    if (d) {
      std::clog << m.toStdString() << '\n';
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    QTimer::singleShot(t, this, SLOT(resetUserInput()));
#else
    QTimer::singleShot(t, this, &EditorWidget::resetUserInput);
#endif
  }  // end of EditorWidget::displayInformativeMessage

  void EditorWidget::setUserInput(CommandLine* const l) {
    if (l == nullptr) {
      return;
    }
    if (std::find(this->ui.begin(), this->ui.end(), l) != this->ui.end()) {
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
    setCommandLineFont(this->ui.back());
    this->minibuffer->addWidget(this->ui.back());
    this->minibuffer->setCurrentWidget(this->ui.back());
    this->ui.back()->setFocus();
    if (this->ui.back()->isBlocking()) {
      this->buffers->setEnabled(false);
    }
    QObject::connect(this->ui.back(), &CommandLine::finished, this,
                     static_cast<void (EditorWidget::*)(CommandLine*)>(
                         &EditorWidget::removeUserInput));
    QObject::connect(this->ui.back(), &CommandLine::destroyed, this,
                     static_cast<void (EditorWidget::*)(CommandLine*)>(
                         &EditorWidget::removeUserInput));
  }

  void EditorWidget::removeUserInput(CommandLine* p) {
    if (this->ui.empty()) {
      return;
    }
    if (std::find(this->ui.begin(), this->ui.end(), p) == this->ui.end()) {
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
  }  // end of EditorWidget::removeUserInput

  void EditorWidget::removeUserInput() {
    if (this->ui.empty()) {
      return;
    }
    this->removeUserInput(this->ui.back());
  }  // end of EditorWidget::removeUserInput

  void EditorWidget::removeUserInputs() {
    while (!this->ui.empty()) {
      this->removeUserInput(this->ui.back());
    }
  }  // end of EditorWidget::removeUserInputs

  bool EditorWidget::hasUserInput() const {
    return !this->ui.empty();
  }  // end of EditorWidget::hasUserInput

  void EditorWidget::focusUserInput() {
    if (!this->ui.empty()) {
      this->ui.back()->setFocus();
    }
  }  // end of EditorWidget::focusUserInput

  void EditorWidget::resetUserInput() {
    if (!this->ui.empty()) {
      this->minibuffer->setCurrentWidget(this->ui.back());
    } else {
      this->minibuffer->setCurrentWidget(this->eui);
    }
    this->um->clear();
  }  // end of EditorWidget::resetPreviousUserInput

  const QStringList& EditorWidget::getKillRing() const {
    return this->killRing;
  }  // end of EditorWidget::getKillRing

  void EditorWidget::addToKillRing(const QString& t) {
    if (t.isEmpty()) {
      return;
    }
    if (this->killRing.size() == 100) {
      this->killRing.pop_front();
    }
    this->killRing << t;
  }  // end of EditorWidget::addToKillRing

  QStringList EditorWidget::getBuffersNames() const {
    QStringList n;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
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
  }  // end of EditorWidget::getBuffersNames

  std::vector<QIcon> EditorWidget::getBuffersIcons() const {
    std::vector<QIcon> icons;
    for (int i = 0; i != this->buffers->count(); ++i) {
      Buffer const* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        icons.push_back(b->getIcon());
      }
    }
    return icons;
  }

  std::vector<int> EditorWidget::getBuffersIds() const {
    std::vector<int> ids;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto const* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        ids.push_back(b->getId());
      }
    }
    return ids;
  }

  void EditorWidget::changeBuffer(const QString& n) {
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b != nullptr) {
        if (b->getBufferName() == n) {
          this->setCurrentBuffer(b);
          return;
        }
      }
    }
  }  // end of EditorWidget::changeBuffer

  void EditorWidget::changeBuffer(const int id) {
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      if (b->getId() == id) {
        this->setCurrentBuffer(b);
        return;
      }
    }
  }  // end of EditorWidget::changeBuffer

  QString EditorWidget::chooseBufferNameSuffix(Buffer* b, const QString& f) {
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
        auto* ob = qobject_cast<Buffer*>(this->buffers->widget(i));
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
  }  // end of EditorWidget::chooseBufferName

  void EditorWidget::changeBuffer() {
    auto* cb = qobject_cast<Buffer*>(this->buffers->currentWidget());
    if (cb == nullptr) {
      return;
    }
    const auto& bn = cb->getBufferName();
    QStringList bnames;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      const auto& n = b->getBufferName();
      if (bn != n) {
        bnames.append(n);
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
  }  // end of EditorWidget::changeBuffer

  void EditorWidget::setCurrentBuffer(Buffer* const b) {
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
  }  // end of EditorWidget::setCurrentBuffer

  struct EditorWidget::SaveCurrentBufferAndKill : public YesOrNoUserInput {
    SaveCurrentBufferAndKill(EditorWidget& p)
        : YesOrNoUserInput(
              QObject::tr("buffer '%1' has been modified; kill anyway ?")
                  .arg(p.getCurrentBuffer().getBufferName()),
              p),
          t(p) {}
    bool isBlocking() const override { return true; }  // end of isBlocking
   protected:
    void treatUserInput() override {
      if (this->input->text() == "y") {
        this->t.removeBuffer(&(this->buffer));
      }
    }  // end of treatUserInput
    EditorWidget& t;
  };

  std::vector<QMenu*> EditorWidget::getCurrentBufferSpecificMenus() {
    if (this->buffers->count() == 0) {
      return std::vector<QMenu*>();
    }
    auto* b = qobject_cast<Buffer*>(this->buffers->currentWidget());
    return b->getSpecificMenus();
  }  // end of EditorWidget::getCurrentBufferMenu()

  void EditorWidget::emitNewTreatedFile(const QString& f) {
    emit newTreatedFile(f);
  }  // end of EditorWidget::emitNewTreatedFile

  void EditorWidget::launchCommand() {
    if (!this->ui.empty()) {
      this->displayInformativeMessage(
          QObject::tr("command attemted "
                      "to use minibuffer "
                      "while in minibuffer"));
    }
    auto* l = new EditorWidget::Command(*this);
    l->setInputHistorySettingAddress("command/history");
    this->setUserInput(l);
  }  // end of EditorWidget::launchCommand

  void EditorWidget::launchCommand(const QString& c) {
    auto& f = CommandFactory::getCommandFactory();
    auto* qc = f.getCommand(c, *this);
    if (qc == nullptr) {
      this->displayInformativeMessage(
          QObject::tr("unknown command '%1'").arg(c));
      return;
    }
    qc->execute();
    qc->deleteLater();
  }  // end of EditorWidget::launchCommand

  Buffer& EditorWidget::getCurrentBuffer() {
    return *(qobject_cast<Buffer*>(this->buffers->currentWidget()));
  }  // end of EditorWidget::getCurrentBuffer

  void EditorWidget::closeBuffer(Buffer& buffer, const bool b) {
    if (b) {
#pragma message("unimplemented")
      // if(!buffer.isOkToClose()){
      // 	this->setUserInput(new
      // EditorWidget::SaveBufferAndKill(buffer,*this));
      // } else {
      this->removeBuffer(&buffer);
      // }
    } else {
      this->removeBuffer(&buffer);
    }
  }  // end of EditorWidget::closeCurrentBuffer

  void EditorWidget::closeCurrentBuffer(const bool b) {
    auto* buffer = qobject_cast<Buffer*>(this->buffers->currentWidget());
    if (buffer == nullptr) {
      return;
    }
    if (b) {
      if (!buffer->isOkToClose()) {
        this->setUserInput(new EditorWidget::SaveCurrentBufferAndKill(*this));
      } else {
        this->removeBuffer(buffer);
      }
    } else {
      this->removeBuffer(buffer);
    }
  }  // end of EditorWidget::closeCurrentBuffer

  void EditorWidget::saveCurrentBuffer() {
    auto* b = qobject_cast<Buffer*>(this->buffers->currentWidget());
    if (b == nullptr) {
      return;
    }
    auto& t = b->getMainFrame();
    if (t.document()->isModified()) {
      t.save();
    }
  }  // end of EditorWidget::closeCurrentBuffer

  void EditorWidget::print() {
    auto& t = this->getCurrentBuffer().getMainFrame();
    t.print();
  }  // end of EditorWidget::print

  void EditorWidget::setSecondaryTasksOrientation(const Qt::Orientation o) {
    QSettings settings;
    settings.setValue("secondary tasks orientation", convert(o));
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* const b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      b->setSecondaryTasksOrientation(o);
    }
  }  // end of EditorWidget::setSecondaryTasksOrientation

  EditorWidget::~EditorWidget() {
    this->removeUserInputs();
    this->buffers->deleteLater();
    this->minibuffer->deleteLater();
  }

}  // end of namespace tfel::gui
