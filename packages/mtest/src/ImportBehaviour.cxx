/*!
 * \file   ImportBehaviour.cxx
 * \brief
 * \author Thomas Helfer
 * \date   02/06/2017
 */

#include <QtCore/QStringListModel>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QVBoxLayout>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/Material/ModellingHypothesis.hxx"
#include "MFront/TargetsDescription.hxx"
#include "QEmacs/Debug.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/MaterialPropertySelectorWidget.hxx"
#include "QEmacs/ImportBehaviour.hxx"

namespace qemacs {

  ImportBehaviour::SelectBehaviourPage::SelectBehaviourPage(
      ImportBehaviour& w)
      : bl(new QComboBox(this)),
        mh(new QComboBox(this)),
        le(new QLineEdit(this)),
        slb(new QPushButton(QObject::tr("Open"), this)),
        wizard(w) {
    this->setTitle("Select a behaviour");
    auto* label = new QLabel(
        "This wizard will help you "
        "to load a behaviour");
    label->setWordWrap(true);
    // library selection
    auto* libraryLabel = new QLabel(QObject::tr("&Library:"));
    libraryLabel->setToolTip(
        QObject::tr("The user shall select a library"));
    libraryLabel->setBuddy(this->le);
    this->bl->setSizePolicy(QSizePolicy::MinimumExpanding,
                            QSizePolicy::Preferred);
    auto* blabel = new QLabel(QObject::tr("&Behaviour:"));
    blabel->setBuddy(this->bl);
    blabel->setToolTip(
        QObject::tr("The user shall shall select a behaviour "
                    "in the selected library."));

    auto* lh = new QLabel(QObject::tr("Modelling hypothesis"));
    this->mh->setSizePolicy(
        QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
    // layouts definition
    auto* l = new QVBoxLayout;
    auto* g = new QGridLayout;
    // library selection
    l->addWidget(label);
    g->addWidget(libraryLabel, 0, 0);
    g->addWidget(this->le, 0, 1);
    g->addWidget(this->slb, 0, 2);
    // behaviour selection
    g->addWidget(blabel, 1, 0);
    g->addWidget(this->bl, 1, 1, 1, 2);
    // modelling hypothesis selection
    g->addWidget(lh, 2, 0);
    g->addWidget(this->mh, 2, 1, 1, 2);

    // setting the main layout
    l->addLayout(g);
    this->setLayout(l);

    QObject::connect(
        this->slb, &QPushButton::pressed, this,
        &ImportBehaviour::SelectBehaviourPage::selectLibrary);
    QObject::connect(
        this->le, &QLineEdit::textChanged, this,
        &ImportBehaviour::SelectBehaviourPage::updateBehaviourList);
    QObject::connect(this->bl, &QComboBox::currentTextChanged, this,
                     &ImportBehaviour::SelectBehaviourPage::
                         updateModellingHypotheses);
    QObject::connect(this->bl, &QComboBox::currentTextChanged, this,
                     [this] { emit behaviourDescriptionChanged(); });
    QObject::connect(this->mh, &QComboBox::currentTextChanged, this,
                     [this] { emit behaviourDescriptionChanged(); });
    this->registerField("Library*", this->le);
    this->registerField("Behaviour*", this->bl, "currentText",
                        "currentTextChanged");
    this->registerField("Hypothesis*", this->mh, "currentText",
                        "currentTextChanged");
  }

  void ImportBehaviour::SelectBehaviourPage::selectLibrary() {
    const auto td = mfront::TargetsDescription();
    const QString prefix =
        mfront::LibraryDescription::getDefaultLibraryPrefix(
            td.system, td.libraryType);
    const QString suffix =
        mfront::LibraryDescription::getDefaultLibrarySuffix(
            td.system, td.libraryType);
    const auto& l = QFileDialog::getOpenFileName(
        this, QObject::tr("Select Library"), "",
        QObject::tr("Shared Library")+ "("+prefix+"*." + suffix + ")");
    if (l.isEmpty()) {
      return;
    }
    this->le->setText(l);
  }

  void ImportBehaviour::SelectBehaviourPage::updateBehaviourList() {
    const auto l = this->field("Library").toString();
    if (l.isEmpty()) {
      return;
    }
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    const auto lib = l.toStdString();
    auto b = QStringList{};
    auto epts = std::vector<std::string>{};
    // clearing the behaviour list
    this->bl->clear();
    this->bl->clear();
    try {
      epts = elm.getEntryPoints(lib);
    } catch (std::exception& ex) {
      qDebug() << ex.what();
      return;
    }
    for (const auto& e : epts) {
      try {
        if (elm.getMaterialKnowledgeType(lib, e) == 1u) {
          this->bl->addItem(QString::fromStdString(e));
        }
      } catch (std::exception& ex) {
        debug(
            "ImportBehaviour::SelectBehaviourPage::updateBehaviourList",
            ex.what());
      }
    }
    if (this->bl->count() > 0) {
      this->bl->setCurrentIndex(0);
    }
  }  // end of ImportBehaviour::SelectBehaviourPage::updateBehaviourList

  void
  ImportBehaviour::SelectBehaviourPage::updateModellingHypotheses() {
    this->mh->clear();
    const auto l = this->field("Library").toString().toStdString();
    const auto b = this->field("Behaviour").toString().toStdString();
    if ((l.empty()) || (b.empty())) {
      return;
    }
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    try {
      for (const auto& h : elm.getSupportedModellingHypotheses(l, b)) {
        this->mh->addItem(QString::fromStdString(h));
      }
    } catch (std::exception& ex) {
      debug(
          "ImportBehaviour::SelectBehaviourPage::"
          "updateModellingHypotheses",
          ex.what());
    }
    if (this->mh->count() > 0) {
      this->mh->setCurrentIndex(0);
    }
  }  // end of
     // ImportBehaviour::SelectBehaviourPage::updateModellingHypotheses

  bool ImportBehaviour::SelectBehaviourPage::validatePage() {
    const auto b = this->wizard.getBehaviourDescription();
    return b.generate() != nullptr;
  }  // end of ImportBehaviour::SelectLibrary::validatePage

  int ImportBehaviour::SelectBehaviourPage::nextId() const {
    auto b = this->wizard.getBehaviourDescription().generate();
    if (b) {
      if(b->getMaterialPropertiesNames().empty()){
        return 2;
      }
      return 1;
    }
    return 2;
  } // end of ImportBehaviour::SelectBehaviourPage::nextId

  ImportBehaviour::MaterialPropertyPage::MaterialPropertyPage(
      ImportBehaviour& w)
      : wizard(w) {
  }  // end of
  // ImportBehaviour::MaterialPropertyPage::MaterialPropertyPage

  void ImportBehaviour::MaterialPropertyPage::
      updateMaterialPropertiesList() {
    auto b = this->wizard.getBehaviourDescription().generate();
    if (b == nullptr) {
      return;
    }
    auto* const vl = new QVBoxLayout;
    for (const auto& mp : b->getMaterialPropertiesNames()) {
      vl->addWidget(new MaterialPropertySelectorWidget(
          QString::fromStdString(mp)));
    }
    this->setLayout(vl);
  }  // end of
  // ImportBehaviour::MaterialPropertyPage::updateMaterialPropertiesList

  int ImportBehaviour::MaterialPropertyPage::nextId() const {
    return 2;
  } // end of ImportBehaviour::MaterialPropertyPage::nextId

  ImportBehaviour::ConclusionPage::ConclusionPage(ImportBehaviour& w)
      : wizard(w) {}  // end of
  // ImportBehaviour::ConclusionPage::ConclusionPage

  int ImportBehaviour::ConclusionPage::nextId() const {
    return -1;
  }

  ImportBehaviour::ImportBehaviour(QEmacsTextEditBase& t)
      : QWizard(&t),
        sb(new SelectBehaviourPage(*this)),
        mp(new MaterialPropertyPage(*this)),
        c(new ConclusionPage(*this)) {
    this->setWindowTitle(QObject::tr("Import Behaviour"));
    this->setPage(0, this->sb);
    this->setPage(1, this->mp);
    this->setPage(2, this->c);
    QObject::connect(
        this->sb, &SelectBehaviourPage::behaviourDescriptionChanged,
        this->mp, &MaterialPropertyPage::updateMaterialPropertiesList);
    this->setStartId(0);
  }  // end of ImportBehaviour::ImportBehaviour

  BehaviourDescription
  ImportBehaviour::getBehaviourDescription() const {
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    BehaviourDescription b;
    b.library    = this->field("Library").toString();
    b.behaviour  = this->field("Behaviour").toString();
    b.hypothesis = this->field("Hypothesis").toString();
    const auto l = b.library.toStdString();
    const auto f = b.behaviour.toStdString();
    try {
      b.minterface = QString::fromStdString(elm.getInterface(l, f));
    } catch (std::exception&) {
      return {};
    } catch (...) {
      return {};
    }
    return b;
  }  // end of ImportBehaviour::getBehaviour

  ImportBehaviour::~ImportBehaviour() = default;

}  // end of namespace qemacs
