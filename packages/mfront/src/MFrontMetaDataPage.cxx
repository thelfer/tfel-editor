/*!
 * \file   MFrontMetaDataPage.cxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#include <QtGui/QValidator>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGridLayout>
#include "MFront/MaterialPropertyDSL.hxx"
#include "MFront/BehaviourDSLCommon.hxx"
#include "MFront/ModelDSLCommon.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontMetaDataPage.hxx"

namespace tfel::gui {

  struct MaterialPropertyNameValidator : public QValidator {
    QValidator::State validate(QString &n, int &) const override {
      return mfront::isValidMaterialPropertyName(n.toStdString())
                 ? QValidator::Acceptable
                 : QValidator::Invalid;
    }  // end of validate
  };   // end of struct MaterialPropertyNameValidator

  struct BehaviourNameValidator : public QValidator {
    QValidator::State validate(QString &n, int &) const override {
      return mfront::isValidBehaviourName(n.toStdString())
                 ? QValidator::Acceptable
                 : QValidator::Invalid;
    }  // end of validate
  };   // end of struct BehaviourNameValidator

  struct ModelNameValidator : public QValidator {
    QValidator::State validate(QString &n, int &) const override {
      return mfront::isValidModelName(n.toStdString()) ? QValidator::Acceptable
                                                       : QValidator::Invalid;
    }  // end of validate
  };   // end of struct ModelNameValidator

  MFrontMetaDataPage::MFrontMetaDataPage(EditorWidget &w,
                                         TextEditBase &cd,
                                         const MaterialKnowledgeType kt,
                                         const int n,
                                         QWizard *const p)
      : QWizardPage(p), d(cd), mt(kt), nextPage(n) {
    // main grid layout
    auto *const mgl = new QGridLayout;
    QLabel *nl = nullptr;
    this->ne = new LineEdit(w);
    if (this->mt == MATERIALPROPERTY) {
      this->setTitle(QObject::tr("Material property' metadata"));
      this->setSubTitle(
          QObject::tr("Specify some meta data describing the "
                      "material property to be created"));
      nl = new QLabel(QObject::tr("MaterialProperty"));
      nl->setToolTip(
          "This is the name of the material property.\n"
          " This name is used to determine the name of the"
          " generated entry point");
      this->ne->setValidator(new MaterialPropertyNameValidator());
    } else if (this->mt == BEHAVIOUR) {
      this->setTitle(QObject::tr("Behaviour' metadata"));
      this->setSubTitle(
          QObject::tr("Specify some meta data describing the "
                      "behaviour to be created"));
      nl = new QLabel(QObject::tr("Behaviour"));
      nl->setToolTip(
          "This is the name of the material property.\n"
          " This name is used to determine the name of the"
          " generated entry point");
      this->ne->setValidator(new BehaviourNameValidator());
    } else {
      this->setTitle(QObject::tr("Model' metadata"));
      this->setSubTitle(
          QObject::tr("Specify some meta data describing the "
                      "model to be created"));
      nl = new QLabel(QObject::tr("Model"));
      nl->setToolTip(
          "This is the name of the model.\n"
          " This name is used to determine the name of the"
          " generated entry point");
      this->ne->setValidator(new ModelNameValidator());
    }
    //
    auto *const ml = new QLabel(QObject::tr("Material"));
    ml->setToolTip("Material name");
    this->me = new LineEdit(w);
    //
    auto *const anl = new QLabel(QObject::tr("Author name"));
    anl->setToolTip("This is the name of the author of the implementation");
    this->ae = new LineEdit(w);
#ifdef Q_OS_UNIX
    this->ae->setText(getUserName());
#endif
    //
    auto *const dl = new QLabel(QObject::tr("Date"));
    this->de = new QDateEdit(QDate::currentDate());
    //     auto *const de = new QDateEdit(QDate::currentDate());
    mgl->addWidget(nl, 0, 0);
    mgl->addWidget(this->ne, 0, 1);
    nl->setBuddy(this->ne);
    mgl->addWidget(ml, 1, 0);
    mgl->addWidget(this->me, 1, 1);
    anl->setBuddy(this->me);
    mgl->addWidget(anl, 2, 0);
    mgl->addWidget(this->ae, 2, 1);
    anl->setBuddy(this->ae);
    mgl->addWidget(dl, 3, 0);
    mgl->addWidget(this->de, 3, 1);
    dl->setBuddy(this->de);
    this->setLayout(mgl);
  }  // end of MFrontMetaDataPage

  bool MFrontMetaDataPage::validatePage() { return true; }

  int MFrontMetaDataPage::nextId() const { return this->nextPage; }

  void MFrontMetaDataPage::write() const {
    auto number = [](const int v) {
      if (v < 10) {
        return "0" + QString::number(v);
      }
      return QString::number(v);
    };
    this->d.insertPlainText("@Behaviour " + this->ne->text() + ";\n");
    if (const auto material = this->me->text(); !material.isEmpty()) {
      this->d.insertPlainText("@Material " + material + ";\n");
    }
    if (const auto author = this->ae->text(); !author.isEmpty()) {
      this->d.insertPlainText("@Author " + author + ";\n");
    }
    const auto date = this->de->date();
    this->d.insertPlainText("@Date " + number(date.day()) + '/' +
                            number(date.month()) + '/' +
                            QString::number(date.year()) + ";\n");
    this->d.insertPlainText("@Description {\n");
    this->d.insertPlainText("}\n\n");
  }  // end of write

  MFrontMetaDataPage::~MFrontMetaDataPage() = default;

}  // end of namespace tfel::gui
