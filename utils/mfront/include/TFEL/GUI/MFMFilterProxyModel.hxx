/*!
 * \file   MFMFilterProxyModel.hxx
 * \brief proxy model filtering the MFM database entries.
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#ifndef LIB_TFEL_GUI_MFMFILTERPROXYMODEL_HXX
#define LIB_TFEL_GUI_MFMFILTERPROXYMODEL_HXX

#include <QtCore/QString>
#include <QtCore/QRegularExpression>
#include <QtCore/QSortFilterProxyModel>
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  /*!
   * \brief A simple proxy model to filter MFrontMaterials entries.
   *
   * It is meant to be used on a model built by \c MFMDataBase::load, whose
   * columns hold the kind of material knowledge, the name, the material, the
   * library and the interface; the other columns are ignored by the filters.
   *
   * \note the patterns are not anchored: a row is kept when the pattern matches
   * any part of the corresponding field
   *
   * \note \c data and \c headerData are not overridden: the values and the
   * labels shown for the remaining rows are those of the source model, and the
   * roles the source model does not handle stay unhandled
   *
   * \note the sorting is left to the base class, so the rows follow the column
   * the user clicks on in the view; the widgets which use this class enable the
   * dynamic sort filter, which re-filters the rows as soon as a filter or the
   * source model changes
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT MFMFilterProxyModel
      : public QSortFilterProxyModel {
    /*!
     * \brief constructor: all the filters are empty, so every row of the source
     * model is accepted
     * \param[in] parent : parent of the model
     */
    MFMFilterProxyModel(QObject* parent = nullptr);

   public slots:
    /*!
     * \brief keep the rows whose name matches the given regular expression
     * \param[in] i : pattern of the filter
     *
     * \note an invalid pattern, as the empty text typed before the first
     * character, disables this filter; the rows are re-filtered
     */
    virtual void setNameFilter(const QString&);
    /*!
     * \brief keep the rows whose interface matches the given regular expression
     * \param[in] i : pattern of the filter
     *
     * \note an invalid pattern disables this filter; the rows are re-filtered
     */
    virtual void setInterfaceFilter(const QString&);
    /*!
     * \brief keep the rows whose material matches the given regular expression
     * \param[in] m : pattern of the filter
     *
     * \note an invalid pattern disables this filter; the rows are re-filtered
     */
    virtual void setMaterialFilter(const QString&);
    /*!
     * \brief keep the rows of the given kind of material knowledge
     * \param[in] t : \c MaterialProperty, \c Behaviour or \c Model
     *
     * \note any other value is ignored, which keeps the previous filter; an
     * empty text is not one of the three values, so it can not restore a model
     * where all the kinds are shown
     */
    virtual void setMaterialKnowledgeType(const QString&);

   protected:
    /*!
     * \return true if the row \c r of the source model, whose parent is \c p,
     * satisfies all the filters which are set: the kind of material knowledge
     * is compared to \c mkt, the other filters are regular expressions matched
     * against the name, the interface and the material of the row
     *
     * \note the filters are combined with a logical and, and the row is
     * rejected as soon as one of them fails
     */
    bool filterAcceptsRow(int, const QModelIndex&) const override;
    //! name filter, matched against the second column of the source model
    QRegularExpression name;
    //! interface filter, matched against the fifth column of the source model
    QRegularExpression mfront_interface;
    //! material filter, matched against the third column of the source model
    QRegularExpression material;
    /*!
     * \brief kind of material knowledge the rows are restricted to, empty to
     * keep all of them
     */
    QString mkt;

   private:
    Q_OBJECT
  };  // end of MFMFilterProxyModel

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFMFILTERPROXYMODEL_HXX */
