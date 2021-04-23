/*!
 * \file   MFMFilterProxyModel.hxx
 * \brief
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#ifndef LIB_TFEL_GUI_MFMFILTERPROXYMODEL_HXX
#define LIB_TFEL_GUI_MFMFILTERPROXYMODEL_HXX

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QSortFilterProxyModel>
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel {

  namespace gui {

    /*!
     * \brief A simple proxy model to filter MFrontMaterials entries.
     */
    struct TFEL_GUI_MIW_VISIBILITY_EXPORT MFMFilterProxyModel
        : public QSortFilterProxyModel {
      MFMFilterProxyModel(QObject* parent = nullptr);

     public slots:
      virtual void setNameFilter(const QString&);
      virtual void setInterfaceFilter(const QString&);
      virtual void setMaterialFilter(const QString&);
      virtual void setMaterialKnowledgeType(const QString&);

     protected:
      bool filterAcceptsRow(int, const QModelIndex&) const override;
      //! name filter
      QRegExp name;
      //! interface filter
      QRegExp mfront_interface;
      //! material filter
      QRegExp material;
      //! material knowledge
      QString mkt;

     private:
      Q_OBJECT
    };  // end of MFMFilterProxyModel

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFMFILTERPROXYMODEL_HXX */
