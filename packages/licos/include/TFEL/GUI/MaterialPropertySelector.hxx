/*!
 * \file  MaterialPropertySelector.hxx
 * \brief
 * \author Helfer Thomas
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

namespace tfel{

  namespace gui{

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct LineEdit;

  struct MaterialProperty {
    QString library;
    QString function;
  };  // end of MaterialProperty

  struct MaterialPropertyModel : public QAbstractTableModel {
    MaterialPropertyModel(QObject *const);

    int rowCount(const QModelIndex &) const override;

    int columnCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &, int) const override;

    QVariant headerData(int section,
                        Qt::Orientation,
                        int) const override;

    void sort(int, Qt::SortOrder) override;

    virtual void setMaterialProperties(
        const QVector<MaterialProperty> &);

    const QVector<MaterialProperty> &materialProperties() const;

   protected:
    static bool sort1(const MaterialProperty &,
                      const MaterialProperty &);

    static bool sort2(const MaterialProperty &,
                      const MaterialProperty &);

    static bool sort3(const MaterialProperty &,
                      const MaterialProperty &);

    static bool sort4(const MaterialProperty &,
                      const MaterialProperty &);

    QVector<MaterialProperty> m;

  };  // end of struct MaterialPropertyModel

  struct MaterialPropertySelector : public QWidget {
    MaterialPropertySelector(EditorWidget &, QWidget *const = nullptr);

   signals:

    void materialPropertiesSelected(const QVector<MaterialProperty> &);

   protected slots:

    virtual void updateMFMModel(const QString &);

    virtual void openLibrary();

    virtual void addMaterialProperties();

   protected:
    virtual QVector<MaterialProperty> callMFMLaws(
        const QString &) const;

   private:
    MaterialPropertyModel *model;
    QTableView *view;
    QLabel *l;
    LineEdit *lf;

    Q_OBJECT

  };  // end of struct MaterialPropertySelector

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_MATERIALPROPERTYSELECTOR_H */
