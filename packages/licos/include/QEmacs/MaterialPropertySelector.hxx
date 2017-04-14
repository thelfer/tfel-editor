/*! 
 * \file  MaterialPropertySelector.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 14 déc. 2012
 */

#ifndef _LIB_MATERIALPROPERTYSELECTOR_H_
#define _LIB_MATERIALPROPERTYSELECTOR_H_ 

#include<QtCore/QVector>
#include<QtCore/QAbstractItemModel>

#ifdef QEMACS_QT4
#include<QtGui/QLabel>
#include<QtGui/QWidget>
#include<QtGui/QTableView>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QLabel>
#include<QtWidgets/QWidget>
#include<QtWidgets/QTableView>
#endif /* QEMACS_QT5 */
#include<QtGui/QTextBlock>

namespace qemacs
{

  struct MaterialProperty
  {
    QString library;
    QString function;
  }; // end of MaterialProperty

  struct MaterialPropertyModel
    : public QAbstractTableModel
  {

    MaterialPropertyModel(QObject *const);

    virtual int
    rowCount(const QModelIndex&) const;

    virtual int
    columnCount(const QModelIndex&) const;

    QVariant data(const QModelIndex&,
		  int) const;
    
    QVariant headerData(int section,
			Qt::Orientation,
			int) const;
  
    void sort(int, Qt::SortOrder);

    virtual void
    setMaterialProperties(const QVector<MaterialProperty>&);

    const QVector<MaterialProperty>&
    materialProperties(void) const;

  protected:

    static bool
    sort1(const MaterialProperty&,
	  const MaterialProperty&);

    static bool
    sort2(const MaterialProperty&,
	  const MaterialProperty&);

    static bool
    sort3(const MaterialProperty&,
	  const MaterialProperty&);

    static bool
    sort4(const MaterialProperty&,
	  const MaterialProperty&);
    
    QVector<MaterialProperty> m;

  }; // end of struct MaterialPropertyModel

  class MaterialPropertySelector
    : public QWidget
  {

    Q_OBJECT
    
  public:
    
    MaterialPropertySelector(QWidget * const = nullptr);

  signals:

    void
    materialPropertiesSelected(const QVector<MaterialProperty>&);

  protected slots:

    virtual void
    updateMFMModel(const QString&);

    virtual void
    openLibrary(void);

    virtual void
    addMaterialProperties();

  protected:

    virtual QVector<MaterialProperty>
    callMFMLaws(const QString&) const;

  private:

    MaterialPropertyModel *model;
    QTableView *view;
    QLabel    *l;
    QLineEdit *lf;

  }; // end of class MaterialPropertySelector

} // end of namespace qemacs

#endif /* _LIB_MATERIALPROPERTYSELECTOR_H */

