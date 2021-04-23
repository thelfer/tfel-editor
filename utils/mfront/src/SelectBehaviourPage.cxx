/*!
 * \file   SelectBehaviourPage.cxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFileDialog>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "MFront/TargetsDescription.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/SelectBehaviourPage.hxx"

namespace tfel {

  namespace gui {

    SelectBehaviourPage::SelectBehaviourPage(EditorWidget& q)
        : bl(new QComboBox(this)),
          mh(new QComboBox(this)),
          le(new LineEdit(q, this)),
          slb(new QPushButton(QObject::tr("Open"), this)) {
      this->setTitle("Select a behaviour");
      auto* label = new QLabel(
          "This wizard will help you "
          "to load a behaviour");
      label->setWordWrap(true);
      // library selection
      auto* libraryLabel = new QLabel(QObject::tr("&Library:"));
      libraryLabel->setToolTip(QObject::tr("The user shall select a library"));
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

      QObject::connect(this->slb, &QPushButton::pressed, this,
                       &SelectBehaviourPage::selectLibrary);
      QObject::connect(this->le, &LineEdit::textChanged, this,
                       &SelectBehaviourPage::updateBehaviourList);
      QObject::connect(this->bl, &QComboBox::currentTextChanged, this,
                       &SelectBehaviourPage::updateModellingHypotheses);
      QObject::connect(this->bl, &QComboBox::currentTextChanged, this, [this] {
        emit behaviourDescriptionChanged(this->getBehaviourDescription());
      });
      QObject::connect(this->mh, &QComboBox::currentTextChanged, this, [this] {
        emit behaviourDescriptionChanged(this->getBehaviourDescription());
      });
      this->registerField("Library*", this->le);
      this->registerField("Behaviour*", this->bl, "currentText",
                          "currentTextChanged");
      this->registerField("Hypothesis*", this->mh, "currentText",
                          "currentTextChanged");
    }

    BehaviourDescription SelectBehaviourPage::getBehaviourDescription() const {
      using tfel::system::ExternalLibraryManager;
      auto& elm = ExternalLibraryManager::getExternalLibraryManager();
      BehaviourDescription b;
      b.library = this->le->text();
      b.behaviour = this->bl->currentText();
      b.hypothesis = this->mh->currentText();
      const auto l = b.library.toStdString();
      const auto f = b.behaviour.toStdString();
      try {
        b.minterface = QString::fromStdString(elm.getInterface(l, f));
      } catch (std::exception& e) {
        return {};
      } catch (...) {
        return {};
      }
      return b;
    }

    void SelectBehaviourPage::selectLibrary() {
      const auto td = mfront::TargetsDescription();
      const QString prefix =
          mfront::LibraryDescription::getDefaultLibraryPrefix(td.system,
                                                              td.libraryType);
      const QString suffix =
          mfront::LibraryDescription::getDefaultLibrarySuffix(td.system,
                                                              td.libraryType);
      const auto& l = QFileDialog::getOpenFileName(
          this, QObject::tr("Select Library"), "",
          QObject::tr("Shared Library") + "(" + prefix + "*." + suffix + ")");
      if (l.isEmpty()) {
        return;
      }
      this->le->setText(l);
    }

    void SelectBehaviourPage::updateBehaviourList() {
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
        debug("SelectBehaviourPage::updateBehaviourList", ex.what());
        return;
      }
      for (const auto& e : epts) {
        try {
          if (elm.getMaterialKnowledgeType(lib, e) == 1u) {
            this->bl->addItem(QString::fromStdString(e));
          }
        } catch (std::exception& ex) {
          debug("SelectBehaviourPage::updateBehaviourList", ex.what());
        }
      }
      if (this->bl->count() > 0) {
        this->bl->setCurrentIndex(0);
      }
    }  // end of SelectBehaviourPage::updateBehaviourList

    void SelectBehaviourPage::updateModellingHypotheses() {
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
            "SelectBehaviourPage::"
            "updateModellingHypotheses",
            ex.what());
      }
      if (this->mh->count() > 0) {
        this->mh->setCurrentIndex(0);
      }
    }  // end of
       // SelectBehaviourPage::updateModellingHypotheses

    bool SelectBehaviourPage::validatePage() {
      const auto b = this->getBehaviourDescription();
      return b.generate() != nullptr;
    }  // end of SelectLibrary::validatePage

    int SelectBehaviourPage::nextId() const {
      auto b = this->getBehaviourDescription().generate();
      if (b) {
        if (b->getMaterialPropertiesNames().empty()) {
          return 2;
        }
        return 1;
      }
      return 2;
    }  // end of SelectBehaviourPage::nextId

    SelectBehaviourPage::~SelectBehaviourPage() = default;

  }  // end of namespace gui
}  // end of namespace tfel
