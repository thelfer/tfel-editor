/*!
 * \file   BehaviourDescription.cxx
 * \brief
 * \author th202608
 * \date   01/03/2018
 */

#ifdef TFEL_HAS_ABAQUS_INTERFACE_SUPPORT
#include "MTest/AbaqusStandardBehaviour.hxx"
#endif /* TFEL_HAS_ABAQUS_INTERFACE_SUPPORT */
#ifdef TFEL_HAS_ABAQUS_EXPLICIT_INTERFACE_SUPPORT
#include "MTest/AbaqusExplicitBehaviour.hxx"
#endif /* TFEL_HAS_ABAQUS_EXPLICIT_INTERFACE_SUPPORT */
#ifdef TFEL_HAS_ANSYS_INTERFACE_SUPPORT
#include "MTest/AnsysStandardBehaviour.hxx"
#endif /* TFEL_HAS_ANSYS_INTERFACE_SUPPORT */
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"

namespace tfel::gui {

  BehaviourDescription::BehaviourDescription() = default;
  BehaviourDescription::BehaviourDescription(BehaviourDescription&&) = default;
  BehaviourDescription::BehaviourDescription(const BehaviourDescription&) =
      default;
  BehaviourDescription& BehaviourDescription::operator=(
      BehaviourDescription&&) = default;
  BehaviourDescription& BehaviourDescription::operator=(
      const BehaviourDescription&) = default;

  static std::string BehaviourDescription_getFunction(
      const BehaviourDescription& bd) {
    using namespace mtest;
    using tfel::material::ModellingHypothesis;
    try {
      const auto si = bd.minterface.toStdString();
      const auto sl = bd.library.toStdString();
      const auto sh =
          ModellingHypothesis::fromString(bd.hypothesis.toStdString());
      const auto f = bd.behaviour.toStdString();
#ifdef TFEL_HAS_ABAQUS_INTERFACE_SUPPORT
      if ((si == "abaqus") || (si == "abaqus_standard") ||
          (si == "abaqus_umat") || (si == "Abaqus")) {
        return f + AbaqusStandardBehaviour::getHypothesisSuffix(sh);
      }
#endif /* TFEL_HAS_ABAQUS_INTERFACE_SUPPORT */
#ifdef TFEL_HAS_ABAQUS_INTERFACE_SUPPORT
      if ((si == "abaqus_explicit") || (si == "abaqus_vumat") ||
          (si == "AbaqusExplicit")) {
        return f + AbaqusExplicitBehaviour::getHypothesisSuffix(sh);
      }
#endif /* TFEL_HAS_ABAQUS_INTERFACE_SUPPORT */
#ifdef TFEL_HAS_ANSYS_INTERFACE_SUPPORT
      if ((si == "ansys") || (si == "ansys_usermat") || (si == "Ansys")) {
        return f + AnsysStandardBehaviour::getHypothesisSuffix(sh);
      }
#endif /* TFEL_HAS_ANSYS_INTERFACE_SUPPORT */
      return f;
    } catch (std::exception& e) {
      debug("BehaviourDescription::getFunction:", e.what());
    } catch (...) {
      debug("BehaviourDescription::getFunction: unexpected exception");
    }
    return "";
  }  // end of BehaviourDescription_getFunction

  QString BehaviourDescription::getFunction() const {
    return QString::fromStdString(BehaviourDescription_getFunction(*this));
  }  // end of BehaviourDescription::getFunction()

  std::shared_ptr<mtest::Behaviour> BehaviourDescription::generate() const {
    using tfel::material::ModellingHypothesis;
    try {
      const auto si = this->minterface.toStdString();
      const auto sl = this->library.toStdString();
      const auto sh =
          ModellingHypothesis::fromString(this->hypothesis.toStdString());
      mtest::Behaviour::Parameters params;
      const auto sf = BehaviourDescription_getFunction(*this);
      if (sf.empty()) {
        return {};
      }
      return mtest::Behaviour::getBehaviour(si, sl, sf, params, sh);
    } catch (std::exception& e) {
      debug("BehaviourDescription::generate:", e.what());
    } catch (...) {
      debug("BehaviourDescription::generate: unexpected exception");
    }
    return {};
  }  // end of BehaviourDescription::generate

}  // end of namespace tfel::gui
