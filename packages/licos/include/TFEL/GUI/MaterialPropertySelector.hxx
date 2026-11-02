/*!
 * \file  MaterialPropertySelector.hxx
 * \brief material properties, their table model and the widget selecting them.
 * \author Thomas Helfer
 * \date   14/12/2012
 */

#ifndef LIB_MATERIALPROPERTYSELECTOR_HXX
#define LIB_MATERIALPROPERTYSELECTOR_HXX

#include <QtCore/QVector>
#include <QtCore/QAbstractItemModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtGui/QTextBlock>

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct LineEdit;

  /*!
   * a material property of a library, as listed by the `mfm-laws` command
   */
  struct MaterialProperty {
    /*!
     * \brief library the property belongs to: either the name of a
     * `MFrontMaterials` library or the path of a dynamic library
     */
    QString library;
    //! name of the property, that is the name of the function which builds it
    QString function;
  };  // end of MaterialProperty

  /*!
   * \brief the material properties shown by `MaterialPropertySelector`, as a
   * two column table: the name of the property, then its library
   */
  struct MaterialPropertyModel : public QAbstractTableModel {
    /*!
     * \brief build an empty model
     * \param[in] p : parent, in the Qt sense
     */
    MaterialPropertyModel(QObject *const);

    /*!
     * \return the number of stored material properties; the parent index is
     * ignored, the model has a single level
     */
    int rowCount(const QModelIndex &) const override;

    /*!
     * \return 2, the number of columns: the name of the property and its
     * library; the parent index is ignored
     */
    int columnCount(const QModelIndex &) const override;

    /*!
     * \param[in] i : index of the cell
     * \param[in] r : role requested by the view
     * \return the name or the library of the property of the row, an invalid
     * value for any other role, for an invalid index or for a column greater
     * than one
     */
    QVariant data(const QModelIndex &, int) const override;

    /*!
     * \param[in] section : index of the header section
     * \param[in] orientation : horizontal or vertical direction
     * \param[in] role : role requested by the view
     * \return the "Name" and "Library" titles of the horizontal header, an
     * invalid value otherwise: the vertical header is left to the view, which
     * hides it
     */
    QVariant headerData(int section, Qt::Orientation, int) const override;

    /*!
     * \brief sort the properties along the given column and reset the model so
     * that the view follows
     * \param[in] c : column to sort along; any other value leaves the model
     * untouched but still resets it
     * \param[in] o : order asked by the header view
     *
     * \note the comparators are used the other way around: an ascending sort
     * arranges the values in decreasing order and the other way around
     *
     * \note a model reset discards the selection made in the view
     */
    void sort(int, Qt::SortOrder) override;

    /*!
     * \brief replace all the material properties of the model and reset it,
     * which discards the selection made in the view
     * \param[in] m : properties to show, in the order they are displayed
     */
    virtual void setMaterialProperties(const QVector<MaterialProperty> &);

    /*!
     * \return the material properties currently shown, in the display order;
     * the result is never null but it may be empty and belongs to the model
     */
    const QVector<MaterialProperty> &materialProperties() const;

   protected:
    //! strict ordering on the name of the property, used by `sort`
    static bool sort1(const MaterialProperty &, const MaterialProperty &);

    //! reversed ordering on the name of the property, used by `sort`
    static bool sort2(const MaterialProperty &, const MaterialProperty &);

    //! strict ordering on the library, used by `sort`
    static bool sort3(const MaterialProperty &, const MaterialProperty &);

    //! reversed ordering on the library, used by `sort`
    static bool sort4(const MaterialProperty &, const MaterialProperty &);

    //! material properties shown by the model
    QVector<MaterialProperty> m;

  };  // end of struct MaterialPropertyModel

  /*!
   * \brief widget which lists the material properties of a library and lets
   * the user select some of them, as the dialog opened by the "Add material
   * properties" action of the licos mode does
   *
   * The list is filled by the `mfm-laws` command: the line edit holds either a
   * regular expression which selects the material properties of
   * `MFrontMaterials` or the path of a dynamic library.
   */
  struct MaterialPropertySelector : public QWidget {
    /*!
     * \brief build the selector and fill it with the material properties
     * matching the regular expression ".\\+", that is all those of
     * `MFrontMaterials`
     * \param[in] q : editor the line edit of the filter is associated with; it
     * is used for this line edit only
     * \param[in] p : parent, in the Qt sense
     */
    MaterialPropertySelector(EditorWidget &, QWidget *const = nullptr);

   signals:

    /*!
     * \brief emitted when the user pushes the "Add" button
     * \param[in] m : material properties selected in the table; the order of
     * the rows is the one of the selection model, not the display order, and
     * the list is empty when no row is selected
     *
     * \note `LicosInsertMaterialPropertyDialog` inserts a `MaterialProperty`
     * line for each property at the cursor of the text edit
     */
    void materialPropertiesSelected(const QVector<MaterialProperty> &);

   protected slots:

    /*!
     * \brief refresh the model and the title above the table; connected to the
     * `textChanged` signal of the filter line edit and called once by the
     * constructor
     * \param[in] m : filter: either the path of a library, recognised by its
     * `.so` suffix, or a regular expression selecting the properties of
     * `MFrontMaterials`
     *
     * \note the columns of the table are resized to fit the new contents
     */
    virtual void updateMFMModel(const QString &);

    /*!
     * \brief ask for a library through a file dialog restricted to `.so` files
     * and put its path in the filter line edit, which refreshes the table;
     * connected to the `pressed` signal of the open button
     *
     * \note a cancelled dialog returns an empty name and leaves the filter
     * unchanged
     */
    virtual void openLibrary();

    /*!
     * \brief emit `materialPropertiesSelected` with the rows selected in the
     * table; connected to the `pressed` signal of the "Add" button
     */
    virtual void addMaterialProperties();

   protected:
    /*!
     * \brief call `mfm-laws` and turn its output into material properties
     * \param[in] r : filter, either a regular expression or the path of a
     * library
     * \return the properties whose line matches "name (library)"; the result
     * is empty when the command cannot be started, does not end in time or
     * says nothing about the filter
     *
     * \note the command is run with blocking waits, so the calling thread, in
     * practice the GUI thread, waits for it
     */
    virtual QVector<MaterialProperty> callMFMLaws(const QString &) const;

   private:
    //! model of the table, a child of this widget
    MaterialPropertyModel *model;
    //! table which shows the model and holds the selection
    QTableView *view;
    //! title above the table, naming the library the properties come from
    QLabel *l;
    //! line edit of the filter
    LineEdit *lf;

    Q_OBJECT

  };  // end of struct MaterialPropertySelector

}  // end of namespace tfel::gui

#endif /* LIB_MATERIALPROPERTYSELECTOR_H */
