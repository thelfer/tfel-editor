/*!
 * \file   MFrontAddVariableDialog.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialogButtonBox>
#include "TFEL/UnicodeSupport/UnicodeSupport.hxx"
#include "TFEL/Glossary/Glossary.hxx"
#include "MFront/SupportedTypes.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/MFrontAddVariableDialog.hxx"

namespace tfel{

  namespace gui {

    MFrontAddVariableDialog::MFrontAddVariableDialog(
        EditorWidget& w, const VariableType t, QWidget* const p)
        : QDialog(p),
          vtype(t),
          ne(new LineEdit(w)),
          cb(new QComboBox),
          tb(new QComboBox),
          gb(new QComboBox),
          ee(new LineEdit(w)) {
      if (this->vtype == MATERIALPROPERTY) {
        this->setWindowTitle("Add a new material property");
      } else if (this->vtype == PARAMETER) {
        this->setWindowTitle("Add a new parameter");
      } else if (this->vtype == LOCALVARIABLE) {
        this->setWindowTitle("Add a new local variable");
      } else if (this->vtype == STATEVARIABLE) {
        this->setWindowTitle("Add a new state variable");
      } else if (this->vtype == AUXILIARYSTATEVARIABLE) {
        this->setWindowTitle("Add a new auxiliary state variable");
      } else if (this->vtype == INTEGRATIONVARIABLE) {
        this->setWindowTitle("Add a new integration variable");
      } else {
        this->setWindowTitle("Add new external state variable");
      }
      //
      auto* const gl = new QGridLayout();
      // variable name
      auto* const nlabel = new QLabel(QObject::tr("Variable name"));
      nlabel->setBuddy(this->ne);
      gl->addWidget(nlabel, 0, 0);
      gl->addWidget(this->ne, 0, 1);
      // variable type
      auto* const clabel = new QLabel(QObject::tr("Variable category"));
      if ((this->vtype == MATERIALPROPERTY) ||
          (this->vtype == EXTERNALSTATEVARIABLE)) {
        this->cb->addItem("Scalar");
      } else {
        this->cb->addItems(QStringList() << "Scalar"
                                        << "Vector"
                                        << "Symmetric tensor"
                                        << "Unsymmetric tensor");
      }
      gl->addWidget(clabel, 1, 0);
      gl->addWidget(this->cb, 1, 1);
      QObject::connect(this->cb, static_cast<void (QComboBox::*)(int)>(
                                     &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateVariableTypesList();
                         this->updateGlossaryNamesList();
                       });
      auto* const tlabel = new QLabel(QObject::tr("Variable type"));
      tlabel->setBuddy(this->tb);
      this->updateVariableTypesList();
      QObject::connect(this->tb, static_cast<void (QComboBox::*)(int)>(
                                     &QComboBox::currentIndexChanged),
                       [this] { this->updateGlossaryNamesList(); });
      gl->addWidget(tlabel, 2, 0);
      gl->addWidget(this->tb, 2, 1);
      auto* const glabel = new QLabel(QObject::tr("Glossary name"));
      this->updateGlossaryNamesList();
      glabel->setBuddy(this->gb);
      gl->addWidget(glabel, 3, 0);
      gl->addWidget(this->gb, 3, 1);
      auto* const elabel = new QLabel(QObject::tr("Entry name"));
      elabel->setBuddy(this->ee);
      QObject::connect(this->ee,
                       static_cast<void (LineEdit::*)(const QString&)>(
                           &LineEdit::textChanged),
                       [this] { this->updateGlossaryNamesList(); });
      gl->addWidget(elabel, 4, 0);
      gl->addWidget(this->ee, 4, 1);
      /* buttons */
      auto* const lv = new QVBoxLayout;
      auto* const bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                            QDialogButtonBox::Cancel);
      QObject::connect(bb, &QDialogButtonBox::accepted, this,
                       &QDialog::accept);
      QObject::connect(bb, &QDialogButtonBox::rejected, this,
                       &QDialog::reject);
      lv->addLayout(gl);
      lv->addWidget(bb);
      this->setLayout(lv);
    }  // end of MFrontAddVariableDialog::MFrontAddVariableDialog

    mfront::VariableDescription
    MFrontAddVariableDialog::getVariableDescription() const {
      const auto sn = this->ne->text().toStdString();
      const auto vn = tfel::unicode::getMangledString(sn);
      const auto tn = this->tb->currentText().toStdString();
      auto v = (sn != vn)
                   ? mfront::VariableDescription(tn, sn, vn, 1u, 0u)
                   : mfront::VariableDescription(tn, vn, 1u, 0u);
      if (this->gb->currentText() != "None") {
        v.setGlossaryName(this->gb->currentText().toStdString());
      }
      if (!this->ee->text().isEmpty()) {
        v.setEntryName(this->ee->text().toStdString());
      }
      return v;
    }  // end of MFrontAddVariableDialog::getVariableDescription

    void MFrontAddVariableDialog::updateVariableTypesList() {
      const auto& cn = this->tb->currentText();
      this->tb->clear();
      const auto t = [this]() {
        const auto& ct = this->cb->currentText();
        if (ct == "Scalar") {
          return mfront::SupportedTypes::SCALAR;
        } else if (ct == "Vector") {
          return mfront::SupportedTypes::TVECTOR;
        } else if (ct == "Symmetric tensor") {
          return mfront::SupportedTypes::STENSOR;
        }
        return mfront::SupportedTypes::TENSOR;
      }();
      auto types = QStringList();
      for (const auto& f : mfront::SupportedTypes::getTypeFlags()) {
        if (f.second == t) {
          types.append(QString::fromStdString(f.first));
        }
      }
      this->tb->addItems(types);
      if (types.contains(cn)) {
        this->tb->setCurrentText(cn);
      }
    }  // end of MFrontAddVariableDialog::updateVariableNamesList

    void MFrontAddVariableDialog::updateGlossaryNamesList() {
      const auto& cn = this->gb->currentText();
      this->gb->clear();
      this->gb->addItem("None");
      if (!this->ee->text().isEmpty()) {
        this->gb->setDisabled(true);
      } else {
        const auto& g = tfel::glossary::Glossary::getGlossary();
        auto ges = QStringList{};
        for (const auto& k : g.getKeys()) {
          ges.append(QString::fromStdString(k));
        }
        this->gb->addItems(ges);
        if (ges.contains(cn)) {
          this->gb->setCurrentText(cn);
        }
        this->gb->setEnabled(true);
      }
    }  // end of updateGlossaryNamesList

    MFrontAddVariableDialog::~MFrontAddVariableDialog() = default;

  }  // end of namespace gui

}  // end of namespace tfel
