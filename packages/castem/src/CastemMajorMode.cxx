/*!
 * \file  CastemMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   05/09/2012
 */

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryFile>
#include <QtGui/QSyntaxHighlighter>
#include <QtGui/QDesktopServices>
#include "TFEL/Glossary/Glossary.hxx"
#include "TFEL/Glossary/GlossaryEntry.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ImportBehaviourWizard.hxx"
#include "QEmacs/ImportMFMBehaviourWizard.hxx"
#include "QEmacs/CastemMajorMode.hxx"
#include "QEmacs/CastemSyntaxHighlighter.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs {

  static QString getCastemName(const std::string& n) {
    using tfel::glossary::Glossary;
    static std::map<std::string, const char*> cns = {
        {Glossary::Temperature, "T"},
        {Glossary::KelvinTemperature, "TK"},
        {Glossary::MeanTemperature, "<T>"},
        {Glossary::IrradiationTemperature, "TIRR"},
        {Glossary::MeanIrradiationTemperature, "<TI>"},
        {Glossary::ThermalConductivity, "K"},
        {Glossary::ThermalConductivity1, "K1"},
        {Glossary::ThermalConductivity2, "K2"},
        {Glossary::ThermalConductivity3, "K3"},
        {Glossary::ConvectiveHeatTransferCoefficient, "H"},
        {Glossary::YoungModulus, "YOUN"},
        {Glossary::YoungModulus1, "YG1"},
        {Glossary::YoungModulus2, "YG2"},
        {Glossary::YoungModulus3, "YG3"},
        {Glossary::PoissonRatio, "NU"},
        {Glossary::PoissonRatio12, "NU12"},
        {Glossary::PoissonRatio23, "NU23"},
        {Glossary::PoissonRatio13, "NU13"},
        {Glossary::ShearModulus12, "G12"},
        {Glossary::ShearModulus23, "G23"},
        {Glossary::ShearModulus13, "G13"},
        {Glossary::MassDensity, "RHO"},
        {Glossary::SpecificHeat, "C"},
        {Glossary::ThermalExpansion, "ALPH"},
        {Glossary::ThermalExpansion1, "ALP1"},
        {Glossary::ThermalExpansion2, "ALP2"},
        {Glossary::ThermalExpansion3, "ALP3"},
        {Glossary::PlateWidth, "DIM3"},
        {Glossary::PowerDensity, "Q"},
        {Glossary::Porosity, "PORO"},
        {Glossary::GrainSize, "GRSI"},
        {Glossary::FissionDensity, "DFIS"},
        {Glossary::BurnUp_AtPercent, "FIMA"},
        {Glossary::IrradiationDamage, "DOSE"},
        {Glossary::Pressure, "PRES"},
        /* name of neutron flux and fluence */
        {Glossary::NeutronFlux, "FLUX"},
        {Glossary::NeutronFluence, "FLUE"},
        {Glossary::FastNeutronFlux_01MeV, "FFLX"},
        {Glossary::FastNeutronFluence_01MeV, "FFLE"},
        {Glossary::FastNeutronFlux_1MeV, "FFX2"},
        {Glossary::FastNeutronFluence_1MeV, "FFE2"},
        /* name of orthotropic axis */
        {Glossary::OrthotropicAxisX1, "V1X"},
        {Glossary::OrthotropicAxisY1, "V1Y"},
        {Glossary::OrthotropicAxisZ1, "V1Z"},
        {Glossary::OrthotropicAxisX2, "V2X"},
        {Glossary::OrthotropicAxisY2, "V2Y"},
        {Glossary::OrthotropicAxisZ2, "V2Z"},
        /* beam properties */
        {Glossary::CrossSectionArea, "SECT"},
        {Glossary::FirstAxisSecondMomentArea, "INRY"},
        {Glossary::SecondAxisSecondMomentArea, "INRZ"},
        {Glossary::TorsionConstant, "TORS"}};
    const auto p = cns.find(n);
    if (p != cns.end()) {
      return p->second;
    }
    return QString::fromStdString(n).left(4).toUpper();
  }  // end of getCastemNames

  static void insertText(QTextCursor& tc, const QStringList& words) {
    auto pw = words.begin();
    auto t = QString();
    while (pw != words.end()) {
      const auto w = pw->trimmed();
      if (t.isEmpty()) {
        t += w;
      } else {
        if (t.size() + 1 + w.size() < 72) {
          if (w != ";") {
            t += " ";
          }
          t += w;
        } else {
          tc.insertText(t + '\n');
          t.clear();
          t += w;
        }
      }
      ++pw;
    }
    if (!t.isEmpty()) {
      tc.insertText(t + '\n');
    }
  }  // end of insertText

  static void insertBehaviour(QEmacsTextEditBase& textEdit,
                              const BehaviourDescription& bd) {
    using tfel::glossary::Glossary;
    using tfel::material::MechanicalBehaviourBase;
    using tfel::material::ModellingHypothesis;
    auto b = bd.generate();
    if (!b) {
      return;
    }
    const auto fs =
        ((b->getBehaviourType() ==
          MechanicalBehaviourBase::STANDARDFINITESTRAINBEHAVIOUR) &&
         (b->getBehaviourKinematic() ==
          MechanicalBehaviourBase::FINITESTRAINKINEMATIC_F_CAUCHY));
    const auto orthotropic = b->getSymmetryType() == 1;
    auto tc = textEdit.textCursor();
    tc.beginEditBlock();
    // material properties
    if (!b->getMaterialPropertiesNames().empty()) {
      tc.insertText("* material properties for behaviour '" +
                    bd.behaviour + "'\n");
      tc.insertText("MPROPS = 'TABLE';\n");
      for (const auto& m : b->getMaterialPropertiesNames()) {
        if ((m == Glossary::OrthotropicAxisX1) ||
            (m == Glossary::OrthotropicAxisY1) ||
            (m == Glossary::OrthotropicAxisZ1) ||
            (m == Glossary::OrthotropicAxisX2) ||
            (m == Glossary::OrthotropicAxisY2) ||
            (m == Glossary::OrthotropicAxisZ2)) {
          continue;
        }
      if (((m == Glossary::ThermalExpansion) ||
           (m == Glossary::ThermalExpansion1) ||
           (m == Glossary::ThermalExpansion2) ||
           (m == Glossary::ThermalExpansion3)) &&
          (fs)) {
        continue;
      }
      const auto mp = QString::fromStdString(m);
      auto mp_compare = [&mp](const MaterialPropertyDescription& mpd) {
        if (mpd.is<ConstantMaterialPropertyDescription>()) {
          const auto& cmpd =
              mpd.get<ConstantMaterialPropertyDescription>();
          return mp == cmpd.name;
        } else if (mpd.is<CastemMaterialPropertyDescription>()) {
          const auto& cmpd =
              mpd.get<CastemMaterialPropertyDescription>();
          return mp == cmpd.name;
        }
        return false;
      };
      const auto pmpd =
          std::find_if(bd.material_properties.begin(),
                       bd.material_properties.end(), mp_compare);
      if (pmpd != bd.material_properties.end()) {
        if (pmpd->is<ConstantMaterialPropertyDescription>()) {
          const auto& cmpd =
              pmpd->get<ConstantMaterialPropertyDescription>();
          tc.insertText("MPROPS. '" + mp + "' = " +
                        QString::number(cmpd.value) + ";\n");
        } else if (pmpd->is<CastemMaterialPropertyDescription>()) {
          const auto& cmpd =
              pmpd->get<CastemMaterialPropertyDescription>();
          tc.insertText("MPROPS. '" + mp + "' = 'TABLE';\n");
          tc.insertText("MPROPS. '" + mp + "'. 'MODELE' = '" +
                        cmpd.function + "';\n");
          tc.insertText("MPROPS. '" + mp + "'. 'LIBRAIRIE' = '" +
                        cmpd.library + "';\n");
          auto args = QStringList{};
          args.append("MPROPS. '" + mp + "'. 'VARIABLES' = ");
          for (const auto& a : cmpd.arguments) {
            args.append("'" + getCastemName(a.toStdString()) + "'");
          }
          args.append(";");
          insertText(tc, args);
        } else {
          tc.insertText("MPROPS. '" + mp + "' = ...;\n");
        }
        } else {
          tc.insertText("MPROPS. '" + mp + "' = ...;\n");
        }
      }
      tc.insertText("\n");
    }
    auto mprops = QStringList();
    mprops.append("MPNAMES =");
    for (const auto& n : b->getMaterialPropertiesNames()) {
      mprops.append("'" +getCastemName(n)+"'");
    }
    mprops.append(";");
    insertText(tc, mprops);
    if (!b->getInternalStateVariablesNames().empty()) {
      auto isvs = QStringList();
      isvs.append("ISVNAMES = 'MOTS'");
      for (const auto& isvn : b->getInternalStateVariablesNames()) {
        auto n = getCastemName(isvn);
        const auto isvtype = b->getInternalStateVariableType(isvn);
        if (isvtype == 0) {
          // scalar
          isvs.append("'" + n + "'");
        } else if (isvtype == 1) {
          // symmetric tensor
          if (n.size() > 2) {
            n = n.left(2);
          }
          for (const auto& s : b->getStensorComponentsSuffixes()) {
            isvs.append("'" + n + QString::fromStdString(s) + "'");
          }
        } else if (isvtype == 3) {
          // unsymmetric tensor
          if (n.size() > 2) {
            n = n.left(2);
          }
          for (const auto& s : b->getTensorComponentsSuffixes()) {
            isvs.append("'" + n + QString::fromStdString(s) + "'");
          }
        } else {
          isvs.clear();
          isvs.append(QObject::tr("* error: unsupported internal "
                                  "state variable type for "
                                  "variable '%1''")
                          .arg(QString::fromStdString(isvn)));
          break;
        }
      }
      isvs.append(";");
      insertText(tc, isvs);
    }
    if (!b->getExternalStateVariablesNames().empty()) {
      auto esvs = QStringList();
      esvs.append("ESVNAMES = 'MOTS'");
      for (const auto& esvn : b->getExternalStateVariablesNames()) {
        esvs.append("'" + getCastemName(esvn) + "'");
      }
      esvs.append(";");
      insertText(tc,esvs);
    }
    auto modl = QStringList();
    modl.append("??? = 'MODELISER' ???? 'MECANIQUE' 'ELASTIQUE'");
    if (!orthotropic) {
      modl.append(" 'ISOTROPE'");
    } else {
      modl.append(" 'ORTHOTROPE'");
    }
    if (fs) {
      modl.append(" 'EPSILON' 'UTILISATEUR'");
    }
    modl.append("'LIB_LOI' '" + bd.library + "'");
    modl.append("'FCT_LOI' '" + bd.behaviour + "'");
    modl.append("'C_MATERIAU' MPNAMES");
    if (!b->getInternalStateVariablesNames().empty()) {
      modl.append("'C_VARINTER' ISVNAMES");
    }
    if (!b->getExternalStateVariablesNames().empty()) {
      modl.append("'PARA_LOI' ESVNAMES");
    }
    const auto h = b->getHypothesis();
    if ((h ==
         ModellingHypothesis::AXISYMMETRICALGENERALISEDPLANESTRAIN) ||
        (h == ModellingHypothesis::GENERALISEDPLANESTRAIN)) {
      modl.append("'DPGE' ????");
    }
    modl.append(";");
    insertText(tc, modl);
    // call to MATERIAU
    auto mate = QStringList();
    mate.append("??? = 'MATERIAU' ???? ");
    for (const auto& n : b->getMaterialPropertiesNames()) {
      if ((n == Glossary::OrthotropicAxisX1) ||
          (n == Glossary::OrthotropicAxisY1) ||
          (n == Glossary::OrthotropicAxisZ1) ||
          (n == Glossary::OrthotropicAxisX2) ||
          (n == Glossary::OrthotropicAxisY2) ||
          (n == Glossary::OrthotropicAxisZ2)) {
        continue;
      }
      if (((n == Glossary::ThermalExpansion) ||
           (n == Glossary::ThermalExpansion1) ||
           (n == Glossary::ThermalExpansion2) ||
           (n == Glossary::ThermalExpansion3)) &&
          (fs)) {
        mate.append("'" + getCastemName(n) + "' 0");
      } else {
        const auto mp = QString::fromStdString(n);
        mate.append("'" + getCastemName(n) + "' (MPROPS. '" + mp +
                    "')");
      }
    }
    if (orthotropic) {
      if ((h == ModellingHypothesis::GENERALISEDPLANESTRAIN) ||
          (h == ModellingHypothesis::PLANESTRAIN) ||
          (h == ModellingHypothesis::PLANESTRESS) ||
          (h == ModellingHypothesis::AXISYMMETRICAL)) {
        mate.append("'DIRECTION' (1 0) 'PARALLELE'");
      } else if (h == ModellingHypothesis::TRIDIMENSIONAL) {
        mate.append("'DIRECTION' (1 0 0) (0 1 0) 'PARALLELE'");
      }
    }
    mate.append(";");
    insertText(tc, mate);
    tc.endEditBlock();
    textEdit.setTextCursor(tc);
  }  // end of insertBehaviour

  QStringList CastemMajorMode::buildKeysList() {
    return {"DROITE",       "MOT",
            "OPTION",       "FIN",
            "DENSITE",      "LAPLACE",
            "CERCLE",       "COTE",
            "QUELCONQUE",   "INTERSECTION",
            "PARABOLE",     "ET",
            "POINT",        "PLUS",
            "MOINS",        "TRANSLATION",
            "ROTATION",     "TRACER",
            "INVERSE",      "ELEMENT",
            "CONTOUR",      "DIFF",
            "CHANGER",      "SURFACE",
            "CONFONDRE",    "INFO",
            "TOURNER",      "HOMOTHETIE",
            "AFFINITE",     "SYMETRIE",
            "INCLUSION",    "TITRE",
            "RACCORD",      "TASSER",
            "SORTIR",       "LIRE",
            "BARYCENTRE",   "DALLER",
            "ORIENTER",     "OUBLIER",
            "COMPORTEMENT", "COUTURE",
            "PAVE",         "COMMENTAIRE",
            "NOEUD",        "MOT",
            "NBEL",         "NOTICE",
            "FACE",         "COORDONNEE",
            "NORME",        "TEMPS",
            "VOLUME",       "LECT",
            "SAUF",         "DEPLACER",
            "PSCAL",        "PVEC",
            "PMIX",         "LIAISON",
            "REGLER",       "HOOKE",
            "SOLS",         "AFFECTE",
            "RIGIDITE",     "BLOQUE",
            "DEPIMPOSE",    "HOTANGE",
            "STRUCTURE",    "TEXTE",
            "PROJETER",     "ELST",
            "JONC",         "RECO",
            "MASSE",        "CLST",
            "SIGMA",        "RELATION",
            "FORCE",        "PRCH",
            "BASE",         "DIMENSION",
            "EXTRAIRE",     "VERSENS",
            "VIBRATION",    "MAXIMUM",
            "XTMX",         ">EG",
            "<EG",          "OU",
            "EGA",          "NON",
            "NEG",          "MULT",
            "FLOTTANT",     "PJBA",
            "CRITERE",      "DIAGNEG",
            "XTX",          "CHOC",
            "BSIGMA",       "PRHC",
            "MAX1",         "REPETER",
            "QUITTER",      "SI",
            "SINO",         "FINSI",
            "IPOL",         "ABS",
            "SIN",          "ATG",
            "ENVELOPPE",    "ISOV",
            "DETRUIRE",     "ENLEVER",
            "REMPLACER",    "INSERER",
            "COLI",         "TABLE",
            "REDUCTION",    "SYMT",
            "ANTISYMETRIE", "RESULT",
            "PRES",         "EXCO",
            "NOMC",         "DEFORME",
            "APPUI",        "INVA",
            "PRIN",         "VMIS",
            "KSIGMA",       "SIGN",
            "SUITE",        "VALPROPRE",
            "ORDONNER",     "TIRE",
            "REGENERER",    "DESSIN",
            "AMOR",         "CHARGEMENT",
            "COULEUR",      "AFCO",
            "EVOLUTION",    "ORTHOGONALISER",
            "THETA",        "COMBTABLE",
            "DEVERSOIR",    "VECTEUR",
            "PICA",         "COPIER",
            "DIMNOYAU",     "SAUVER",
            "RESTITUER",    "CARACTERISTIQUE",
            "MATERIAU",     "GENERATRICE",
            "LOG",          "CAPACITE",
            "ELFE",         "JACOBIEN",
            "PLAS",         "GREEN",
            "MODELISER",    "PROCEDURE",
            "FINPROC",      "DEBPROC",
            "KTAN",         "FORME",
            "MESSAGE",      "NNOR",
            "CUBP",         "CUBT",
            "CER3",         "SEISME",
            "ENER",         "EPSI",
            "INTG",         "COURBE",
            "REACTION",     "SUPER",
            "ZERO",         "EXCITER",
            "KP",           "ACTIVE",
            "ELASTICITE",   "ERREUR",
            "CONGE",        "LUMP",
            "OBTENIR",      "VARI",
            "MODIFIER",     "MASQUE",
            "EXISTE",       "MINIMUM",
            "GRADIENT",     "ENSEMBLE",
            "IFRE",         "SIGSOL",
            "MAPP",         "SOMME",
            "BRUIT",        "RTENS",
            "DSPR",         "TFR",
            "TOTEMP",       "GRAF",
            "TRESCA",       "TYPE",
            "OSCI",         "SPO",
            "INDEX",        "CHSP",
            "DFOURIER",     "TAGR",
            "PERMEABILITE", "CABLE",
            "FOFI",         "WORK",
            "QULX",         "DEBIT",
            "ARTICULATION", "CMOY",
            "COMT",         "CONDUCTIVITE",
            "FLUX",         "MODL",
            "MATR",         "RIMP",
            "FILTRE",       "CONCAT",
            "ITERER",       "ACQUERIR",
            "SOURCE",       "CONVECTION",
            "PSMO",         "ECOULE",
            "INSI",         "MENAGE",
            "SYNTHESE",     "ARGUMENT",
            "GLIN",         "DYNE",
            "FONCTION",     "RESPRO",
            "PLACE",        "VALEUR",
            "PROI",         "EXCELLENCE",
            "ARETE",        "CALP",
            "INDI",         "ACT3",
            "BIOT",         "DEDU",
            "CONNECTIVITE", "NLOC",
            "CHAINE",       "COSI",
            "CVOL",         "DIAD",
            "HANN",         "LSQF",
            "LTL",          "PERT",
            "PRNS",         "PSRS",
            "SIAR",         "SPON",
            "VISAVIS",      "CCON",
            "MESURE",       "PILEPS",
            "SIMPLEX",      "UTILISATEUR",
            "MENU",         "COSH",
            "SINH",         "DEG3",
            "AIDE",         "RACPOL",
            "REFE",         "KSOF",
            "NSKE",         "KMAB",
            "CNEQ",         "NOEL",
            "DOMA",         "FPU",
            "GMV",          "EQPR",
            "EQEX",         "VIBC",
            "AVCT",         "KDIA",
            "KMTP",         "KMF",
            "MDIA",         "DFDT",
            "TCRR",         "TCNM",
            "SQTP",         "NLIN",
            "CMCT",         "KCHT",
            "LAPN",         "RAFT",
            "KLOP",         "KRES",
            "CSON",         "NUAGE",
            "WEIP",         "KHIS",
            "KOPS",         "FSUR",
            "FLAM",         "ELNO",
            "TANH",         "DBIT",
            "NS",           "TOIM",
            "FIMP",         "KMBT",
            "KBBT",         "DUDW",
            "FROT",         "KONV",
            "KCHA",         "MHYB",
            "MATP",         "HDEB",
            "HVIT",         "HYBP",
            "SMTP",         "MOCU",
            "CHAUSSETTE",   "TAILLE",
            "ERF",          "SENSIBILITE",
            "IMPOSE",       "DANS",
            "IMPF",         "FRON",
            "FUIT",         "EPTH",
            "FPT",          "KFPT",
            "FPA",          "KFPA",
            "ECHI",         "KPRO",
            "FFOR",         "RAYE",
            "RAYN",         "VSUR",
            "TRAJ",         "AJU1",
            "AJU2",         "EXCF",
            "PREC",         "ONDE",
            "CFL",          "DEDOUBLE",
            "DCOV",         "PARC",
            "FRIG",         "POLA",
            "CHI1",         "CHI2",
            "PENT",         "PRET",
            "METH",         "XXT",
            "CBLO",         "ZLEG",
            "MESM",         "FION",
            "NEUT",         "LOGK",
            "COAC",         "RESISTANCE",
            "MUTU",         "DIRI",
            "LIGN",         "OBJET",
            "DEBMETH",      "FINM",
            "HERITE",       "DECO",
            "EXTE",         "DMTD",
            "BMTD",         "SSCH",
            "MREM",         "ASSISTANT",
            "FISS",         "PRIMITIVE",
            "ANNU",         "SAIS",
            "CHOI",         "DETO",
            "PARTITION",    "CLMI",
            "PMAT",         "EXCP",
            "PROPHASE",     "ALEA",
            "GNFL",         "MPRO",
            "SSTE",         "ADVE",
            "BGMO",         "ECFE",
            "COUPER",       "DFER",
            "GYROSCOPIQUE", "CORIOLIS",
            "KENT",         "FANTOME",
            "ITRC",         "RETO",
            "IJET",         "MOCA",
            "LEVM",         "RAVC",
            "IDLI",         "RAFF",
            "CFND",         "ADET",
            "PSIP",         "ASIN",
            "TAN",          "TRIE",
            "GANE",         "HIST",
            "ETG",          "OTER",
            "XFEM",         "MOTA",
            "LIST",         "ELIM",
            "MANU",         "NBNO",
            "PROGRESSION",  "RESO",
            "VENV",         "MOME",
            "YTMX",         "ENTI",
            "MOTS",         "COS",
            "TRIA",         "SAUT",
            "CARB",         "CHPO",
            "CAPI",         "EXP",
            "XTY",          "FDT ",
            "DEPB",         "TFRI",
            "SOMT",         "FIMP",
            "TSCA",         "DIVU",
            "NTAB",         "QOND",
            "GENJ",         "SORE",
            "DMMU",         "PROB",
            "PHAJ",         "VERM",
            "IMPE",         "ACOS",
            "MEPL"};
  }  // end of buildKeysList

  const QStringList& CastemMajorMode::getKeysList() {
    static QStringList keys(buildKeysList());
    return keys;
  }  // end of getKeysList

  struct CastemMajorModeRessourceLoader {
    CastemMajorModeRessourceLoader() : l(getLoader()) {}

   private:
    struct Loader {
      Loader() { Q_INIT_RESOURCE(CastemModeResources); }
    };
    static Loader& getLoader() {
      static Loader l;
      return l;
    }
    Loader l;
  };

  CastemMajorMode::CastemMajorMode(QEmacsWidget& w,
                                   QEmacsBuffer& b,
                                   QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w, b, t, &t),
        co(nullptr),
        ha1(nullptr),
        ha2(nullptr) {
    this->c = new QCompleter(CastemMajorMode::getKeysList(), &t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,
                     static_cast<void (QCompleter::*)(const QString&)>(
                         &QCompleter::activated),
                     &t, &QEmacsTextEditBase::insertCompletion);
    // creating actions
    this->slc = new QAction(QObject::tr("Send line to Cast3M"), this);
    this->slc->setIcon(QIcon::fromTheme("system-run"));
    QObject::connect(this->slc, &QAction::triggered, this,
                     &CastemMajorMode::sendLineToCastem);
    this->src = new QAction(QObject::tr("Send region to Cast3M"), this);
    this->src->setIcon(QIcon::fromTheme("system-run"));
    QObject::connect(this->src, &QAction::triggered, this,
                     &CastemMajorMode::sendRegionToCastem);
    this->sbc =
        new QAction(QObject::tr("Send buffert to Cast3M"), this);
    this->sbc->setIcon(QIcon::fromTheme("system-run"));
    QObject::connect(this->sbc, &QAction::triggered, this,
                     &CastemMajorMode::sendBufferToCastem);
    this->iba = new QAction(QObject::tr("Import Behaviour"), this);
    QObject::connect(this->iba, &QAction::triggered, this,
                     &CastemMajorMode::showImportBehaviourWizard);
    this->imfmba =
        new QAction(QObject::tr("Import MFM behaviour"), this);
    QObject::connect(this->imfmba, &QAction::triggered, this,
                     &CastemMajorMode::showImportMFMBehaviourWizard);
  }  // end of CastemMajorMode

  void CastemMajorMode::showImportBehaviourWizard() {
    ImportBehaviourWizard w(this->textEdit);
    if (w.exec() == QDialog::Accepted) {
      insertBehaviour(this->textEdit, w.getSelectedBehaviour());
    }
  }  // end of CastemMajorMode::showImportBehaviourWizard

  void CastemMajorMode::showImportMFMBehaviourWizard() {
    using tfel::material::ModellingHypothesis;
    using tfel::system::ExternalLibraryManager;
    ImportMFMBehaviourWizard::Options o;
    o.minterface = "Castem";
    ImportMFMBehaviourWizard w(this->textEdit, o);
    if (w.exec() == QDialog::Accepted) {
      insertBehaviour(this->textEdit, w.getSelectedBehaviour());
    }
  }  // end of CastemMajorMode::showImportMFMBehaviourDialog

  bool CastemMajorMode::sendToCastem(const QString& l) {
    if (!this->startCastem()) {
      return false;
    }
    if (this->co == nullptr) {
      return false;
    }
    auto& p = this->co->getProcess();
    if (p.state() != QProcess::Running) {
      return false;
    }
    p.write((l + '\n').toLatin1());
    return true;
  }  // end of CastemMajorMode::sendToCastem

  bool CastemMajorMode::startCastem() {
    if (this->co == nullptr) {
      this->co =
          new ProcessInteractionFrame(this->qemacs, this->buffer);
      this->co->setMajorMode("castem-output");
      if (this->co == nullptr) {
        return false;
      }
      this->co->setAttribute(Qt::WA_DeleteOnClose);
      QObject::connect(this->co, &QObject::destroyed, this,
                       [this] { this->co = nullptr; });
      QDir d(this->textEdit.getDirectory());
      auto& p = co->getProcess();
      if (d.exists()) {
        p.setWorkingDirectory(d.absolutePath());
      } else {
        p.setWorkingDirectory(QDir::current().absolutePath());
      }
#ifdef _WIN32
      p.start("castem17.bat", QStringList());
#else
      p.start("castem2014_PLEIADES", QStringList());
#endif
      p.waitForStarted();
      this->buffer.attachSecondaryTask("* castem *", this->co);
      if (p.state() != QProcess::Running) {
        return false;
      }
    }
    auto& p = co->getProcess();
    if (p.state() != QProcess::Running) {
      this->co = nullptr;
      return this->startCastem();
    }
    return true;
  }

  QString CastemMajorMode::getName() const { return "Cast3M"; }

  QString CastemMajorMode::getDescription() const {
    return "major mode dedicated to the "
           "castem finite element solver";
  }

  QIcon CastemMajorMode::getIcon() const {
    static QIcon i(":/Cast3MIcon.png");
    return i;
  }  // end of CastemMajorMode::getIcon()

  void CastemMajorMode::setSyntaxHighlighter(QTextDocument* d) {
    new CastemSyntaxHighlighter(d);
  }

  QCompleter* CastemMajorMode::getCompleter() {
    return this->c;
  }  // end of getCompleter

  void CastemMajorMode::completeCurrentWord(QEmacsTextEditBase& t,
                                            const QString& w) {
    auto tc = t.textCursor();
    tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    tc.insertText("'" + w.toUpper() + "' ");
    t.setTextCursor(tc);
  }  // end of completeCurrentWord

  bool CastemMajorMode::isCastemKeyWord(const QString& w) {
    const auto& keys = CastemMajorMode::getKeysList();
    for (const QString& k : keys) {
      if (k.left(4) == w) {
        return true;
      }
    }
    return false;
  }

  QMenu* CastemMajorMode::getSpecificMenu() {
    auto* t = qobject_cast<QWidget*>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    auto* m = new QMenu(this->getName(), t);
    m->addAction(this->slc);
    m->addAction(this->src);
    m->addAction(this->sbc);
    m->addSeparator();
    m->addAction(this->iba);
    m->addAction(this->imfmba);
    return m;
  }  // end of CastemMajorMode::getSpecificMenu

  void CastemMajorMode::indentLine(const QTextCursor& c_) {
    QTextCursor tc(c_);
    tc.beginEditBlock();
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    tc.insertText("  ");
    tc.endEditBlock();
  }  // end of CastemMajorMode::indentLine

  bool CastemMajorMode::handleShortCut(const int k1,
                                       const Qt::KeyboardModifiers m,
                                       const int k2) {
    if ((k1 == Qt::Key_X) && (m == Qt::NoModifier) &&
        (k2 == Qt::Key_H)) {
      const auto& w = this->textEdit.getCurrentWord();
      const auto& w2 = w.left(4);
      if (this->isCastemKeyWord(w2)) {
        this->displayHelp(w, w2);
        return true;
      }
    } else if ((k1 == Qt::Key_C) && (k2 == Qt::Key_L) &&
               (m == Qt::ControlModifier)) {
      return this->sendLineToCastem();
    } else if ((k1 == Qt::Key_C) && (k2 == Qt::Key_R) &&
               (m == Qt::ControlModifier)) {
      return this->sendRegionToCastem();
    } else if ((k1 == Qt::Key_C) && (k2 == Qt::Key_B) &&
               (m == Qt::ControlModifier)) {
      return this->sendBufferToCastem();
    }
    return false;
  }  // end of CastemMajorMode::handleShortCut

  void CastemMajorMode::completeContextMenu(QMenu* const m,
                                            const QTextCursor& tc) {
    QEmacsMajorModeBase::completeContextMenu(m, tc);
    QTextCursor mtc(tc);
    mtc.select(QTextCursor::WordUnderCursor);
    const auto w = mtc.selectedText();
    if (this->isCastemKeyWord(w.left(4))) {
      delete this->ha1;
      delete this->ha2;
      this->ha1 =
          new QAction(QObject::tr("Help on %1 (notice)").arg(w), this);
      this->ha1->setData(w);
      this->ha2 =
          new QAction(QObject::tr("Help on %1 (web)").arg(w), this);
      this->ha2->setData(w);
      const auto& cactions = m->actions();
      if (cactions.isEmpty()) {
        m->addAction(this->ha1);
        m->addAction(this->ha2);
      } else {
        m->insertAction(*(cactions.begin()), this->ha1);
        m->insertAction(*(cactions.begin()), this->ha2);
      }
      QObject::connect(m, &QMenu::triggered, this,
                       &CastemMajorMode::actionTriggered);
    }
  }  // end of CastemMajorMode::completeContextMenu

  void CastemMajorMode::actionTriggered(QAction* a) {
    if (a == this->ha1) {
      const auto w = this->ha1->data().toString();
      this->displayHelp(w, w.left(4));
    } else if (a == this->ha2) {
      const auto w = this->ha2->data().toString();
      this->openWebHelp(w.left(4));
    }
  }

  void CastemMajorMode::displayHelp(const QString& w,
                                    const QString& w2) {
    // creating a temporary directory
    QString path;
    {
      QTemporaryFile file;
      if (file.open()) {
        path = file.fileName();
      } else {
        return;
      }
      file.close();
    }
    QDir dir(path);
    dir.mkdir(path);
    if (dir.exists()) {
      auto* nf = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.attachSecondaryTask(QObject::tr("notice %1").arg(w),
                                       nf);
      auto& p = nf->getProcess();
      p.setWorkingDirectory(dir.absolutePath());
      if (p.state() != QProcess::Running) {
        p.start("castem2014_PLEIADES", QStringList());
        p.waitForStarted();
        p.write(QString("INFO '%1';\n").arg(w2).toLatin1());
        p.write("FIN;\n");
        p.waitForFinished(1000);
      }
      bool r = true;
      for (auto info : dir.entryInfoList(
               QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                   QDir::AllDirs | QDir::Files,
               QDir::DirsFirst)) {
        if (!info.isDir()) {
          QFile::remove(info.absoluteFilePath());
        } else {
          r = false;
        }
      }
      if (r) {
        dir.rmdir(path);
      }
    }

  }  // end of CastemMajorMode::displayHelp

  void CastemMajorMode::openWebHelp(const QString& w2) {
    QDesktopServices::openUrl(
        QUrl("http://www-cast3m.cea.fr/index.php?page=notices&notice=" +
             w2.toLower()));
  }  // end of CastemMajorMode::openWebHelp

  int CastemMajorMode::getMinimalCompletionLength() {
    return 2;
  }  // end of CastemMajorMode::getMinimalCompletionLength

  void CastemMajorMode::format() {}  // end of CastemMajorMode::format

  QString CastemMajorMode::getCommentSyntax() {
    return "*";
  }  // end of CastemMajorMode::getCommentSyntax

  bool CastemMajorMode::sendLineToCastem() {
    auto tc = this->textEdit.textCursor();
    tc.select(QTextCursor::LineUnderCursor);
    return this->sendToCastem(tc.selectedText());
  }  // end of CastemMajorMode::sendLineToCastem

  bool CastemMajorMode::sendRegionToCastem() {
    auto tc = this->textEdit.textCursor();
    if (!tc.hasSelection()) {
      return false;
    }
    QTextCursor b, e;
    this->beginAndEndOfSelection(b, e, tc);
    e.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    e.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    while (b < e) {
      b.select(QTextCursor::LineUnderCursor);
      if (!this->sendToCastem(b.selectedText())) {
        return false;
      }
      b.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
    }
    return true;
  }  // end of CastemMajorMode::sendRegionToCastem

  bool CastemMajorMode::sendBufferToCastem() {
    auto b = this->textEdit.textCursor();
    auto e = b;
    b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    e.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    while (b < e) {
      b.select(QTextCursor::LineUnderCursor);
      if (!this->sendToCastem(b.selectedText())) {
        return false;
      }
      b.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
    }
    return true;
  }  // end of CastemMajorMode::sendBufferToCastem

  CastemMajorMode::~CastemMajorMode() {
    delete this->ha1;
    delete this->ha2;
  }

  static StandardQEmacsMajorModeProxy<CastemMajorMode> proxy(
      "Cast3M",
      QVector<QRegExp>() << QRegExp("^.+\\.dgibi$"),
      ":/Cast3MIcon.png");

}  // end of namespace qemacs
