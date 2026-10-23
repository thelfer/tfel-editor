/*!
 * \file   EditorWidget.cxx
 * \brief
 * \author Thomas Helfer
 * \date   26/06/2012
 */

#include <iostream>
#include <algorithm>

#include <QtCore/QFile>
#include <QtCore/QMap>
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

  /*!
   * \brief return the buffer designated by the given name, nullptr if no
   * buffer matches or if the designation is ambiguous.
   *
   * \note The name of a buffer is made of the base name of the visited file
   * and, when several buffers visit files with the same base name, of a
   * suffix holding the first parent directory which makes this name unique
   * (see EditorWidget::updateBufferNameSuffixes). A buffer is thus designated
   * either by its complete name or, when this is not ambiguous, by the base
   * name of the visited file.
   *
   * \param[in] w: widget holding the buffers
   * \param[in] n: name of the buffer
   * \param[in] e: buffer which shall not be returned, even if its name
   * matches. It is used to disambiguate a name shared by several buffers, so
   * that switching to such a name still designates another buffer.
   */
  static Buffer* findBuffer(QStackedWidget& w,
                            const QString& n,
                            Buffer* const e) {
    Buffer* exact = nullptr;
    Buffer* raw = nullptr;
    bool ambiguous = false;
    for (int i = 0; i != w.count(); ++i) {
      auto* b = qobject_cast<Buffer*>(w.widget(i));
      if ((b == nullptr) || (b == e)) {
        continue;
      }
      if (b->getBufferName() == n) {
        if (exact == nullptr) {
          exact = b;
        }
      } else if (b->getBufferRawName() == n) {
        if (raw == nullptr) {
          raw = b;
        } else {
          ambiguous = true;
        }
      }
    }
    if (exact != nullptr) {
      return exact;
    }
    return ambiguous ? nullptr : raw;
  }  // end of findBuffer

  static void setQLineEditFont(LineEdit* l) {
    QSettings s;
    if (s.contains("font/textedit")) {
      auto f = s.value("font/textedit").value<QFont>();
      f.setStyleStrategy(QFont::PreferAntialias);
      l->setFont(f);
    }
    //    auto f = l->font();
    //    f.setPointSize(8);
    l->setContentsMargins(0, 0, 0, 0);
  }  // end of setQLineEditFont

  static void setCommandLineFont(CommandLine* l) {
    QSettings s;
    if (s.contains("font/textedit")) {
      auto f = s.value("font/textedit").value<QFont>();
      f.setStyleStrategy(QFont::PreferAntialias);
      l->setFont(f);
    }
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
    // the suffix of the name of a buffer depends on the file it visits and on
    // the files visited by all the other buffers: all the suffixes are
    // re-chosen once this buffer is held by the widget handling the buffers
    this->updateBufferNameSuffixes();
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
    // the suffix of the name of the buffers which do not visit any file is a
    // counter which depends on how many such buffers exist
    this->updateBufferNameSuffixes();
    this->setCurrentBuffer(b);
    emit bufferAdded();
  }  // end of ::createEmptyBuffer()

  Buffer* EditorWidget::createNewBuffer(const QString& f) {
    QSettings settings;
    Buffer* b = nullptr;
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
    if (b == nullptr) {
      return;
    }
    if (this->buffers->indexOf(b) == -1) {
      return;
    }
    const auto n = b->getBufferName();
    this->bHistory.removeAll(n);
    // removing the buffer in the secondary task manager, cleaning
    // lonely task
    SecondaryTaskManager::removeBuffer(b);
    this->buffers->removeWidget(b);
    // the suffix of the name of the other buffers may be made useless by the
    // removal of this buffer: it shall be re-chosen before the buffers history
    // is used again as it holds the names of the buffers
    this->updateBufferNameSuffixes();
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
    if (this->buffers->indexOf(b) == -1) {
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
    const auto found =
        std::find_if(this->ui.begin(), this->ui.end(),
                     [l](const auto& e) { return e.first == l; });
    if (found != this->ui.end()) {
      return;
    }
    if (!this->ui.empty()) {
      if (this->ui.back().second) {
        this->displayInformativeMessage(
            QObject::tr("command attemted "
                        "to use minibuffer "
                        "while in minibuffer"));
        return;
      }
    }
    // record the blocking state now, while the command line is still an
    // object of its most derived type: once it is being destroyed, a virtual
    // call to isBlocking() would dispatch to CommandLine::isBlocking() and
    // report false for an input that really was blocking
    const auto blocking = l->isBlocking();
    this->ui.push_back({l, blocking});
    setCommandLineFont(this->ui.back().first);
    this->minibuffer->addWidget(this->ui.back().first);
    this->minibuffer->setCurrentWidget(this->ui.back().first);
    this->ui.back().first->setFocus();
    if (blocking) {
      this->buffers->setEnabled(false);
    }
    QObject::connect(this->ui.back().first, &CommandLine::finished, this,
                     static_cast<void (EditorWidget::*)(CommandLine*)>(
                         &EditorWidget::removeUserInput));
    QObject::connect(this->ui.back().first, &CommandLine::destroying, this,
                     static_cast<void (EditorWidget::*)(CommandLine*)>(
                         &EditorWidget::removeUserInput));
  }

  void EditorWidget::removeUserInput(CommandLine* p) {
    if (this->ui.empty()) {
      return;
    }
    const auto it = std::find_if(this->ui.begin(), this->ui.end(),
                                 [p](const auto& e) { return e.first == p; });
    if (it == this->ui.end()) {
      return;
    }
    // the blocking state is read from the record, not from p: p may be on its
    // way out, in which case the virtual call would report false and the
    // buffers would stay disabled forever
    if (it->second) {
      Q_ASSERT(it->first == this->ui.back().first);
      this->buffers->setEnabled(true);
    }
    this->minibuffer->removeWidget(p);
    this->ui.erase(it);
    if (this->ui.empty()) {
      this->minibuffer->setCurrentWidget(this->eui);
    } else {
      this->minibuffer->setCurrentWidget(this->ui.back().first);
    }
    if (auto* const cw = this->buffers->currentWidget()) {
      cw->setFocus();
    }
    p->deleteLater();
  }  // end of EditorWidget::removeUserInput

  void EditorWidget::removeUserInput() {
    if (this->ui.empty()) {
      return;
    }
    this->removeUserInput(this->ui.back().first);
  }  // end of EditorWidget::removeUserInput

  void EditorWidget::removeUserInputs() {
    while (!this->ui.empty()) {
      this->removeUserInput(this->ui.back().first);
    }
  }  // end of EditorWidget::removeUserInputs

  bool EditorWidget::hasUserInput() const {
    return !this->ui.empty();
  }  // end of EditorWidget::hasUserInput

  void EditorWidget::focusUserInput() {
    if (!this->ui.empty()) {
      this->ui.back().first->setFocus();
    }
  }  // end of EditorWidget::focusUserInput

  void EditorWidget::resetUserInput() {
    if (!this->ui.empty()) {
      this->minibuffer->setCurrentWidget(this->ui.back().first);
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
    // the current buffer is discarded first: when several buffers visit
    // files with the same base name, the name of the current buffer can
    // designate another buffer too, and switching shall not be a no-op
    auto* const cb = qobject_cast<Buffer*>(this->buffers->currentWidget());
    auto* b = findBuffer(*this->buffers, n, cb);
    if (b == nullptr) {
      b = findBuffer(*this->buffers, n, nullptr);
    }
    if (b != nullptr) {
      this->setCurrentBuffer(b);
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

  /*!
   * \brief a buffer and the information used to choose the suffix of its name
   */
  struct BufferSuffixInfo {
    //! buffer
    Buffer* b;
    //! file visited by the buffer, empty if the buffer visits no file
    QString path;
    /*!
     * \brief parent directories of the visited file, from the visited file to
     * the root directory, so that the first element is the directory holding
     * the visited file
     */
    QStringList dirs;
  };  // end of BufferSuffixInfo

  /*!
   * \brief return the parent directories of the path visited by a buffer
   * \param[in] f: path of the file, or of the directory, visited by a buffer
   * \return the parent directories of the given path, from this path to the
   * root directory, an empty list if the given path is empty
   */
  static QStringList getParentDirectories(const QString& f) {
    // the last component is the file (or the directory) itself, which is what
    // the raw name of the buffer holds: it is discarded
    auto d = f.split('/', Qt::SkipEmptyParts);
    if (!d.empty()) {
      d.removeLast();
    }
    // the directories are reversed as the suffix of the name of a buffer is
    // built from the file it visits, the first parent directory which makes a
    // difference being looked for first
    std::reverse(d.begin(), d.end());
    return d;
  }  // end of getParentDirectories

  /*!
   * \brief choose the suffix of the name of buffers visiting files with the
   * same name
   * \param[in,out] v: buffers visiting files with the same name
   * \param[in] depth: index in the parent directories of the visited files at
   * which the comparison is performed
   *
   * The given buffers are split according to the parent directory of the file
   * they visit at the given depth. A buffer which is alone in its split gets
   * this parent directory as suffix: this is the first parent directory which
   * makes its name differ from the names of the other buffers. The buffers
   * sharing this parent directory are handled again at the next depth as this
   * directory can not tell them apart. When the parent directories are
   * exhausted, which happens when a buffer visits a file held by a parent
   * directory of the files visited by the other buffers, the complete path is
   * used as suffix.
   */
  static void assignSuffixes(QList<BufferSuffixInfo>& v, const int depth) {
    QMap<QString, QList<BufferSuffixInfo>> s;
    for (const auto& i : v) {
      const auto k = (depth < i.dirs.size()) ? i.dirs[depth] : QString();
      s[k].append(i);
    }
    if (s.size() == 1) {
      if (s.firstKey().isEmpty()) {
        // the parent directories of all the given buffers are exhausted: the
        // visited path is the only information left to tell them apart. Two
        // buffers visiting the same file can not happen (see
        // EditorWidget::getBufferVisitingFile), so such buffers can only be
        // the result of a save as operation
        for (auto& i : v) {
          i.b->setBufferNameSuffix(i.path);
        }
        return;
      }
      // all the given buffers visit files held by the same directory
      assignSuffixes(v, depth + 1);
      return;
    }
    for (auto it = s.begin(); it != s.end(); ++it) {
      auto& g = it.value();
      if (g.size() == 1) {
        // this parent directory makes this buffer differ from the others
        g[0].b->setBufferNameSuffix(it.key().isEmpty() ? g[0].path : it.key());
      } else {
        assignSuffixes(g, depth + 1);
      }
    }
  }  // end of assignSuffixes

  void EditorWidget::updateBufferNameSuffixes() {
    // the suffix of the name of a buffer depends on the files visited by all
    // the buffers, so all of them are handled at once. The buffers are
    // gathered first as setting the suffix of a buffer emits the
    // bufferNameChanged signal, which may create or remove buffers
    QList<BufferSuffixInfo> v;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      if (b == nullptr) {
        continue;
      }
      BufferSuffixInfo bi;
      bi.b = b;
      bi.path = b->getMainFrame().getCompleteFileName();
      bi.dirs = getParentDirectories(bi.path);
      v.append(bi);
    }
    // only the buffers visiting files with the same name need a suffix: the
    // grouping is performed on the raw name, that is on what is displayed
    QMap<QString, QList<BufferSuffixInfo>> g;
    for (const auto& b : v) {
      g[b.b->getBufferRawName()].append(b);
    }
    for (auto it = g.begin(); it != g.end(); ++it) {
      auto& s = it.value();
      if (s.size() == 1) {
        s[0].b->setBufferNameSuffix(QString());
        continue;
      }
      if (it.key().isEmpty()) {
        // buffers which do not visit any file: the empty raw name they share
        // holds no directory information, so a counter tells them apart, the
        // first buffer of the group keeping no suffix
        for (int j = 0; j != s.size(); ++j) {
          s[j].b->setBufferNameSuffix(j == 0 ? QString() : QString::number(j));
        }
        continue;
      }
      assignSuffixes(s, 0);
    }
  }  // end of EditorWidget::updateBufferNameSuffixes

  void EditorWidget::changeBuffer() {
    auto* cb = qobject_cast<Buffer*>(this->buffers->currentWidget());
    if (cb == nullptr) {
      return;
    }
    QStringList bnames;
    //! the only other buffer, nullptr if there is none or several ones
    Buffer* ob = nullptr;
    for (int i = 0; i != this->buffers->count(); ++i) {
      auto* b = qobject_cast<Buffer*>(this->buffers->widget(i));
      // the current buffer is discarded by identity: two buffers visiting
      // files with the same base name can share the same name and both of
      // them shall be selectable
      if ((b == nullptr) || (b == cb)) {
        continue;
      }
      bnames.append(b->getBufferName());
      ob = b;
    }
    if (bnames.empty()) {
      this->displayInformativeMessage(QObject::tr("no other buffer"));
      return;
    }
    if (bnames.size() == 1) {
      // switching by the buffer name is not enough here: another buffer may
      // share the same name, so the buffer itself is used
      this->setCurrentBuffer(ob);
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
    if (this->buffers->indexOf(b) == -1) {
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
    // guard the pointer that is dereferenced, not the count: the count is
    // nonzero whenever a non-Buffer widget happens to be current, and the
    // cast is what can actually yield null here. Every sibling method in this
    // file (changeBuffer, closeCurrentBuffer, saveCurrentBuffer) checks the
    // cast.
    auto* b = qobject_cast<Buffer*>(this->buffers->currentWidget());
    if (b == nullptr) {
      return std::vector<QMenu*>();
    }
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
    auto* b = qobject_cast<Buffer*>(this->buffers->currentWidget());
    if (b == nullptr) {
      // The constructor creates no buffer, so the stacked widget is genuinely
      // empty between construction and MainWindow's first createEmptyBuffer().
      // The 39 call sites that take this by reference rely on that window
      // being unreachable; materialise the buffer instead, which is what
      // removeBuffer() already does when the last buffer is killed.
      this->createEmptyBuffer();
      b = qobject_cast<Buffer*>(this->buffers->currentWidget());
    }
    return *b;
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
