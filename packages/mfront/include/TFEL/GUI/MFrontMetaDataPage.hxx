/*!
 * \file   MFrontMetaDataPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTMETADATAPAGE_HXX
#define LIB_TFEL_GUI_MFRONTMETADATAPAGE_HXX

#include <QtWidgets/QWizard>

// forward declaration
class QDateEdit;

namespace tfel {

  namespace gui {

    // forward declaration
    struct EditorWidget;
    struct TextEditBase;
    struct LineEdit;

    struct MFrontMetaDataPage : public QWizardPage {
      //! list of supported material knowledge
      enum MaterialKnowledgeType {
        MATERIALPROPERTY,
        BEHAVIOUR,
        MODEL
      };  // end of enum MaterialKnowledgeType
      /*!
       * \brief constructor
       * \param[in] w: editor widget
       * \param[in] cd: current document
       * \param[in] t: knowledge type
       * \param[in] n: next page
       * \param[in] p: parent
       */
      MFrontMetaDataPage(EditorWidget &,
                         TextEditBase &,
                         const MaterialKnowledgeType,
                         const int,
                         QWizard *const);

      bool validatePage() override;

      int nextId() const override;
      //! \brief write the output of the wizard
      virtual void write() const;
      //! destructor
      ~MFrontMetaDataPage() override;

     protected:
      //!\brief document
      TextEditBase &d;
      //! \brief knowledge type
      const MaterialKnowledgeType mt;
      //! \brief name
      LineEdit *ne = nullptr;
      //! \brief autho
      LineEdit *ae = nullptr;
      //! \brief date
      QDateEdit *de = nullptr;
      //! \brief next page id
      const int nextPage;

     private:
      Q_OBJECT
    };  // end of struct MFrontMetaDataPage

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTMETADATAPAGE_HXX */
