/*!
 * \file   ImportBehaviour.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   02 mai 2017
 */

#ifndef LIB_QEMACS_MTEST_IMPORTBEHAVIOUR_HXX
#define LIB_QEMACS_MTEST_IMPORTBEHAVIOUR_HXX

#include<memory>
#include<QtWidgets/QComboBox>
#include<QtWidgets/QPushButton>
#include<QtWidgets/QLineEdit>
#include<QtWidgets/QWizard>
#include "MTest/Behaviour.hxx"

namespace qemacs
{

  // forward declaration
  class QEmacsTextEditBase;
  
  class ImportBehaviour
    : public QWizard
  {

    Q_OBJECT
    
  public:
    
    ImportBehaviour(QEmacsTextEditBase&);
    //! \return the hypothesis selected
    QString getHypothesis() const;
    //! \return a Behaviour build on the information provided by the user
    std::shared_ptr<mtest::Behaviour> getBehaviour() const;
    //! destructor
    ~ImportBehaviour();
  protected:

    struct SelectHypothesisPage;
    struct SelectBehaviourPage;
    struct MaterialPropertyPage;

    SelectHypothesisPage *sh;
    SelectBehaviourPage  *sb;
    MaterialPropertyPage *mp;
    
  }; // end of struct ImportBehaviour

  class ImportBehaviour::SelectHypothesisPage
    : public QWizardPage
  {

    Q_OBJECT

  public:
    /*!
     * \brief constructor
     * \param[in] w: reference to the calling wizard;
     */
    SelectHypothesisPage(ImportBehaviour&);

  private:
    //! the calling wizard
    ImportBehaviour& wizard;
  };
  
  class ImportBehaviour::SelectBehaviourPage
    : public QWizardPage
  {

    Q_OBJECT

  public:
    
    SelectBehaviourPage(ImportBehaviour&);

  private slots:

    /*!
     * \brief update the list of behaviours
     * \param[in] l: library name
     */
    virtual void updateBehaviourList(const QString&);
    
    virtual void selectLibrary();

    bool validatePage() override;
    //! 
    int nextId() const override;

  private:
    //! list of behaviours declared in the selected library
    QComboBox *bl;
    //! enter the library location
    QLineEdit *le;
    //! select library button
    QPushButton *slb;
    //! the calling wizard
    ImportBehaviour& wizard;
  };

  class ImportBehaviour::MaterialPropertyPage
    : public QWizardPage
  {

    Q_OBJECT

  public:
    
    MaterialPropertyPage(ImportBehaviour&);

    int nextId() const override;

  private:
    //! the calling wizard
    ImportBehaviour& wizard;
  }; // end of ImportBehaviour::MaterialPropertyPage
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_MTEST_IMPORTBEHAVIOUR_HXX */