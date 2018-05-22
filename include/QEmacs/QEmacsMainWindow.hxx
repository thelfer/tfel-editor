/*!
 * \file  QEmacsMainWindow.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#ifndef LIB_QEMACS_QEMACSMAINWINDOW_HXX
#define LIB_QEMACS_QEMACSMAINWINDOW_HXX

#include <QtCore/QStringList>
#include <QtWidgets/QMainWindow>
#include "QEmacs/Config.hxx"

namespace qemacs {

  struct QEmacsMainWindow : public QMainWindow {
    QEmacsMainWindow(const QStringList & = QStringList(),
                     QWidget *const = nullptr);

    virtual QSize sizeHint() const override;

   protected:
    virtual void closeEvent(QCloseEvent *) override;

   protected slots:

    virtual void about();

    virtual void undo();

    virtual void redo();

    virtual void cut();

    virtual void copy();

    virtual void paste();

    virtual void selectAll();

    virtual void print();

    virtual void updateBuffersMenu();

    virtual void updateOptionsMenu();

    virtual void createMainMenu();

    virtual void openFile();

    virtual void selectFont();

    virtual void useEmacsShortCuts();

    virtual void useQtShortCuts();

    virtual void addToRecentFiles(const QString &);

   protected:
    virtual void createActions();

    virtual void createRecentFilesMenu(QMenu *const,
                                       const QStringList &);

    QAction *na;
    QAction *oa;
    QAction *sa;
    QAction *saa;
    QAction *ka;
    QAction *pra;
    QAction *ea;

    QAction *ua;
    QAction *ra;
    QAction *ca;
    QAction *ca2;
    QAction *pa;
    QAction *sea;

    QAction *fa;
    QAction *esca;
    QAction *qsca;
    QAction *aa;
    QAction *aa2;

    QMenu *fm;
    QMenu *em;
    QMenu *om;
    QMenu *bm;
    QMenu *hm;

   private:
    Q_OBJECT

  };  // end of class QEmacsMainWindow

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSMAINWINDOW_HXX */
