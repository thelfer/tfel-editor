
/*!
 * \file  Buffer.cxx
 * \brief
 * \author Thomas Helfer
 * \date   27/06/2012
 */

#include <QtCore/QDir>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#ifdef TFEL_GUI_HAVE_WEBENGINE
#include "TFEL/GUI/QWebEngineViewWrapper.hxx"
#endif /* TFEL_GUI_HAVE_WEBENGINE */
#include "TFEL/GUI/QAbstractScrollAreaWrapper.hxx"
#include "TFEL/GUI/Buffer.hxx"

namespace tfel::gui {

  /*!
   * \brief structure in charge of displaying SecondaryTask widgets
   */
  struct Buffer::SecondaryTaskTabWidget : public QTabWidget {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    SecondaryTaskTabWidget(QWidget *const p) : QTabWidget(p) {
      setFontSizeAndContentsMargins(this->tabBar());
      this->tabBar()->setFocusPolicy(Qt::NoFocus);
      this->setTabPosition(QTabWidget::North);
    }  // end of SecondaryTaskTabWidget

    void show() { QTabWidget::show(); }  // end of show

    void focusInEvent(QFocusEvent *) override {
      QTabWidget::setFocus();
      auto *const c = this->currentWidget();
      if (c != nullptr) {
        c->setFocus();
      }
    }  // end of focusInEvent

    void removeTab(int i) {
      auto *w = QTabWidget::widget(i);
      if (w == nullptr) {
        return;
      }
      if (w->close()) {
        QTabWidget::removeTab(i);
      }
      if (this->count() == 0) {
        this->hide();
      }
      return;
    }  // end of removeTab
  };   // end of SecondaryTaskTabWidget

  Buffer::Buffer(const int i, EditorWidget &w)
      : QWidget(&w),
        editor(w),
        splitter(nullptr),
        stw(new SecondaryTaskTabWidget(this)),
        e(new PlainTextEdit(w, *this)),
        id(i) {
    this->initialize();
  }  // end of Buffer

  Buffer::Buffer(const QString &f, const int i, EditorWidget &w)
      : QWidget(&w),
        editor(w),
        splitter(nullptr),
        stw(new SecondaryTaskTabWidget(this)),
        e(new PlainTextEdit(f, w, *this)),
        id(i) {
    this->initialize();
    this->emitNewTreatedFile(this->e->getCompleteFileName());
  }  // end of Buffer

  void Buffer::initialize() {
    // setting
    this->e->setMainFrame(true);
    this->e->setFocus();
    // without SecondaryTasks, the SecondaryTasks tab widget is
    // hidden
    this->stw->setTabPosition(QTabWidget::South);
    this->stw->setDocumentMode(true);
    this->stw->setTabsClosable(true);
    // setFontSizeAndContentsSize(this->stw->tabBar());
    if (this->stw->count() == 0) {
      this->stw->hide();
    }
    this->info = new QHBoxLayout;
    this->timer = new QTimer(this);
    this->bni = new QLabel(this);
    this->rpi = new QLabel(this);
    this->api = new QLabel(this);
    this->ii = new QLabel(this);
    this->mi = new QLabel(this);
    this->ti = new QLabel(this);
    setQLabelFontSizeAndMargins(this->bni);
    setQLabelFontSizeAndMargins(this->rpi);
    setQLabelFontSizeAndMargins(this->api);
    setQLabelFontSizeAndMargins(this->mi);
    setQLabelFontSizeAndMargins(this->ti);
    this->info->addStretch();
    this->info->addWidget(this->bni);
    this->info->addStretch();
    this->info->addWidget(this->rpi);
    this->info->addStretch();
    this->info->addWidget(this->api);
    this->info->addStretch();
    this->info->addWidget(this->ii);
    this->info->addWidget(this->mi);
    this->info->addStretch();
    this->info->addWidget(this->ti);
    this->info->addStretch();
    // the warning shown when the file visited by this buffer is modified
    // by another process: a line above the text naming the file, a button
    // which reloads the file and a button which dismisses the warning, as
    // gedit does. The strip wears the editor's own colors rather than
    // gedit's dark one: it sets no palette of its own, so the plain widget
    // lets the background of the buffer show through — the very background
    // of the information line below the text — and its label takes the
    // font of the mode-line labels
    this->fileModifiedBanner = new QWidget(this);
    this->fileModifiedBanner->setObjectName("file-modified-banner");
    auto *const bfl = new QHBoxLayout;
    bfl->setContentsMargins(6, 2, 6, 2);
    bfl->setSpacing(6);
    this->fileModifiedBannerLabel = new QLabel(this->fileModifiedBanner);
    this->fileModifiedBannerLabel->setTextFormat(Qt::PlainText);
    setQLabelFontSizeAndMargins(this->fileModifiedBannerLabel);
    auto *const rb =
        new QPushButton(QObject::tr("Reload"), this->fileModifiedBanner);
    auto *const db = new QToolButton(this->fileModifiedBanner);
    db->setText(QString::fromUtf8("\xE2\x9C\x95"));
    db->setAutoRaise(true);
    bfl->addWidget(this->fileModifiedBannerLabel);
    bfl->addStretch();
    bfl->addWidget(rb);
    bfl->addWidget(db);
    this->fileModifiedBanner->setLayout(bfl);
    // no style sheet on purpose: a plain widget paints no background, so
    // the strip shows the buffer's own background — the very background the
    // information line is drawn on — and the label keeps the inherited text
    // color, exactly like the mode line
    // a one-line strip: the layout would otherwise give the banner a share
    // of the window's height, since `Preferred` allows growth (verified on
    // a live display: the strip ate the top half of the buffer)
    this->fileModifiedBanner->setSizePolicy(QSizePolicy::Preferred,
                                            QSizePolicy::Fixed);
    this->fileModifiedBanner->setVisible(false);
    QObject::connect(rb, &QPushButton::clicked, this,
                     [this] { this->e->reloadFile(); });
    QObject::connect(db, &QToolButton::clicked, this, [this] {
      // dismissing the warning silences the banner for the state the
      // file is in; a further change on disk is worth a warning again,
      // and a save still confirms before overwriting those changes
      this->e->acknowledgeFileModifiedOnDisk();
      this->hideFileModifiedBanner();
    });
    // main gui
    auto *vl = new QVBoxLayout;
    auto *hl = new QHBoxLayout;
    vl->addWidget(this->fileModifiedBanner);
    this->splitter = new QSplitter;
    this->splitter->setOrientation(Qt::Vertical);
    QSizePolicy qs;
    qs.setHorizontalPolicy(QSizePolicy::Minimum);
    qs.setVerticalPolicy(QSizePolicy::Minimum);
    this->e->setSizePolicy(qs);
    qs.setHorizontalPolicy(QSizePolicy::Maximum);
    qs.setVerticalPolicy(QSizePolicy::Maximum);
    this->bni->setSizePolicy(qs);
    this->rpi->setSizePolicy(qs);
    this->api->setSizePolicy(qs);
    this->mi->setSizePolicy(qs);
    this->ti->setSizePolicy(qs);
    //    hl->setMargin(0);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(0);
    this->splitter->addWidget(this->e);
    this->splitter->addWidget(this->stw);
    hl->addWidget(this->splitter);
    //    vl->setMargin(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    //    vl->addWidget(this->e);
    //    vl->addWidget(this->stw);
    vl->addLayout(hl);
    vl->addLayout(this->info);
    // the information of the buffer is displayed now, the suffix of its name
    // being chosen once the buffer is added to the stack of buffers, as the
    // suffix depends on the files visited by all the buffers (see
    // EditorWidget::updateBufferNameSuffixes)
    this->updateBufferInformations();
    this->setLayout(vl);
    // timer
    QObject::connect(this->timer, &QTimer::timeout, this, &Buffer::updateDate);
    // watch the visited file: a buffer which is visible or not, its file can
    // be written over by another process at any time (issue: avoid cross
    // modifications by warning the user as soon as the disk disagrees with
    // the buffer)
    QObject::connect(this->timer, &QTimer::timeout, this,
                     [this] { this->e->checkFileModifiedOnDisk(); });
    // update every second
    this->timer->start(1000);
    // connecting signals
    QObject::connect(this->e, &PlainTextEdit::fileNameChanged, this,
                     &Buffer::emitNewTreatedFile);
    QObject::connect(this->e, &PlainTextEdit::fileNameChanged, this,
                     &Buffer::updateBufferName);
    QObject::connect(this->e, &PlainTextEdit::cursorPositionChanged, this,
                     &Buffer::updatePosition);
    QObject::connect(this->e, &PlainTextEdit::cursorPositionChanged, this,
                     &Buffer::updateDate);
    // updateBufferName and not updateBufferInformations is connected here: a
    // major mode change may follow a file name change (see
    // TextEditBase::setFileName which sets the file name and emits
    // majorModeChanged before fileNameChanged) and updateBufferInformations
    // displays the name of the buffer, which holds the suffix re-chosen by
    // EditorWidget::updateBufferNameSuffixes
    QObject::connect(this->e, &PlainTextEdit::majorModeChanged, this,
                     &Buffer::updateBufferName);
    QObject::connect(this->e, &PlainTextEdit::majorModeChanged, this,
                     [this] { emit mainFrameMajorModeChanged(); });
    QObject::connect(this->stw, &SecondaryTaskTabWidget::tabCloseRequested,
                     this, &Buffer::closeSecondaryTask);
    QObject::connect(this->stw, &SecondaryTaskTabWidget::currentChanged, this,
                     [this]() {
                       auto *const c = this->stw->currentWidget();
                       if (c != nullptr) {
                         this->editor.setCurrentSecondaryTask(this, c);
                       }
                     });
  }  // end of Buffer::initialize

  int Buffer::getId() const { return this->id; }  // end of getId

  void Buffer::emitNewTreatedFile(const QString &f) {
    emit newTreatedFile(f);
  }  // end of emitNewTreatedFile

  void Buffer::changeFont(const QFont &f) {
    this->e->setFont(f);
    // the icon of the information line is the only label which is not given the
    // font of the text edits when the buffer is built: it holds a pixmap, which
    // Qt draws with the application font anyway
    for (auto *l : {this->bni, this->rpi, this->api, this->mi, this->ti}) {
      l->setFont(f);
    }
    // the banner of a file modified on disk reads with the same font as the
    // information line, so it follows the font dialog too; the pointer is
    // null when a font change reaches a buffer whose banner was not built
    if (this->fileModifiedBannerLabel != nullptr) {
      this->fileModifiedBannerLabel->setFont(f);
    }
    // the tab bar of the secondary tasks is not a child of the main frame
    // either, and the font given to it at construction wins over the font of
    // this widget, so it has to be handled here too
    this->stw->tabBar()->setFont(f);
    // a text edit used as a secondary task, such as the list of completions of
    // a command line, reads the font of the text edits when it is built, which
    // a command line does the first time it completes. A scroll area attached
    // as a secondary task reads it when it is attached, and is held by a
    // wrapper, which is what the tab widget shows
    for (int i = 0; i != this->stw->count(); ++i) {
      auto *const w = this->stw->widget(i);
      if (auto *const t = qobject_cast<TextEditBase *>(w)) {
        t->setFont(f);
        continue;
      }
      if (auto *const a = qobject_cast<QAbstractScrollArea *>(w)) {
        a->setFont(f);
        continue;
      }
      if (auto *const r = qobject_cast<QAbstractScrollAreaWrapper *>(w)) {
        if (auto *const a =
                qobject_cast<QAbstractScrollArea *>(r->getWrappedObject())) {
          a->setFont(f);
        }
      }
    }
  }  // end of changeFont

  QString Buffer::getBufferRawName() const {
    const auto f = this->e->getFileName();
    QFileInfo fi(f);
    if (fi.isDir()) {
      return QDir(f).dirName();
    }
    return fi.fileName();
  }  // end of getBufferRawName

  QString Buffer::getBufferName() const {
    const auto s = this->getBufferNameSuffix();
    const auto f = this->getBufferRawName();
    if (!s.isEmpty()) {
      // the separating space is dropped when the buffer visits no file, as
      // there is nothing in front of the suffix
      return f.isEmpty() ? ("<" + s + ">") : (f + " <" + s + ">");
    }
    return f;
  }  // end of getBufferName

  QString Buffer::getBufferNameSuffix() const {
    return this->bufferNameSuffix;
  }  // end of getBufferNameSuffix

  void Buffer::setBufferNameSuffix(const QString &s) {
    const auto o = this->getBufferName();
    this->bufferNameSuffix = s;
    // the suffix is known from now on, so the information of the buffer,
    // which displays the name of this buffer, can be refreshed
    this->updateBufferInformations();
    const auto n = this->getBufferName();
    // the name may be left untouched: the suffixes of all the buffers are
    // re-chosen at once, and only a few of them usually change
    if (o != n) {
      emit bufferNameChanged(this, o, n);
    }
  }  // end of setBufferNameSuffix

  void Buffer::focusInEvent(QFocusEvent *) {
    if (!this->e->hasFocus()) {
      this->e->setFocus();
    }
  }  // end of focusInEvent

  void Buffer::updateBufferName() {
    // the suffix of the name of this buffer depends on the files visited by
    // all the other buffers, so the whole set of buffers is refreshed
    this->editor.updateBufferNameSuffixes();
  }  // end of updateBufferName()

  std::vector<QMenu *> Buffer::getSpecificMenus() {
    return this->e->getSpecificMenus();
  }  // end of getSpecificMenu

  QIcon Buffer::getIcon() const {
    return this->e->getIcon();
  }  // end of getIcon

  void Buffer::updateDate() {
    QTime t = QTime::currentTime();
    int h = t.hour();
    int m = t.minute();
    QString tl;
    if (h < 9) {
      tl += "0" + QString::number(h);
    } else {
      tl += QString::number(h);
    }
    tl += ":";
    if (m < 9) {
      tl += "0" + QString::number(m);
    } else {
      tl += QString::number(m);
    }
    this->ti->setText(tl);
  }

  void Buffer::updatePosition() {
    const auto &c = this->e->textCursor();
    const auto &d = *(this->e->document());
    const auto cn = c.blockNumber() + 1;
    const auto bn = d.blockCount();
    if (bn > 0) {
      if (cn == bn) {
        this->rpi->setText("100%");
      } else {
        QString p =
            QString::number(static_cast<int>(qreal(cn) / qreal(bn) * 100.)) +
            "%";
        this->rpi->setText(p);
      }
    }
    QTextCursor b(c);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    this->api->setText("(" + QString::number(cn) + "," +
                       QString::number(c.position() - b.position()) + ")");
  }

  void Buffer::updateBufferInformations() {
    // getBufferName() holds the suffix which distinguishes the buffers
    // visiting files with the same name, so it shall not be appended once
    // again. This method must be called once this suffix has been chosen: it
    // always is as the only caller is setBufferNameSuffix, which is called
    // with the new suffix, apart from the very first display performed by
    // initialize
    //
    // The name is escaped as the surrounding bold tags make the label parse
    // its text as rich text: without an escape, the <tfel> suffix of a buffer
    // visiting CMakeLists.txt for instance is taken for an html tag and the
    // name is displayed with no suffix at all, so that all the buffers
    // visiting files with the same name look alike
    this->bni->setText("<b>" + this->getBufferName().toHtmlEscaped() + "</b>");
    this->updatePosition();
    const auto ic = this->e->getIcon();
    if (!ic.isNull()) {
      this->ii->setPixmap(ic.pixmap(12, 12));
      this->ii->show();
    } else {
      this->ii->hide();
    }
    if (this->e->hasMajorMode()) {
      if (!ic.isNull()) {
        this->mi->setText(" " + this->e->getMajorMode().getName());
      } else {
        this->mi->setText(this->e->getMajorMode().getName());
      }
      this->mi->show();
    } else {
      this->mi->hide();
    }
    this->updateDate();
  }  // end Buffer::updateBufferInformations

  PlainTextEdit &Buffer::getMainFrame() {
    return *(this->e);
  }  // end of getTextEdit

  QWidget *Buffer::getCurrentSecondaryTask() {
    if (this->stw->isVisible()) {
      return this->stw->currentWidget();
    }
    return nullptr;
  }  // end of getCurrentSecondaryTask

  bool Buffer::hasSecondaryTasks() const {
    if (this->stw == nullptr) {
      return false;
    }
    return this->stw->count() != 0;
  }

  QWidget *Buffer::attachSecondaryTask(const QString &t, QWidget *const s) {
    if (s == nullptr) {
      return nullptr;
    }
#ifdef TFEL_GUI_HAVE_WEBENGINE
    auto *pw = qobject_cast<QWebEngineView *>(s);
#endif /* TFEL_GUI_HAVE_WEBENGINE */
    auto *p = qobject_cast<QAbstractScrollArea *>(s);
    SecondaryTask st;
    if (t.size() > 8) {
      st.title = t.left(8);
    } else {
      st.title = t;
    }
    st.current = true;
#ifdef TFEL_GUI_HAVE_WEBENGINE
    if (pw != nullptr) {
      auto *w = new QWebEngineViewWrapper(pw, this);
      // left registering the inner widget on purpose: this branch is not
      // compiled (WebEngine is never found, see cmake/modules/qt.cmake) and
      // QWebEngineViewWrapper has no getWrappedObject(), so the identity
      // resolution in SecondaryTaskManager could not undo it. Re-enable
      // together with an accessor if WebEngine ever comes back.
      st.w = s;
      this->editor.attachSecondaryTask(this, st);
      if (this->isVisible()) {
        this->stw->addTab(w, st.title);
        this->stw->setCurrentWidget(w);
        this->stw->show();
      }
      return w;
    } else if (p != nullptr) {
#else  /* TFEL_GUI_HAVE_WEBENGINE */
    if (p != nullptr) {
#endif /* TFEL_GUI_HAVE_WEBENGINE */
      auto *w = new QAbstractScrollAreaWrapper(p, this);
      // register the wrapper, not the wrapped widget: the wrapper is what the
      // tab widget holds and what attachSecondaryTask returns, so the registry
      // and the tab must agree on one identity. Registering the inner widget
      // here is what kept tasks from ever being unregistered and leaked the
      // wrapper.
      st.w = w;
      this->editor.attachSecondaryTask(this, st);
      if (this->isVisible()) {
        this->stw->addTab(w, st.title);
        this->stw->setCurrentWidget(w);
        this->stw->show();
      }
      return w;
    }
    st.w = s;
    this->editor.attachSecondaryTask(this, st);
    if (this->isVisible()) {
      this->stw->addTab(s, st.title);
      this->stw->setCurrentWidget(s);
      this->stw->show();
    }
    return s;
  }  // end of attachSecondaryTask

  void Buffer::attachSecondaryTask(QWidget *const p) {
    if (p == nullptr) {
      return;
    }
    if (this->getSecondaryTaskIndex(p) != -1) {
      return;
    }
    const auto &st = this->editor.attachSecondaryTask(this, p);
    if (st.w != nullptr) {
      this->stw->addTab(st.w, st.icon, st.title);
      this->stw->setCurrentWidget(st.w);
      this->stw->show();
    }
  }  // end of attachSecondaryTask

  int Buffer::getSecondaryTaskIndex(QWidget *const p) const {
    if (p == nullptr) {
      return -1;
    }
    for (int i = 0; i != this->stw->count(); ++i) {
      auto *pi = this->stw->widget(i);
      if (pi == p) {
        return i;
      }
      auto *w = qobject_cast<QAbstractScrollAreaWrapper *>(pi);
      if (w != nullptr) {
        if (p == w->getWrappedObject()) {
          return i;
        }
      }
    }
    return -1;
  }  // end of getSecondaryTaskIndex

  QString Buffer::getSecondaryTaskTitle(QWidget *const p) const {
    QString n;
    const auto i = this->getSecondaryTaskIndex(p);
    if (i != -1) {
      n = this->stw->tabText(i);
    }
    return n;
  }

  void Buffer::setSecondaryTaskIcon(QWidget *const p, const QIcon &i) {
    const auto sid = this->getSecondaryTaskIndex(p);
    if (sid != -1) {
      this->editor.setSecondaryTaskIcon(p, i);
      this->stw->setTabIcon(sid, i);
    }
  }

  void Buffer::setSecondaryTaskTitle(QWidget *const p, const QString &n) {
    const auto i = this->getSecondaryTaskIndex(p);
    if (i != -1) {
      this->editor.setSecondaryTaskTitle(p, n);
      this->stw->setTabText(i, n.left(8));
    }
  }  // end of setSecondaryTaskName

  void Buffer::refreshSecondaryTaskTabWidget() {
    this->stw->clear();
    const auto &tasks = this->editor.getSecondaryTasks(this);
    if (tasks.empty()) {
      return;
    }
    auto current = [&tasks]() -> QWidget * {
      for (const auto &t : tasks) {
        if (t.current) {
          return t.w;
        }
      }
      return nullptr;
    }();
    auto visible = false;
    for (const auto &t : tasks) {
      if (t.w != nullptr) {
        this->stw->addTab(t.w, t.icon, t.title);
        visible = visible || t.visible;
      }
    }
    if (visible) {
      this->stw->show();
    } else {
      this->stw->hide();
    }
    if (current != nullptr) {
      this->stw->setCurrentWidget(current);
    }
  }  // end of refreshSecondaryTaskTabWidget

  void Buffer::removeSecondaryTask(QWidget *const s) {
    if (s == nullptr) {
      return;
    }
    for (int i = 0; i != this->stw->count(); ++i) {
      if (s == this->stw->widget(i)) {
        this->stw->removeTab(i);
        this->editor.detachSecondaryTask(this, s);
        if (this->stw->count() == 0) {
          this->stw->hide();
        }
        return;
      }
    }
  }  // end of removeSecondaryTask

  void Buffer::showSecondaryTask(QWidget *const s) {
    if (s == nullptr) {
      return;
    }
    // look if all secondary tasks are hidden
    this->editor.showSecondaryTask(this, s);
    for (int i = 0; i != this->stw->count(); ++i) {
      auto *w = this->stw->widget(i);
      if (s == w) {
        s->show();
        this->stw->show();
      }
    }
  }  // end of showSecondaryTask

  void Buffer::hideSecondaryTask(QWidget *const s) {
    if (s == nullptr) {
      return;
    }
    // look if all secondary tasks are hidden
    auto h = true;
    this->editor.hideSecondaryTask(this, s);
    for (int i = 0; i != this->stw->count(); ++i) {
      auto *w = this->stw->widget(i);
      if (s == w) {
        s->hide();
      }
      h = h && (!w->isVisible());
    }
    if (h) {
      this->stw->hide();
    }
  }  // end of hideSecondaryTask

  void Buffer::focusCurrentSecondaryTask() {
    if (this->stw != nullptr) {
      auto *s = this->stw->currentWidget();
      if (s != nullptr) {
        s->setFocus();
      }
    }
  }  // end of focusCurrentSecondaryTask

  bool Buffer::areSecondaryTasksVisible() const {
    if (this->stw == nullptr) {
      return false;
    }
    return this->stw->isVisible();
  }

  void Buffer::focusMainFrame() {
    this->e->setFocus();
  }  // end of focusMainFrame

  void Buffer::showSecondaryTasks() {
    if (this->stw->count() == 0) {
      this->editor.displayInformativeMessage(
          QObject::tr("no SecondaryTask to be shown"));
      return;
    }
    this->stw->show();
  }  // end of showSecondaryTasks

  void Buffer::setSecondaryTasksOrientation(const Qt::Orientation o) {
    this->splitter->setOrientation(o);
  }  // end of setSecondaryTasksOrientation

  void Buffer::hideSecondaryTasks() {
    this->stw->hide();
  }  // end of hideSecondaryTasks

  void Buffer::closeSecondaryTask(int i) {
    debug("Buffer::closeSecondaryTask: removing tab", i);
    auto *const w = this->stw->widget(i);
    if (w == nullptr) {
      return;
    }
    // detachSecondaryTask queues a deleteLater on the widget, so the tab must
    // go first: if the removal is refused, the deferred delete still fires and
    // the tab widget is left holding a dangling page.
    this->stw->removeTab(i);
    this->editor.detachSecondaryTask(this, w);
    if (this->stw->count() == 0) {
      this->stw->hide();
    }
  }  // end of closeSecondaryTask

  bool Buffer::isOkToClose() const {
    return !this->e->document()->isModified();
  }  // end of isOkToClose

  void Buffer::showFileModifiedBanner(const QString &f) {
    if (this->fileModifiedBannerLabel == nullptr) {
      return;
    }
    this->fileModifiedBannerLabel->setText(
        QObject::tr("file '%1' has been modified on disk").arg(f));
    // while the warning is up, the buffer can not be edited: saving over
    // what another program wrote has to stay a decision, not a slip of the
    // fingers. `isReadOnly` stops the editing commands and the self-insert
    // of keys, while the arrow keys, the mark-based selection, the mouse
    // selection and PageUp/PageDown/Home/End keep working, as they do in
    // every read-only buffer of the editor; the state the frame was in
    // before the freeze is kept, so a frame the mode had already set
    // read-only (a directory buffer, say) is not handed back writable
    if (!this->mainFrameFrozenByBanner) {
      this->mainFrameEditableBeforeBanner = !this->e->isReadOnly();
      if (this->mainFrameEditableBeforeBanner) {
        this->e->setReadOnly(true);
      }
      this->mainFrameFrozenByBanner = true;
    }
    this->fileModifiedBanner->setVisible(true);
  }  // end of showFileModifiedBanner

  void Buffer::hideFileModifiedBanner() {
    // the freeze lasts exactly as long as the banner: whichever way the
    // disagreement was settled — a reload, a save which overwrote the
    // file, or the dismissal — the buffer is the user's again; the
    // banner pointers, and with them the freeze flag, are declared above
    // the main frame, so this runs safely from that frame's constructor,
    // where the flag answers `false` and the main frame is not touched
    if (this->mainFrameFrozenByBanner) {
      this->e->setReadOnly(!this->mainFrameEditableBeforeBanner);
      this->mainFrameFrozenByBanner = false;
    }
    // the main frame of a buffer visiting a file is built before the banner
    // exists (see the constructor), and it clears the banner from
    // `setFileName`, which is where that visit is declared
    if (this->fileModifiedBanner != nullptr) {
      this->fileModifiedBanner->setVisible(false);
    }
  }  // end of hideFileModifiedBanner

  void Buffer::closeCurrentSecondaryTask() {
    if (this->stw->count() != 0) {
      this->closeSecondaryTask(this->stw->currentIndex());
    }
  }  // end of closeCurrentSecondaryTask

  void Buffer::updateMenu() { emit updatedMenu(); }  // end of updateMenu

  // Unregister this buffer's secondary tasks. The registry is owned by the
  // EditorWidget, which outlives any single buffer, so without this a Buffer
  // could leave entries behind whose widget is already gone.
  //
  // The call is qualified with SecondaryTaskManager:: on purpose:
  // EditorWidget::removeBuffer overrides it and does full UI work, including
  // createEmptyBuffer() and deleteLater on the buffer being destroyed, which
  // must not happen from a destructor.
  Buffer::~Buffer() { this->editor.SecondaryTaskManager::removeBuffer(this); }

}  // end of namespace tfel::gui
