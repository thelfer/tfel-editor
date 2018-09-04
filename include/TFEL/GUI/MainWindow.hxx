/*!
 * \file  MainWindow.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#ifndef LIB_TFEL_GUI_MAINWINDOW_HXX
#define LIB_TFEL_GUI_MAINWINDOW_HXX

#include <QtCore/QStringList>
#include <QtWidgets/QMainWindow>
#include "TFEL/GUI/Config.hxx"

namespace tfel{

  namespace gui{

  struct MainWindow : public QMainWindow {
    MainWindow(const QStringList & = QStringList(),
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

  };  // end of class MainWindow

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MAINWINDOW_HXX */
