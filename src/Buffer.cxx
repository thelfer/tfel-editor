/*!
 * \file  Buffer.cxx
 * \brief
 * \author Helfer Thomas
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
#include <QtWidgets/QSplitter>
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

namespace tfel{

  namespace gui{

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
      this->currentWidget()->setFocus();
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
  };   // end of Buffer::SecondaryTaskTabWidget

  Buffer::Buffer(const int i, EditorWidget &w)
      : QWidget(&w),
        editor(w),
        splitter(nullptr),
        stw(new SecondaryTaskTabWidget(this)),
        e(new PlainTextEdit(w, *this)),
        id(i) {
    this->initialize();
  }  // end of Buffer::Buffer

  Buffer::Buffer(const QString &f,
                             const int i,
                             EditorWidget &w)
      : QWidget(&w),
        editor(w),
        splitter(nullptr),
        stw(new SecondaryTaskTabWidget(this)),
        e(new PlainTextEdit(f, w, *this)),
        id(i) {
    this->initialize();
    this->emitNewTreatedFile(this->e->getCompleteFileName());
  }  // end of Buffer::Buffer

  bool Buffer::eventFilter(QObject *o, QEvent *ev) {
    if (!this->isVisible()) {
      return false;
    }
    auto *const w = qobject_cast<QWidget *>(o);
    if (!((w == this->e->widget()) ||
          (this->stw->indexOf(w) != -1))) {
      return false;
    }
    if (ev->type() == QEvent::KeyPress) {
    }
    return QObject::eventFilter(o, ev);
  }  // end of Buffer::eventFilter

  void Buffer::initialize() {
    // setting
    this->e->setMainFrame(true);
    this->e->setFocus();
    this->e->widget()->installEventFilter(this);
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
    // main gui
    auto *vl = new QVBoxLayout;
    auto *hl = new QHBoxLayout;
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
    hl->setMargin(0);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(0);
    this->splitter->addWidget(this->e);
    this->splitter->addWidget(this->stw);
    hl->addWidget(this->splitter);
    vl->setMargin(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    //    vl->addWidget(this->e);
    //    vl->addWidget(this->stw);
    vl->addLayout(hl);
    vl->addLayout(this->info);
    this->updateBufferInformations();
    this->setLayout(vl);
    // timer
    QObject::connect(this->timer, &QTimer::timeout, this,
                     &Buffer::updateDate);
    // update every second
    this->timer->start(1000);
    // connecting signals
    QObject::connect(this->e, &PlainTextEdit::fileNameChanged,
                     this, &Buffer::emitNewTreatedFile);
    QObject::connect(this->e, &PlainTextEdit::fileNameChanged,
                     this, &Buffer::updateBufferName);
    QObject::connect(this->e,
                     &PlainTextEdit::cursorPositionChanged, this,
                     &Buffer::updatePosition);
    QObject::connect(this->e,
                     &PlainTextEdit::cursorPositionChanged, this,
                     &Buffer::updateDate);
    QObject::connect(this->e, &PlainTextEdit::majorModeChanged,
                     this, &Buffer::updateBufferInformations);
    QObject::connect(this->e, &PlainTextEdit::majorModeChanged,
                     this,
                     [this] { emit mainFrameMajorModeChanged(); });
    QObject::connect(this->stw,
                     &SecondaryTaskTabWidget::tabCloseRequested, this,
                     &Buffer::closeSecondaryTask);
    QObject::connect(this->stw, &SecondaryTaskTabWidget::currentChanged,
                     this, [this]() {
                       this->editor.setCurrentSecondaryTask(
                           this, this->stw->currentWidget());
                     });
    this->updateBufferName();
  }

  int Buffer::getId() const {
    return this->id;
  }  // end of Buffer::getId

  void Buffer::emitNewTreatedFile(const QString &f) {
    emit newTreatedFile(f);
  }  // end of Buffer::emitNewTreatedFile

  QString Buffer::getBufferRawName() const {
    const auto f = this->e->getFileName();
    QFileInfo fi(f);
    if (fi.isDir()) {
      return QDir(f).dirName();
    }
    return fi.fileName();
  }  // end of Buffer::getBufferRawName

  QString Buffer::getBufferName() const {
    const auto s = this->getBufferNameSuffix();
    const auto f = this->getBufferRawName();
    if (!s.isEmpty()) {
      return f + " <" + s + ">";
    }
    return f;
  }  // end of Buffer::getBufferName

  QString Buffer::getBufferNameSuffix() const {
    return this->bufferNameSuffix;
  }  // end of Buffer::setBufferName

  void Buffer::focusInEvent(QFocusEvent *) {
    if (!this->e->hasFocus()) {
      this->e->setFocus();
    }
  }  // end of Buffer::focusInEvent

  void Buffer::updateBufferName() {
    const auto o = this->getBufferName();
    this->bufferNameSuffix = this->editor.chooseBufferNameSuffix(
        this, this->getBufferRawName());
    this->updateBufferInformations();
    const auto n = this->getBufferName();
    emit bufferNameChanged(this, o, n);
  }  // end of Buffer::updateBufferName()

  std::vector<QMenu *> Buffer::getSpecificMenus() {
    return this->e->getSpecificMenus();
  }  // end of Buffer::getSpecificMenu

  QIcon Buffer::getIcon() const {
    return this->e->getIcon();
  }  // end of Buffer::getIcon

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
        QString p = QString::number(static_cast<int>(
                        qreal(cn) / qreal(bn) * 100.)) +
                    "%";
        this->rpi->setText(p);
      }
    }
    QTextCursor b(c);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    this->api->setText("(" + QString::number(cn) + "," +
                       QString::number(c.position() - b.position()) +
                       ")");
  }

  void Buffer::updateBufferInformations() {
    const auto &s = this->getBufferNameSuffix();
    if (s.isEmpty()) {
      this->bni->setText("<b>" + this->getBufferName() + "</b>");
    } else {
      this->bni->setText("<b>" + this->getBufferName() + " &lt;" + s +
                         "&gt;</b>");
    }
    this->updatePosition();
    QIcon ic = this->e->getIcon();
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
  }  // end of Buffer::getTextEdit

  //! \return the current active SecondaryTask if any, nullptr
  //! otherwise
  QWidget *Buffer::getCurrentSecondaryTask() {
    if (this->stw->isVisible()) {
      return this->stw->currentWidget();
    }
    return nullptr;
  }  // end of Buffer::getCurrentSecondaryTask

  QString Buffer::getCurrentSecondaryTaskTitle() const {
    const auto tb = this->stw->tabBar();
    return tb->tabText(this->stw->currentIndex());
  }  // end of Buffer::getCurrentSecondaryTaskTitle

  bool Buffer::hasSecondaryTasks() const {
    if (this->stw == nullptr) {
      return false;
    }
    return this->stw->count() != 0;
  }

  QWidget *Buffer::attachSecondaryTask(const QString &t,
                                             QWidget *const s) {
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
      st.w = s;
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
  }  // end of Buffer::attachSecondaryTask

  void Buffer::attachSecondaryTask(QWidget *const p) {
    if (this->getSecondaryTaskIndex(p) != -1) {
      return;
    }
    const auto &st = this->editor.attachSecondaryTask(this, p);
    if (st.w != nullptr) {
      this->stw->addTab(st.w, st.icon, st.title);
      this->stw->setCurrentWidget(st.w);
      this->stw->show();
    }
  }  // end of Buffer::attachSecondaryTask

  int Buffer::getSecondaryTaskIndex(QWidget *const p) const {
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
  }  // end of Buffer::getSecondaryTaskIndex

  QString Buffer::getSecondaryTaskTitle(QWidget *const p) const {
    QString n;
    const auto i = this->getSecondaryTaskIndex(p);
    if (i != -1) {
      n = this->stw->tabText(i);
    }
    return n;
  }

  void Buffer::setSecondaryTaskIcon(QWidget *const p,
                                          const QIcon &i) {
    const auto sid = this->getSecondaryTaskIndex(p);
    if (sid != -1) {
      this->editor.setSecondaryTaskIcon(p, i);
      this->stw->setTabIcon(sid, i);
    }
  }

  void Buffer::setSecondaryTaskTitle(QWidget *const p,
                                           const QString &n) {
    const auto i = this->getSecondaryTaskIndex(p);
    if (i != -1) {
      this->editor.setSecondaryTaskTitle(p, n);
      this->stw->setTabText(i, n.left(8));
    }
  }  // end of Buffer::setSecondaryTaskName

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
    bool visible = false;
    for (const auto &t : tasks) {
      this->stw->addTab(t.w, t.icon, t.title);
      visible = visible || t.visible;
    }
    if (visible) {
      this->stw->show();
    } else {
      this->stw->hide();
    }
    if (current != nullptr) {
      this->stw->setCurrentWidget(current);
    }
  }  // end of Buffer::refreshSecondaryTaskTabWidget

  void Buffer::removeSecondaryTask(QWidget *const s) {
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
  }  // end of Buffer::removeSecondaryTask

  void Buffer::showSecondaryTask(QWidget *const s) {
    // look if all secondary tasks are hidden
    this->editor.showSecondaryTask(this, s);
    for (int i = 0; i != this->stw->count(); ++i) {
      auto *w = this->stw->widget(i);
      if (s == w) {
        s->show();
        this->stw->show();
      }
    }
  }  // end of Buffer::showSecondaryTask

  void Buffer::hideSecondaryTask(QWidget *const s) {
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
  }  // end of Buffer::hideSecondaryTask

  void Buffer::focusCurrentSecondaryTask() {
    if (this->stw != nullptr) {
      auto *s = this->stw->currentWidget();
      if (s != nullptr) {
        s->setFocus();
      }
    }
  }  // end of Buffer::focusCurrentSecondaryTask

  bool Buffer::areSecondaryTasksVisible() const {
    if (this->stw == nullptr) {
      return false;
    }
    return this->stw->isVisible();
  }

  void Buffer::focusMainFrame() {
    this->e->setFocus();
  }  // end of Buffer::focusMainFrame

  void Buffer::showSecondaryTasks() {
    if (this->stw->count() == 0) {
      this->editor.displayInformativeMessage(
          QObject::tr("no SecondaryTask to be shown"));
      return;
    }
    this->stw->show();
  }  // end of Buffer::showSecondaryTasks

  void Buffer::setSecondaryTasksOrientation(const Qt::Orientation o) {
    this->splitter->setOrientation(o);
  } // end of Buffer::setSecondaryTasksOrientation

  void Buffer::hideSecondaryTasks() {
    this->stw->hide();
  }  // end of Buffer::hideSecondaryTasks

  void Buffer::closeSecondaryTask(int i) {
    debug("Buffer::closeSecondaryTask: removing tab", i);
    this->editor.detachSecondaryTask(this, this->stw->widget(i));
    this->stw->removeTab(i);
    if (this->stw->count() == 0) {
      this->stw->hide();
    }
  }  // end of Buffer::closeSecondaryTask

  bool Buffer::isOkToClose() const {
    return !this->e->document()->isModified();
  }  // end of Buffer::isOkToClose

  void Buffer::closeCurrentSecondaryTask() {
    if (this->stw->count() != 0) {
      this->closeSecondaryTask(this->stw->currentIndex());
    }
  }  // end of Buffer::closeCurrentSecondaryTask

  void Buffer::updateMenu() {
    emit updatedMenu();
  }  // end of Buffer::updateMenu(){

  Buffer::~Buffer() = default;

}  // end of namespace gui
}// end of namespace tfel
