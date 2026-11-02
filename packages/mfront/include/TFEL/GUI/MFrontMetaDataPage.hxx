/*!
 * \file   MFrontMetaDataPage.hxx
 * \brief wizard page collecting the name, material, author and date.
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTMETADATAPAGE_HXX
#define LIB_TFEL_GUI_MFRONTMETADATAPAGE_HXX

#include <QtWidgets/QWizard>

// forward declaration
class QDateEdit;

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  struct TextEditBase;
  struct LineEdit;

  /*!
   * \brief a wizard page which collects the metadata of the material knowledge
   * to be created: its name, the material it belongs to, its author and the
   * date of the creation
   */
  struct MFrontMetaDataPage : public QWizardPage {
    //! list of supported material knowledge
    enum MaterialKnowledgeType {
      MATERIALPROPERTY,  //!< a material property
      BEHAVIOUR,         //!< a behaviour
      MODEL              //!< a model
    };                   // end of enum MaterialKnowledgeType
    /*!
     * \brief constructor
     * \param[in] w: editor widget, used by the line edits of the page
     * \param[in] cd: current document, which the metadata are written in
     * \param[in] t: knowledge type. It selects the title of the page, the label
     * of the name and the validator which checks that the name is a valid
     * MFront entry point name
     * \param[in] n: identifier of the page the wizard goes to when this one is
     * accepted
     * \param[in] p: parent, that is the wizard the page belongs to
     *
     * \note the author line edit is pre-filled with the name of the current
     * user under Unix and the date edit with the current date
     *
     * \note the four fields are created here and are reparented to the page by
     * its layout, so the page owns them
     *
     * \note only the \c BEHAVIOUR type is used by the editor today
     */
    MFrontMetaDataPage(EditorWidget &,
                       TextEditBase &,
                       const MaterialKnowledgeType,
                       const int,
                       QWizard *const);

    /*!
     * \return always true: an invalid name is not caught here, only the
     * validator of the line edit restricts what the user can type
     */
    bool validatePage() override;

    //! \return the identifier given to the constructor as next page
    int nextId() const override;
    /*!
     * \brief write the output of the wizard
     *
     * The name is always written as a \c @Behaviour declaration, whatever the
     * kind of knowledge the page was created for. The material and the author
     * declarations are only written when the corresponding fields are not empty
     * and the date is written as \c dd/mm/yyyy, followed by an empty
     * \c @Description block.
     *
     * \note the text is inserted at the cursor of the document, which is left
     * at the end of it
     */
    virtual void write() const;
    /*!
     * \brief destructor
     *
     * The widgets of the page are children of the page and are released by Qt
     */
    ~MFrontMetaDataPage() override;

   protected:
    //!\brief document
    TextEditBase &d;
    //! \brief knowledge type
    const MaterialKnowledgeType mt;
    //! \brief name
    LineEdit *ne = nullptr;
    //! \brief material
    LineEdit *me = nullptr;
    //! \brief author
    LineEdit *ae = nullptr;
    //! \brief date
    QDateEdit *de = nullptr;
    //! \brief next page id
    const int nextPage;

   private:
    Q_OBJECT
  };  // end of struct MFrontMetaDataPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTMETADATAPAGE_HXX */
