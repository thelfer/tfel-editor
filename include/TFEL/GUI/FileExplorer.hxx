/*!
 * \file  FileExplorer.hxx
 * \brief panel showing the file system, which opens in a buffer the file it is
 * double clicked on.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#ifndef LIB_TFEL_GUI_FILEEXPLORER_HXX
#define LIB_TFEL_GUI_FILEEXPLORER_HXX

#include <QtCore/QString>
#include <QtCore/QModelIndex>
#include <QtGui/QFont>
#include <QtGui/QFileSystemModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  //! forward declaration
  struct EditorWidget;

  /*!
   * \brief panel showing the contents of a directory, on the left of the
   * buffers, which opens in a new buffer the file it is double clicked on
   *
   * The panel is a `QTreeView` showing a `QFileSystemModel`: the model is the
   * one Qt provides for the file system, so the entries come with the icon the
   * desktop gives them, they appear and disappear as they are created or
   * removed on disk, and the order they are read in is the model's own, which
   * puts the directories before the files and both in alphabetical order.
   *
   * \note the panel stands in the `QSplitter` the editor holds above its
   * minibuffer, which is what makes it as tall as the buffers and what keeps it
   * clear of the informative message area and the line the user types in.
   *
   * \note Qt has no class named `QFileModelView`: the pair which shows the file
   * system through a model is `QFileSystemModel` — in the `QtGui` module since
   * Qt 6, not in `QtWidgets` where it used to be — and one of the views of
   * `QtWidgets`.
   *
   * \note asking the view to sort is not what orders the entries: the model
   * answers that order without being asked, and a second model of the same
   * directory, read by a view which asks nothing, answers the same order. That
   * is why no `QSortFilterProxyModel` stands between the model and the tree.
   *
   * \note the model is rooted through `setRootPath`: an index asked for a
   * directory the model knows nothing of is valid and has no row, which
   * reads as an empty directory rather than as a model which looked at
   * nothing. That is why `setRootDirectory` is the one place where the
   * path of the model and the root index of the view are both set.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT FileExplorer : public QWidget {
    Q_OBJECT
   public:
    /*!
     * \brief constructor
     * \param[in] e : editor the double clicked files are opened in
     * \param[in] p : parent widget
     *
     * The panel is rooted at the current working directory and wears the
     * font of the `font/textedit` setting, like the widgets which read that
     * setting when they are built; `changeFont` is what hands it a font
     * chosen later.
     */
    FileExplorer(EditorWidget &e, QWidget *const p = nullptr);

    //! destructor
    ~FileExplorer() override;

    /*!
     * \return the directory whose contents the panel shows, an empty name when
     * the model is rooted at nothing
     */
    QString rootDirectory() const;

    /*!
     * \brief show the contents of the given directory
     * \param[in] d : name of the directory; nothing happens when it is empty or
     * names something which is not a directory, since a panel which went blank
     * on a typo is worse than one which stayed where it was
     *
     * The tree is rooted at that directory: its entries are the ones shown and
     * the directories above it are out of the way, which is what `goUp` and
     * `chooseDirectory` are for.
     */
    void setRootDirectory(const QString &);

    /*!
     * \brief change the font of the tree and of the line which names the
     * directory the panel is rooted at
     * \param[in] f : new font
     *
     * \note called by `EditorWidget::changeFont`: the tree wears the font of
     * the text edits from its constructor on, and Qt never replaces an
     * explicit font with the inherited one.
     */
    void changeFont(const QFont &);

    /*!
     * \brief open the file the given index names in a new buffer of the editor
     * \param[in] i : index of the tree; a directory is not opened, and so is an
     * index which names nothing, which is what a double click on the blank part
     * of the tree answers
     *
     * \note a file already visited keeps its buffer: what reaches the screen is
     * that buffer, not a new one reading the file again.
     */
    virtual void openFileAt(const QModelIndex &);

    //! \brief root the panel at the directory which holds the one it shows
    virtual void goUp();

    /*!
     * \brief ask for a directory through a file dialog and root the panel at it
     *
     * A dialog which is cancelled leaves the panel where it was.
     */
    virtual void chooseDirectory();

   protected:
    //! \brief show in the header the directory the panel is rooted at
    void updateHeader();

    //! the editor the double clicked files are opened in
    EditorWidget &editor;
    //! the model of the file system, rooted at the directory shown
    QFileSystemModel *model;
    //! the tree showing the entries of that directory
    QTreeView *tree;
    //! the line of the header naming the directory the panel is rooted at
    QLabel *path;
    //! the button of the header which roots the panel at the parent directory
    QToolButton *up;
    //! the button of the header which asks for another directory
    QToolButton *choose;

   };  // end of struct FileExplorer

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_FILEEXPLORER_HXX */
