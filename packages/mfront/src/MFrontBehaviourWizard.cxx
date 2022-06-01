/*!
 * \file   MFrontBehaviourWizard.cxx
 * \brief
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#include <cstring>
#include <QtCore/QDebug>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGridLayout>
#include "MFront/AbstractBehaviourDSL.hxx"
#include "MFront/BehaviourBrickFactory.hxx"
#include "MFront/BehaviourBrick/OptionDescription.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontBehaviourWizardPages.hxx"
#include "TFEL/GUI/MFrontMetaDataPage.hxx"
#include "TFEL/GUI/MFrontBehaviourPage.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"
#include "TFEL/GUI/MFrontStandardElastoViscoPlasticityBrickPage.hxx"
#include "TFEL/GUI/MFrontTemplateGenerationOptionsPage.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"

namespace tfel::gui {

  MFrontBehaviourWizard::MFrontBehaviourWizard(EditorWidget& w,
                                               TextEditBase& cd,
                                               QWidget* const p)
      : QWizard(p),
        md(new MFrontMetaDataPage(
            w, cd, MFrontMetaDataPage::BEHAVIOUR, 1, this)),
        im(new MFrontBehaviourPage(w, cd, this)),
        variables(new MFrontAddVariablesPage(w, cd, this)),
        sevpbp(new MFrontStandardElastoViscoPlasticityBrickPage(w, cd, this)),
        tgop(new MFrontTemplateGenerationOptionsPage(this)),
        editor(w),
        d(cd) {
    this->setWindowTitle(QObject::tr("Behaviour wizard"));
    this->setPage(MFrontBehaviourWizardPages::METADATA, this->md);
    this->setPage(MFrontBehaviourWizardPages::BEHAVIOUR, this->im);
    this->setPage(MFrontBehaviourWizardPages::STANDARDELASTOVISCOPLASTICITY,
                  this->sevpbp);
    this->setPage(MFrontBehaviourWizardPages::ADDVARIABLES, this->variables);
    this->setPage(MFrontBehaviourWizardPages::TEMPLATEGENERATIONOPTIONS,
                  this->tgop);
    this->setStartId(0);
  }  // end of MFrontBehaviourWizard::MFrontBehaviourWizard

  std::shared_ptr<mfront::AbstractBehaviourDSL>
  MFrontBehaviourWizard::getBehaviourDSL(const DSLGenerationOptions& o) const {
    return this->im->getBehaviourDSL(o);
  }  // end of MFrontBehaviourWizard::getBehaviourDSL

  mfront::BehaviourDSLDescription
  MFrontBehaviourWizard::getBehaviourDSLDescription() const {
    const auto dsl = this->im->getBehaviourDSL({});
    auto dsldd = dsl->getBehaviourDSLDescription();
    try {
      const auto b = this->im->getSelectedBrick();
      if (b.isEmpty()) {
        return dsldd;
      }
      const auto& bbf = mfront::BehaviourBrickFactory::getFactory();
      auto bd = dsl->getBehaviourDescription();
      const auto bb = bbf.get(b.toStdString(), *dsl, bd);
      const auto bbd = bb->getDescription();
      mfront::complete(dsldd, bbd);
      return dsldd;
    } catch (...) {
    }
    return dsldd;
  }

  void MFrontBehaviourWizard::write() const {
    constexpr const auto h =
        tfel::material::ModellingHypothesis::UNDEFINEDHYPOTHESIS;
    try {
      const auto& n = this->im->getSelectedDomainSpecificLanguage();
      const auto dsl = this->im->getBehaviourDSL({});
      const auto dsld = this->getBehaviourDSLDescription();
      const auto o = this->tgop->getTemplateGenerationOptions();
      if (dsl == nullptr) {
        return;
      }
      this->variables->complete(*dsl);
      try {
        dsl->endsInputFileProcessing();
      } catch (std::runtime_error& e) {
        this->editor.displayInformativeMessage(
            "MFrontBehaviourWizard::write: " + QString(e.what()));
      } catch (...) {
        this->editor.displayInformativeMessage(
            "MFrontBehaviourWizard::write: unknown exception");
      }
      const auto bd = dsl->getBehaviourDescription();
      this->d.insertPlainText("@DSL " + n + ";\n\n");
      this->md->write();
      this->im->write();
      this->variables->write();
      // local variables
      const auto lvs = this->variables->getLocalVariables();
      if (!lvs.empty()) {
        this->d.insertPlainText("@InitLocalVariables{\n");
        for (const auto& lv : lvs) {
          const auto vn = QString::fromStdString(mfront::displayName(lv));
          this->d.insertPlainText(vn + " = ;\n");
        }
        this->d.insertPlainText("}\n\n");
      }
      const auto cbs = dsld.typicalCodeBlocks;
      for (const auto& c : cbs) {
        if ((!bd.hasCode(h, c)) || (c == mfront::BehaviourData::Integrator) ||
            (c == mfront::BehaviourData::ComputeDerivative)) {
          auto t = QString::fromStdString(dsl->getCodeBlockTemplate(c, o));
          if (t.startsWith("@TangentOperator")) {
            continue;
          }
          if (t.startsWith("@PredictionOperator")) {
            const auto to = this->im->getSelectedTangentOperator();
            if (!to.isEmpty()) {
              t.insert(std::strlen("@PredictionOperator"), "<" + to + ">");
            }
          }
          if (!t.isEmpty()) {
            this->d.insertPlainText(t + "\n");
          }
        }
      }

      // @UpdateAuxiliaryStateVariables
      const auto asvs = this->variables->getAuxiliaryStateVariables();
      if (!asvs.empty()) {
        this->d.insertPlainText("@UpdateAuxiliaryStateVariables{\n");
        for (const auto& asv : asvs) {
          const auto vn = QString::fromStdString(mfront::displayName(asv));
          this->d.insertPlainText(vn + " = ;\n");
        }
        this->d.insertPlainText("}\n\n");
      }

      for (const auto& c : cbs) {
        if (!bd.hasCode(h, c)) {
          auto t = QString::fromStdString(dsl->getCodeBlockTemplate(c, o));
          if (!t.startsWith("@TangentOperator")) {
            continue;
          }
          const auto to = this->im->getSelectedTangentOperator();
          if (!to.isEmpty()) {
            t.insert(std::strlen("@TangentOperator"), "<" + to + ">");
          }
          if (!t.isEmpty()) {
            this->d.insertPlainText(t + "\n");
          }
        }
      }
    } catch (std::runtime_error& e) {
      this->editor.displayInformativeMessage("MFrontBehaviourWizard::write: " +
                                             QString(e.what()));
    } catch (...) {
      this->editor.displayInformativeMessage(
          "MFrontBehaviourWizard::write: unknown exception");
    }
  }  // end of MFrontBehaviourWizard::write

  void MFrontBehaviourWizard::writeBehaviourBrickOptions(
      const QString& b) const {
    auto write_options =
        [this](const std::vector<mfront::bbrick::OptionDescription>& opts) {
          auto tc = this->d.textCursor();
          auto append = [&tc](const QString& text) { tc.insertText(text); };
          if (opts.empty()) {
            return;
          }
          append("{\n");
          for (decltype(opts.size()) i = 0; i != opts.size();) {
            const auto& o = opts[i];
            if (++i != opts.size()) {
              append(QString::fromStdString(o.name + ": , //" + o.description +
                                            "\n"));
            } else {
              append(QString::fromStdString(o.name + ": //" + o.description +
                                            "\n"));
            }
          }
          append("}");
        };
    if (b == "StandardElastoViscoPlasticity") {
      this->sevpbp->write();
    } else {
      try {
        const auto& f = mfront::BehaviourBrickFactory::getFactory();
        const auto o = [] {
          auto opts = DSLGenerationOptions{};
          opts.with_brick = false;
          return opts;
        }();
        const auto dsl = this->getBehaviourDSL(o);
        if (dsl == nullptr) {
          return;
        }
        auto bd = dsl->getBehaviourDescription();
        const auto bbrick = f.get(b.toStdString(), *dsl, bd);
        write_options(bbrick->getOptions(false));
      } catch (std::runtime_error& e) {
        this->editor.displayInformativeMessage(
            "MFrontBehaviourWizard::writeBehaviourBrickOptions: " +
            QString(e.what()));
      } catch (...) {
        this->editor.displayInformativeMessage(
            "MFrontBehaviourWizard::writeBehaviourBrickOptions: "
            "unknown exception");
      }
    }
  }  // end of MFrontBehaviourWizard::writeBehaviourBrickOptions

  MFrontBehaviourWizard::~MFrontBehaviourWizard() = default;

}  // end of namespace tfel::gui
