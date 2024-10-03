/*!
 * \file  Utilities.cxx
 * \brief
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAbstractScrollArea>
#ifdef TFEL_GUI_HAVE_WEBENGINE
#include <QtWebEngineWidgets/QWebEngineView>
#endif /* TFEL_GUI_HAVE_WEBENGINE */
#include "TFEL/GUI/Utilities.hxx"

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif /* Q_OS_UNIX */

namespace tfel::gui {

  QString findFileInDirectoryOrParentDirectory(const QString& d,
                                               const QString& f) {
    auto dir = QDir(d);
    auto fi = QFileInfo(dir.absolutePath() + QDir::separator() + f);
    if (fi.exists()) {
      return fi.absolutePath();
    }
    while (dir.cdUp()) {
      fi.setFile(dir.absolutePath() + QDir::separator() + f);
      return fi.absolutePath();
    }
    return "";
  }  // end of findFileInDirectoryOrParentDirectory()

  void setFontSizeAndContentsMargins(QWidget* const w) {
    auto f = w->font();
    f.setPointSize(8);
    w->setFont(f);
    w->setContentsMargins(0, 0, 0, 0);
  }  // end of setQLabelFontSize

  void setQLabelFontSizeAndMargins(QLabel* const l) {
    setFontSizeAndContentsMargins(l);
    l->setMargin(0);
  }  // end of setQLabelFontSize

  void setQAbstractScrollAreaInLayout(QHBoxLayout* const hl,
                                      QAbstractScrollArea* const a) {
    a->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* s = a->verticalScrollBar();
    hl->addWidget(s);
    hl->addWidget(a);
    //    hl->setMargin(0);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(0);
  }

#ifdef TFEL_GUI_HAVE_WEBENGINE
  void setQWebEngineViewInLayout(QHBoxLayout* const hl,
                                 QWebEngineView* const a) {
    // a->page()->setScrollBarPolicy(Qt::Vertical,
    // 				  Qt::ScrollBarAlwaysOff);
    // QScrollBar* s = a->page()->mainFrame()->verticalScrollBar();
    //    hl->addWidget(s);
    hl->addWidget(a);
    hl->setMargin(0);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(0);
  }
#endif /* TFEL_GUI_HAVE_WEBENGINE */

  QString commonPart(const QString& s1, const QString& s2) {
    const auto l = qMin(s1.size(), s2.size());
    if (l == 0) {
      return QString();
    }
    QString r;
    for (int i = 0; i != l; ++i) {
      if (s1[i] != s2[i]) {
        break;
      }
      r += s1[i];
    }
    return r;
  }  // end of commonPart

  QString commonPart(const QStringList& l) {
    if (l.empty()) {
      return QString();
    }
    if (l.size() == 1) {
      return l[0];
    }
    auto r = commonPart(l[0], l[1]);
    if(l.size() > 2){
      for (int i = 2; i != l.size(); ++i) {
	const auto& s = l[i];
	if (!s.startsWith(r)) {
	  r = commonPart(r, s);
	}
	if (r.isEmpty()) {
	  return "";
	}
      }
    }
    return r;
  }

  QString fileNameRegExp() { return "[\\w0-9._+-]+"; }  // end of fileNameRegExp

  void clearMenu(QMenu* const m) {
    for (auto* a : m->actions()) {
      if (a->isSeparator()) {
      } else if (a->menu()) {
        clearMenu(a->menu());
        a->menu()->deleteLater();
      } else {
        a->deleteLater();
      }
    }
    //    m->clear();
  }  // end of clearMenu

  void clearMenuBar(QMenuBar* const m) {
    for (auto a : m->actions()) {
      if (a->isSeparator()) {
      } else if (a->menu()) {
        clearMenu(a->menu());
        a->menu()->deleteLater();
      } else {
        a->deleteLater();
      }
    }
    //    m->clear();
  }  // end of clearMenuBar

#ifdef Q_OS_UNIX

  QString getLoginName() {
    struct passwd* pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);
    if (pw != nullptr) {
      return QString(pw->pw_name);
    }
    return QString();
  }

  QString getUserName() {
    struct passwd* pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);
    if (pw != nullptr) {
      return QString(pw->pw_gecos);
    }
    return QString();
  }

#endif /* Q_OS_UNIX */

}  // end of namespace tfel::gui
