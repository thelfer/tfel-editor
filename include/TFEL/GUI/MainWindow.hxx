/*!
 * \file  MainWindow.hxx
 * \brief main window: an editor widget and the menu bar built on top of it.
 * \author Thomas Helfer
 * \date   27/06/2012
 */

#ifndef LIB_TFEL_GUI_MAINWINDOW_HXX
#define LIB_TFEL_GUI_MAINWINDOW_HXX

#include <QtCore/QStringList>
#include <QtWidgets/QMainWindow>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief main window of the editor: holds an EditorWidget as central
   * widget and builds the menu bar on top of it
   *
   * \note exported so the unit tests can drive the real application shape
   * instead of a hand-made `QMainWindow` stand-in, see issue #123
   */
  struct TFEL_GUI_VISIBILITY_EXPORT MainWindow : public QMainWindow {
    /*!
     * \brief constructor
     * \param[in] f : names of the files to open, each one in its own buffer;
     * a single empty buffer is created if this list is empty
     * \param[in] p : parent widget
     *
     * \note the EditorWidget signals are connected so that the menu bar is
     * rebuilt when the current buffer or its major mode changes and the list
     * of recent files is updated when a buffer starts visiting a file
     */
    MainWindow(const QStringList & = QStringList(), QWidget *const = nullptr);

    //! \return the preferred size of the window: 800 times 600 pixels
    virtual QSize sizeHint() const override;

   protected:
    /*!
     * \brief handle the closing of the window: the event is accepted if the
     * central EditorWidget closes, which may prompt for unsaved buffers, and
     * ignored otherwise
     * \param[inout] ev : close event
     */
    virtual void closeEvent(QCloseEvent *) override;

   protected slots:

    //! \brief display a message box describing the application
    virtual void about();

    //! \brief undo the last operation of the main frame of the current buffer
    virtual void undo();

    //! \brief redo the last undone operation of the main frame
    virtual void redo();

    //! \brief cut the selection of the main frame of the current buffer
    virtual void cut();

    //! \brief copy the selection of the main frame of the current buffer
    virtual void copy();

    //! \brief paste the clipboard into the main frame of the current buffer
    virtual void paste();

    //! \brief select the whole text of the main frame of the current buffer
    virtual void selectAll();

    //! \brief print the main frame of the current buffer
    virtual void print();

    /*!
     * \brief rebuild the buffers menu: one entry per buffer, with its name
     * and its icon, switching to the buffer by its id when triggered; called
     * when a buffer is added, removed or renamed
     */
    virtual void updateBuffersMenu();

    /*!
     * \brief rebuild the options menu: font selection, available spell
     * checking dictionaries when hunspell is supported and the actions to
     * switch the short cut style, the entries corresponding to the current
     * style being omitted
     */
    virtual void updateOptionsMenu();

    /*!
     * \brief rebuild the whole menu bar: create the actions, then the File,
     * Edit, Buffers, Editor, mode specific, Options and Help menus; called
     * when the current buffer or its major mode changes
     */
    virtual void createMainMenu();

    /*!
     * \brief ask the user for a file through a file dialog and open it in a
     * new buffer; nothing happens if the dialog is cancelled
     */
    virtual void openFile();

    /*!
     * \brief ask the user for a font through a font dialog: the selected font
     * is stored at the "font/textedit" setting address and applied to all the
     * main frames
     */
    virtual void selectFont();

    /*!
     * \brief open the dialog of the configuration, which lists the entries the
     * editor and its major modes declared they use, gathered by theme, and
     * writes what the user changes; this is the `Options` menu entry
     * "Configuration..." and the \c configure-tfel-editor command
     */
    virtual void openConfigurationDialog();

    /*!
     * \brief switch to the emacs style short cuts, storing this choice in the
     * `ShortCutStyle` singleton and refreshing the options menu
     */
    virtual void useEmacsShortCuts();

    /*!
     * \brief switch to the Qt style short cuts, storing this choice in the
     * `ShortCutStyle` singleton and refreshing the options menu
     */
    virtual void useQtShortCuts();

    /*!
     * \brief add a file to the list of recent files stored at the "recent
     * files" setting address: previous appearances are dropped, the file is
     * appended and the oldest file, with its memorised cursor position, is
     * removed when the list grows over 50 entries
     * \param[in] f : complete name of the file
     *
     * \note triggered by the newTreatedFile signal of the EditorWidget; this
     * persistent list is distinct from the in memory list of
     * EditorWidget::addToRecentFiles
     */
    virtual void addToRecentFiles(const QString &);

   protected:
    /*!
     * \brief (re)create the menu actions of the main window: buffers, file,
     * edition, exit, font, short cut style and help actions, each one
     * connected to the corresponding slot of this window or of the
     * EditorWidget
     */
    virtual void createActions();

    /*!
     * \brief fill a menu with the five most recent readable files, each entry
     * reopening the corresponding file and being iconed with the major mode
     * matching the file name
     * \param[in] m : menu to fill, left empty if no recent file is readable
     * \param[in] files : list of recent files, the last one being the most
     * recent one
     */
    virtual void createRecentFilesMenu(QMenu *const, const QStringList &);

    //! action creating a new empty buffer
    QAction *na = nullptr;
    //! action opening a file through a file dialog
    QAction *oa = nullptr;
    //! action saving the current buffer
    QAction *sa = nullptr;
    //! action saving all the buffers
    QAction *saa = nullptr;
    //! action closing the current buffer
    QAction *ka = nullptr;
    //! action printing the current buffer
    QAction *pra = nullptr;
    //! action exiting the editor
    QAction *ea = nullptr;

    //! action undoing the last operation
    QAction *ua = nullptr;
    //! action redoing the last undone operation
    QAction *ra = nullptr;
    //! action cutting the selection
    QAction *ca = nullptr;
    //! action copying the selection
    QAction *ca2 = nullptr;
    //! action pasting the clipboard
    QAction *pa = nullptr;
    //! action selecting all the text
    QAction *sea = nullptr;

    //! action selecting the font of the main frames
    QAction *fa = nullptr;
    //! action opening the dialog of the configuration
    QAction *cfa = nullptr;
    //! action switching to the emacs style short cuts
    QAction *esca = nullptr;
    //! action switching to the Qt style short cuts
    QAction *qsca = nullptr;
    //! action showing the about message box
    QAction *aa = nullptr;
    //! action showing the Qt about message box
    QAction *aa2 = nullptr;

    //! the "File" menu
    QMenu *fm;
    //! the "Editor" menu, holding the commands and the major modes
    QMenu *cm;
    //! the "Edit" menu, holding the edition actions
    QMenu *em;
    //! the "Options" menu
    QMenu *om;
    //! the "Buffers" menu, listing the open buffers
    QMenu *bm;
    //! the "Help" menu
    QMenu *hm;

   private:
    Q_OBJECT

  };  // end of class MainWindow

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAINWINDOW_HXX */
