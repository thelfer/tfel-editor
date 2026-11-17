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
#include <QtWidgets/QSplitter>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  //! forward declaration
  struct EditorWidget;
  //! forward declaration
  struct FileExplorer;

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
     *
     * \note the central widget is a horizontal splitter holding the file
     * explorer panel and the editor widget. The panel starts hidden and is
     * shown by the `Display` menu entry `Hide/Show Explorer Panel`; the first
     * time it is shown it takes a third of the width of the window, which the
     * user then changes by dragging the handle of the splitter, and it keeps
     * that width across the later hides and shows
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

    /*!
     * \brief delete the rectangle the selection of the main frame covers and
     * keep it, which is the `cut-rectangle` command
     *
     * \note what is taken goes in the register of `RectangleEditing`, which the
     * kill ring knows nothing of: `Ctrl-Y` does not bring a rectangle back
     */
    virtual void cutRectangle();

    /*!
     * \brief keep the rectangle the selection of the main frame covers without
     * taking it from the buffer, which is the `copy-rectangle` command
     */
    virtual void copyRectangle();

    /*!
     * \brief write the last killed rectangle at the cursor of the main frame,
     * one line at a time, which is the `paste-rectangle` command
     */
    virtual void pasteRectangle();

    /*!
     * \brief show the dialog which searches for a text or a regular expression
     * and replaces it, the `Edit` menu entry `Find/Replace...` and the
     * `find-replace` command
     *
     * \note an editor holds one such dialog and reuses it: what was typed in it
     * is kept, and the buffers stay editable while it is shown
     */
    virtual void showFindReplace();

    /*!
     * \brief show the file explorer panel, or hide it when it is already shown,
     * which is the `Display` menu entry `Hide/Show Explorer Panel`
     *
     * The panel is built by the constructor and hidden: what this slot does the
     * first time it is called is to root it at the directory of the file the
     * current buffer visits, or at the working directory when it visits none,
     * which is the same choice the question of `Ctrl-X Ctrl-F` makes, and to
     * give it a third of the width of the window. Any later call leaves
     * both the directory and the width where the user left them
     */
    virtual void toggleExplorerPanel();

    /*!
     * \brief hide the auxiliary tasks of the current buffer, or show them when
     * they are hidden, which is the `Display` menu entry `Hide/Show Auxiliary
     * Tasks`
     *
     * \note what is shown or hidden is the tab widget of the secondary tasks
     * of the buffer on the screen, the same one `Ctrl-X 1` hides and
     * `Ctrl-X 2` and `Ctrl-X 3` show with the orientation they name. A buffer
     * with no auxiliary task says so, as `Buffer::showSecondaryTasks` does.
     */
    virtual void toggleAuxiliaryTasks();

    /*!
     * \brief tick the two entries of the `Display` menu to what they would
     * show
     *
     * Called when the menu is built and before it is opened, since the state
     * they name can change between the two, by a key of the keyboard or by a
     * buffer switch
     */
    virtual void updateDisplayMenu();

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
     * Edit, Display, Buffers, Editor, mode specific, Options and Help menus;
     * called when the current buffer or its major mode changes
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

    /*!\brief action showing the dialog which searches a buffer and replaces
     * what it finds
     *
     * \note the four actions below this one are added to the `Edit` menu, as
     * the others above are, which is what lets `clearMenuBar` take them back
     * when the menu bar is rebuilt
     */
    QAction *fra = nullptr;
    //! action cutting the rectangle the selection covers
    QAction *rca = nullptr;
    //! action copying the rectangle the selection covers
    QAction *rca2 = nullptr;
    //! action writing the last killed rectangle at the cursor
    QAction *rpa = nullptr;

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

    /*!
     * \brief action showing or hiding the file explorer panel
     *
     * \note this action and the next one are added to the `Display` menu, as
     * the others above are, which is what lets `clearMenuBar` take them back
     * when the menu bar is rebuilt
     */
    QAction *epa = nullptr;
    //! action showing or hiding the auxiliary tasks of the current buffer
    QAction *ataa = nullptr;

    //! the "File" menu
    QMenu *fm;
    //! the "Editor" menu, holding the commands and the major modes
    QMenu *cm;
    //! the "Edit" menu, holding the edition actions
    QMenu *em;
    //! the "Display" menu, holding the two panels which can be hidden
    QMenu *dm;
    //! the "Options" menu
    QMenu *om;
    //! the "Buffers" menu, listing the open buffers
    QMenu *bm;
    //! the "Help" menu
    QMenu *hm;

    /*!
     * \brief editor holding the buffers, which is the right hand side of the
     * splitter below
     *
     * \note what the window shows is held here and not read back from
     * `centralWidget` because the central widget of this window is the
     * splitter, not the editor: the panel is its other half, and a panel
     * which is hidden is still there
     */
    EditorWidget *editor = nullptr;
    //! horizontal splitter holding the explorer panel and the editor
    QSplitter *splitter = nullptr;
    /*!
     * \brief panel showing the file system, hidden until the `Display` menu
     * asks for it
     */
    FileExplorer *explorer = nullptr;
    /*!
     * \brief true once the panel has been given its initial width and
     * directory
     */
    bool explorer_started = false;

   private:
    Q_OBJECT

  };  // end of class MainWindow

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAINWINDOW_HXX */
