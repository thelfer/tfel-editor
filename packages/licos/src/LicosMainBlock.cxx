/*! 
 * \file  LicosMainBlock.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#include<stdexcept>

#include"QEmacs/LicosMainBlock.hxx"

namespace qemacs
{

  struct CouplingItemBlock
    : public StandardLicosBlock
  {
    CouplingItemBlock(const QString&);
  };

  CouplingItemBlock::CouplingItemBlock(const QString& n_)
    : StandardLicosBlock(n_)
  {
    this->addKey("ActivatingEvent");
    this->addKey("DesactivatingEvent");
    this->addKey("ActivatingEvents");
    this->addKey("DesactivatingEvents");
    this->addKey("Active");
    this->addKey("FieldOutput");
    this->addKey("FieldRequirement");
    this->addKeys(getParameterKeys());
  } // end of CouplingItemBlock::CouplingItemBlock

  struct GlobalCouplingItemBlock
    : public StandardLicosBlock
  {
    GlobalCouplingItemBlock(const QString&);
  };

  GlobalCouplingItemBlock::GlobalCouplingItemBlock(const QString& n_)
    : StandardLicosBlock(n_)
  {
    this->addKeys(getParameterKeys());
  } // end of GlobalCouplingItemBlock::GlobalCouplingItemBlock

  struct PlenumBlock
    : public StandardLicosBlock
  {
    PlenumBlock();
  };

  struct MaterialDescriptionBlock
    : public StandardLicosBlock
  {
    MaterialDescriptionBlock(const QString& = "MaterialDescription");
  };

  struct GaseousMaterialDescriptionBlock
    : public StandardLicosBlock
  {
    GaseousMaterialDescriptionBlock(const QString& = "GaseousMaterialDescription");
  };

  struct GaseousMaterialBlock
    : public GaseousMaterialDescriptionBlock
  {
    GaseousMaterialBlock();
  };

  struct LicosGlobalGaseousMaterialBlock
    : public GaseousMaterialDescriptionBlock
  {
    LicosGlobalGaseousMaterialBlock();
  };

  struct MaterialBlock
    : public MaterialDescriptionBlock
  {
    MaterialBlock();
  };

  MaterialBlock::MaterialBlock()
    : MaterialDescriptionBlock("Material")
  {
    this->addKey("AnisotropicAxises");
  }

  GaseousMaterialBlock::GaseousMaterialBlock()
    : GaseousMaterialDescriptionBlock("GaseousMaterial")
  {}

  PlenumBlock::PlenumBlock()
    : StandardLicosBlock("Plenum")
  {
    this->addKey("TemperatureEvolution");
    this->addKey("Volume");
  }

  LicosGlobalGaseousMaterialBlock::LicosGlobalGaseousMaterialBlock()
    : GaseousMaterialDescriptionBlock("GlobalGaseousMaterial")
  {
    this->addKey("Gases");
    this->addBlock(new PlenumBlock);
  }

  MaterialDescriptionBlock::MaterialDescriptionBlock(const QString& n_)
    : StandardLicosBlock(n_)
  {
    this->addKey("MaterialProperty");
    this->addKey("ChemicalSpecies");
    this->addKey("StateVariable");
    this->addKeys(getParameterKeys());
    this->addBlock("ThermalBehaviour",
		   QStringList() << "MaterialProperty");
    this->addBlock("MechanicalBehaviour",
		   QStringList() << "MaterialProperty"
		   << "StateVariable"
		   << "InternalStateVariable"
		   << "HandleThermalExpansionComputation");
  }

  GaseousMaterialDescriptionBlock::GaseousMaterialDescriptionBlock(const QString& n_)
    : StandardLicosBlock(n_)
  {
    this->addKey("MaterialProperty");
    this->addKey("StateVariable");
    this->addKey("ChemicalSpecies");
    this->addKeys(getParameterKeys());
    this->addBlock("ThermalBehaviour",
		   QStringList() << "MaterialProperty");
  }

  struct ThermalLoadingBlock
    : public StandardLicosBlock
  {
    ThermalLoadingBlock(const QString& = "ThermalLoading");

    virtual bool
    isSubKey(const QString&) const;

    virtual bool
    isSubBlock(const QString&) const;

  }; // end of struct ThermalLoadingBlock

  ThermalLoadingBlock::ThermalLoadingBlock(const QString& n_)
    : StandardLicosBlock(n_)
  {} // end of ThermalLoadingBlock::ThermalLoadingBlock

  bool
  ThermalLoadingBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  ThermalLoadingBlock::isSubBlock(const QString&) const
  {
    return false;
  } // end of ThermalLoadingBlock::isSubBlock


  struct ThermalBoundaryConditionBlock
    : public ThermalLoadingBlock
  {
    ThermalBoundaryConditionBlock();
  }; // end of struct ThermalBoundaryConditionBlock

  ThermalBoundaryConditionBlock::ThermalBoundaryConditionBlock()
    : ThermalLoadingBlock("ThermalBoundaryCondition")
  {} // end of ThermalBoundaryConditionBlock::ThermalBoundaryConditionBlock

  struct MechanicalLoadingBlock
    : public StandardLicosBlock
  {
    MechanicalLoadingBlock(const QString& = "MechanicalLoading");

    virtual bool
    isSubKey(const QString&) const;

    virtual bool
    isSubBlock(const QString&) const;

  }; // end of struct MechanicalLoadingBlock

  MechanicalLoadingBlock::MechanicalLoadingBlock(const QString& n_)
    : StandardLicosBlock(n_)
  {} // end of MechanicalLoadingBlock::MechanicalLoadingBlock

  bool
  MechanicalLoadingBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  MechanicalLoadingBlock::isSubBlock(const QString&) const
  {
    return false;
  } // end of MechanicalLoadingBlock::isSubBlock

  struct MechanicalBoundaryConditionBlock
    : public MechanicalLoadingBlock
  {
    MechanicalBoundaryConditionBlock();
  }; // end of struct MechanicalBoundaryConditionBlock

  MechanicalBoundaryConditionBlock::MechanicalBoundaryConditionBlock()
    : MechanicalLoadingBlock("MechanicalBoundaryCondition")
  {} // end of MechanicalBoundaryConditionBlock::MechanicalBoundaryConditionBlock

  struct MechanicalPostComputationBlock
    : public StandardLicosBlock
  {
    MechanicalPostComputationBlock();

    virtual bool
    isSubKey(const QString&) const;

    virtual bool
    isSubBlock(const QString&) const;

  }; // end of struct MechanicalPostComputationBlock

  MechanicalPostComputationBlock::MechanicalPostComputationBlock()
    : StandardLicosBlock("MechanicalPostComputation")
  {} // end of MechanicalPostComputationBlock::MechanicalPostComputationBlock

  bool
  MechanicalPostComputationBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  MechanicalPostComputationBlock::isSubBlock(const QString&) const
  {
    return false;
  } // end of MechanicalPostComputationBlock::isSubBlock

  struct MechanicalPostProcessingBlock
    : public StandardLicosBlock
  {
    MechanicalPostProcessingBlock();

    virtual bool
    isSubKey(const QString&) const;

    virtual bool
    isSubBlock(const QString&) const;

  }; // end of struct MechanicalPostProcessingBlock

  MechanicalPostProcessingBlock::MechanicalPostProcessingBlock()
    : StandardLicosBlock("MechanicalPostProcessing")
  {} // end of MechanicalPostProcessingBlock::MechanicalPostProcessingBlock

  bool
  MechanicalPostProcessingBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  MechanicalPostProcessingBlock::isSubBlock(const QString&) const
  {
    return false;
  } // end of MechanicalPostProcessingBlock::isSubBlock

  struct MultipleCurves
    : public StandardLicosBlock
  {
    MultipleCurves();
  };

  MultipleCurves::MultipleCurves()
    : StandardLicosBlock("MultipleCurves")
  {
    this->addKey("Curve");
  }

  struct MultipleLineCurves
    : public StandardLicosBlock
  {
    MultipleLineCurves();
  };

  MultipleLineCurves::MultipleLineCurves()
    : StandardLicosBlock("MultipleLineCurves")
  {
    this->addKey("LineCurve");
  }

  struct RamsesStudy
    : public StandardLicosBlock
  {
    RamsesStudy();
  };

  RamsesStudy::RamsesStudy()
    : StandardLicosBlock("RamsesStudy")
  {
    this->addKey("Field");
    this->addKey("Evolution");
    this->addKey("StressInterpolationPolicy");
    this->addBlock("StressProfile",
		   QStringList() << getParameterKeys());
    this->addBlock("Criterium",
		   QStringList() << getParameterKeys());
  } // end of RamsesStudy::RamsesStudy

  struct LicosProcessBlock
    : public StandardLicosBlock
  {
    LicosProcessBlock();

    static const QStringList&
    getKeys(void);

    static const QMap<QString,QSharedPointer<LicosBlock> >&
    getBlocks(void);

  protected:

    static QStringList
    buildKeys(void);

    static const QMap<QString,QSharedPointer<LicosBlock> >
    buildBlocks(void);

  };

  struct LicosSpecificSectionBlock
    : public StandardLicosBlock
  {
    LicosSpecificSectionBlock();

    static const QStringList&
    getKeys(void);

    static const QMap<QString,QSharedPointer<LicosBlock> >&
    getBlocks(void);

  protected:

    static QStringList
    buildKeys(void);

    static const QMap<QString,QSharedPointer<LicosBlock> >
    buildBlocks(void);

  };

  struct CouplingBlock
    : public StandardLicosBlock
  {
    CouplingBlock();

    virtual bool
    isSubBlock(const QString&) const;

    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const;

  };

  struct Block
    : public StandardLicosBlock
  {
    Block();

    virtual QString blockEnd(void) const;

    virtual void
    treatParameters(const QStringList&);
  
    virtual void
    treatArguments(const CxxTokenizer::const_iterator,
		   const CxxTokenizer::const_iterator);

    virtual bool
    isSubBlock(const QString&) const;

    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const;

  private:
  
    QString end;
  };

  Block::Block()
    : StandardLicosBlock("Block"),
      end("EndOfBlock")
  {
    this->addKeys(getParameterKeys());
  }

  void
  Block::treatParameters(const QStringList&)
  {} // end of Block::treatParameters

  void
  Block::treatArguments(const CxxTokenizer::const_iterator p,
			const CxxTokenizer::const_iterator pe)
  {
    if(p!=pe){
      if(p->flag==Token::Standard){
	this->end = "EndOf" + p->value;
      }
    }
  } // end of Block::treatArguments

  QString
  Block::blockEnd(void) const
  {
    return this->end;
  }

  bool
  Block::isSubBlock(const QString& b) const
  {
    return b=="Block";
  }

  QSharedPointer<LicosBlock>
  Block::getSubBlock(const QString& b) const
  {
    using namespace std;
    if(b!="Block"){
      string msg("Block::getSubBlock : ");
      msg += "no sub block named '"+b.toStdString()+"'";
      throw(runtime_error(msg));
    }
    return QSharedPointer<LicosBlock>(new Block);
  }

  struct SubMeshesDataBlock
    : public StandardLicosBlock
  {
    SubMeshesDataBlock();
  };

  SubMeshesDataBlock::SubMeshesDataBlock()
    : StandardLicosBlock("SubMeshesData")
  {
    this->addKey("Map");
    this->addKey("Dictionary");
  }

  struct LoadingBlock
    : public LicosBlock
  {
    virtual QString name(void) const;

    virtual QString blockEnd(void) const;

    virtual void
    treatParameters(const QStringList&);
  
    virtual void
    treatArguments(const CxxTokenizer::const_iterator,
		   const CxxTokenizer::const_iterator);

    virtual bool isSubKey(const QString&) const;

    virtual bool isSubBlock(const QString&) const;

    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const;

    virtual ~LoadingBlock();

  };

  QString 
  LoadingBlock::name(void) const
  {
    return "Loading";
  }

  QString 
  LoadingBlock::blockEnd(void) const
  {
    return "EndOfLoading";
  }

  void
  LoadingBlock::treatParameters(const QStringList&)
  {}
  
  void
  LoadingBlock::treatArguments(const CxxTokenizer::const_iterator,
			       const CxxTokenizer::const_iterator)
  {}

  bool
  LoadingBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  LoadingBlock::isSubBlock(const QString&) const
  {
    return false;
  }

  QSharedPointer<LicosBlock>
  LoadingBlock::getSubBlock(const QString&) const
  {
    return QSharedPointer<LicosBlock>();
  }

  LoadingBlock::~LoadingBlock()
  {} // end of LoadingBlock::~LoadingBlock()

  struct GlobalLoadingEvolutionBlock
    : public LicosBlock
  {
    virtual QString name(void) const;

    virtual QString blockEnd(void) const;

    virtual void
    treatParameters(const QStringList&);
  
    virtual void
    treatArguments(const CxxTokenizer::const_iterator,
		   const CxxTokenizer::const_iterator);

    virtual bool isSubKey(const QString&) const;

    virtual bool isSubBlock(const QString&) const;

    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const;

    virtual ~GlobalLoadingEvolutionBlock();

  };

  QString 
  GlobalLoadingEvolutionBlock::name(void) const
  {
    return "GlobalLoadingEvolution";
  }

  QString 
  GlobalLoadingEvolutionBlock::blockEnd(void) const
  {
    return "EndOfGlobalLoadingEvolution";
  }

  void
  GlobalLoadingEvolutionBlock::treatParameters(const QStringList&)
  {}
  
  void
  GlobalLoadingEvolutionBlock::treatArguments(const CxxTokenizer::const_iterator,
					      const CxxTokenizer::const_iterator)
  {}

  bool
  GlobalLoadingEvolutionBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  GlobalLoadingEvolutionBlock::isSubBlock(const QString&) const
  {
    return false;
  }

  QSharedPointer<LicosBlock>
  GlobalLoadingEvolutionBlock::getSubBlock(const QString&) const
  {
    return QSharedPointer<LicosBlock>();
  }

  GlobalLoadingEvolutionBlock::~GlobalLoadingEvolutionBlock()
  {} // end of GlobalLoadingEvolutionBlock::~GlobalLoadingEvolutionBlock()


  struct LoadingEvolutionBlock
    : public LicosBlock
  {
    virtual QString name(void) const;

    virtual QString blockEnd(void) const;

    virtual void
    treatParameters(const QStringList&);
  
    virtual void
    treatArguments(const CxxTokenizer::const_iterator,
		   const CxxTokenizer::const_iterator);

    virtual bool isSubKey(const QString&) const;

    virtual bool isSubBlock(const QString&) const;

    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const;

    virtual ~LoadingEvolutionBlock();

  };

  QString 
  LoadingEvolutionBlock::name(void) const
  {
    return "LoadingEvolution";
  }

  QString 
  LoadingEvolutionBlock::blockEnd(void) const
  {
    return "EndOfLoadingEvolution";
  }

  void
  LoadingEvolutionBlock::treatParameters(const QStringList&)
  {}
  
  void
  LoadingEvolutionBlock::treatArguments(const CxxTokenizer::const_iterator,
					const CxxTokenizer::const_iterator)
  {}

  bool
  LoadingEvolutionBlock::isSubKey(const QString&) const
  {
    return true;
  }

  bool
  LoadingEvolutionBlock::isSubBlock(const QString&) const
  {
    return false;
  }

  QSharedPointer<LicosBlock>
  LoadingEvolutionBlock::getSubBlock(const QString&) const
  {
    return QSharedPointer<LicosBlock>();
  }

  LoadingEvolutionBlock::~LoadingEvolutionBlock()
  {} // end of LoadingEvolutionBlock::~LoadingEvolutionBlock()

  const QMap<QString,QSharedPointer<LicosBlock> >
  LicosSpecificSectionBlock::buildBlocks(void)
  {
    typedef QSharedPointer<LicosBlock> BPtr;
    typedef CouplingItemBlock LCIB;
    QMap<QString,BPtr> b;
    b["Block"]                       = BPtr(new Block);
    b["Coupling"]                    = BPtr(new CouplingBlock);
    b["Model"]                       = BPtr(new LCIB("Model"));
    b["AuxiliaryTask"]               = BPtr(new LCIB("AuxiliaryTask"));
    b["Criterium"]                   = BPtr(new LCIB("Criterium"));
    b["PostProcessing"]              = BPtr(new LCIB("PostProcessing"));
    b["Loading"]                     = BPtr(new LoadingBlock);
    b["LoadingEvolution"]            = BPtr(new LoadingEvolutionBlock);
    b["Material"]                    = BPtr(new MaterialBlock);
    b["MaterialDescription"]         = BPtr(new MaterialDescriptionBlock);
    b["GaseousMaterial"]             = BPtr(new GaseousMaterialBlock);
    b["GaseousMaterialDescription"]  = BPtr(new GaseousMaterialDescriptionBlock);
    b["ThermalLoading"]              = BPtr(new ThermalLoadingBlock);
    b["MultipleCurves"]              = BPtr(new MultipleCurves);
    b["MultipleLineCurves"]          = BPtr(new MultipleLineCurves);
    b["RamsesStudy"]                 = BPtr(new RamsesStudy);
    StandardLicosBlock *tl = new StandardLicosBlock("ThermalLoadings");
    tl->addKey("TL");
    tl->addKey("ThermalLoading");
    b["ThermalLoadings"]             = BPtr(tl);
    b["ThermalBoundaryCondition"]    = BPtr(new ThermalBoundaryConditionBlock);
    tl = new StandardLicosBlock("ThermalBoundaryConditions");
    tl->addKey("TBC");
    tl->addKey("ThermalBoundaryCondition");
    b["ThermalBoundaryConditions"]   = BPtr(tl);
    b["MechanicalLoading"]           = BPtr(new MechanicalLoadingBlock);
    StandardLicosBlock *ml = new StandardLicosBlock("MechanicalLoadings");
    ml->addKey("ML");
    ml->addKey("MechanicalLoading");
    b["MechanicalLoadings"]          = BPtr(ml);
    b["MechanicalBoundaryCondition"] = BPtr(new MechanicalBoundaryConditionBlock);
    ml = new StandardLicosBlock("MechanicalBoundaryConditions");
    ml->addKey("MBC");
    ml->addKey("MechanicalBoundaryCondition");
    b["MechanicalBoundaryConditions"] = BPtr(ml);
    b["MechanicalPostComputation"] = BPtr(new MechanicalPostComputationBlock);
    StandardLicosBlock *mp = new StandardLicosBlock("MechanicalPostComputations");
    mp->addKey("MPC");
    mp->addKey("MechanicalPostComputation");
    b["MechanicalPostComputations"]          = BPtr(mp);
    b["MechanicalPostProcessing"] = BPtr(new MechanicalPostProcessingBlock);
    StandardLicosBlock *mpp = new StandardLicosBlock("MechanicalPostProcessings");
    mpp->addKey("MPp");
    mpp->addKey("MechanicalPostProcessing");
    b["MechanicalPostProcessings"]          = BPtr(mpp);
    return b;
  } // end of LicosSpecificSectionBlock::buildBlocks

  const QMap<QString,QSharedPointer<LicosBlock> >&
  LicosSpecificSectionBlock::getBlocks(void)
  {
    typedef QSharedPointer<LicosBlock> BPtr;
    static QMap<QString,BPtr> b(buildBlocks());
    return b;
  }

  CouplingBlock::CouplingBlock()
    : StandardLicosBlock("Coupling")
  {
    this->addKeys(QStringList() << "Model"
		  << "AuxiliaryTask"
		  << "Criterium"
		  << "PostProcessing");
    this->addBlock(new CouplingItemBlock("SpecialisedModel"));
    this->addBlock(new CouplingItemBlock("SpecialisedAuxiliaryTask"));
    this->addBlock(new CouplingItemBlock("SpecialisedCriterium"));
    this->addBlock(new CouplingItemBlock("SpecialisedPostProcessing"));
    this->addBlock(new CouplingItemBlock("SpecializedModel"));
    this->addBlock(new CouplingItemBlock("SpecializedAuxiliaryTask"));
    this->addBlock(new CouplingItemBlock("SpecializedCriterium"));
    this->addBlock(new CouplingItemBlock("SpecializedPostProcessing"));
  } // end of CouplingBlock

  bool
  CouplingBlock::isSubBlock(const QString& b) const
  {
    if(b=="Coupling"){
      return true;
    }
    return StandardLicosBlock::isSubBlock(b);
  }

  QSharedPointer<LicosBlock>
  CouplingBlock::getSubBlock(const QString& b) const
  {
    if(b=="Coupling"){
      return QSharedPointer<LicosBlock>(new CouplingBlock());
    }
    return StandardLicosBlock::getSubBlock(b);
  } // end of StandardLicosBlock::getSubBlock

  struct GlobalCouplingBlock
    : public StandardLicosBlock
  {
    GlobalCouplingBlock();

    virtual bool
    isSubBlock(const QString&) const;

    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const;

  };

  struct LocalResolution
    : public StandardLicosBlock
  {
    LocalResolution();
  };

  LocalResolution::LocalResolution()
    : StandardLicosBlock("LocalResolution")
  {
    this->addBlock(new CouplingBlock);
  }

  GlobalCouplingBlock::GlobalCouplingBlock()
    : StandardLicosBlock("GlobalCoupling")
  {
    this->addKeys(QStringList() << "GlobalModel"
		  << "GlobalAuxiliaryTask"
		  << "GlobalCriterium"
		  << "GlobalPostProcessing");
    this->addKeys(getParameterKeys());
    this->addBlock(new LocalResolution);
    this->addBlock(new GlobalCouplingItemBlock("SpecialisedTimeStepController"));
    this->addBlock(new GlobalCouplingItemBlock("SpecialisedGlobalModel"));
    this->addBlock(new GlobalCouplingItemBlock("SpecialisedGlobalAuxiliaryTask"));
    this->addBlock(new GlobalCouplingItemBlock("SpecialisedGlobalCriterium"));
    this->addBlock(new GlobalCouplingItemBlock("SpecializedGlobalPostProcessing"));
    this->addBlock(new GlobalCouplingItemBlock("SpecializedTimeStepController"));
    this->addBlock(new GlobalCouplingItemBlock("SpecializedGlobalModel"));
    this->addBlock(new GlobalCouplingItemBlock("SpecializedGlobalAuxiliaryTask"));
    this->addBlock(new GlobalCouplingItemBlock("SpecializedGlobalCriterium"));
    this->addBlock(new GlobalCouplingItemBlock("SpecializedGlobalPostProcessing"));
  } // end of GlobalCouplingBlock::GlobalCouplingBlock

  bool
  GlobalCouplingBlock::isSubBlock(const QString& b) const
  {
    if(b=="GlobalCoupling"){
      return true;
    }
    return StandardLicosBlock::isSubBlock(b);
  }

  QSharedPointer<LicosBlock>
  GlobalCouplingBlock::getSubBlock(const QString& b) const
  {
    if(b=="GlobalCoupling"){
      return QSharedPointer<LicosBlock>(new GlobalCouplingBlock());
    }
    return StandardLicosBlock::getSubBlock(b);
  } // end of GlobalCouplingBlock::getSubBlock


  LicosSpecificSectionBlock::LicosSpecificSectionBlock()
    : StandardLicosBlock("SpecificSection")
  {
    this->addKeys(LicosSpecificSectionBlock::getKeys());
    this->addBlocks(LicosSpecificSectionBlock::getBlocks());
  }

  const QStringList&
  LicosSpecificSectionBlock::getKeys()
  {
    static QStringList l(LicosSpecificSectionBlock::buildKeys());
    return l;
  }

  QStringList
  LicosSpecificSectionBlock::buildKeys()
  {
    QStringList k;
    k << "GasGroup"
      << "CastemGraphicalOutput"
      << "Dependencies"
      << "Event"
      << "ExportToMED"
      << "ExportToTxt"
      << "ExportToVTK"
      << "GeneralisedPlaneStrainReferencePoints"
      << "Import"
      << "ImportExternalData"
      << "LoadingEvolutionFromTextFile"
      << "MPScript"
      << "MPScriptII"
      << "MPScriptIII"
      << "MPScriptIV"
      << "Materials"
      << "MechanicalMaterials"
      << "MechanicalPostProcessingScript"
      << "MechanicalPostProcessingScriptII"
      << "MechanicalPostProcessingScriptIII"
      << "MechanicalPostProcessingScriptIV"
      << "ParserFunction"
      << "PostProcessingTimes"
      << "ScalarValueTransformation"
      << "TPScript"
      << "TPScriptII"
      << "TPScriptIII"
      << "TPScriptIV"
      << "Curve"
      << "LineCurve"
      << "ThermalMaterials"
      << "ThermalPostProcessingScript"
      << "ThermalPostProcessingScriptII"
      << "ThermalPostProcessingScriptIII"
      << "ThermalPostProcessingScriptIV"
      << "UniformLoadingFromEvolution"
      << getParameterKeys();
    return k;
  }

  LicosProcessBlock::LicosProcessBlock()
    : StandardLicosBlock("Process")
  {
    this->addKeys(LicosProcessBlock::getKeys());
    this->addBlocks(LicosProcessBlock::getBlocks());
  }

  const QMap<QString,QSharedPointer<LicosBlock> >
  LicosProcessBlock::buildBlocks(void)
  {
    typedef QSharedPointer<LicosBlock> BPtr;
    QMap<QString,BPtr> b;
    return b;
  } // end of LicosProcessBlock::buildBlocks

  const QMap<QString,QSharedPointer<LicosBlock> >&
  LicosProcessBlock::getBlocks(void)
  {
    typedef QSharedPointer<LicosBlock> BPtr;
    static QMap<QString,BPtr> b(buildBlocks());
    return b;
  }

  const QStringList&
  LicosProcessBlock::getKeys()
  {
    static QStringList l(LicosProcessBlock::buildKeys());
    return l;
  }

  QStringList
  LicosProcessBlock::buildKeys()
  {
    QStringList k;
    k << "Mesh"
      << "CastemMemoryManagementPolicy"
      << "CastemMemorySize"
      << "CastemMeshDensity" 
      << "CastemSwapSize"
      << "ExternalLibrary"
      << "ModellingHypothesis"
      << "OutOfBoundsPolicy"
      << "VerboseMode"
      << "WarningMode"
      << "Global";
    return k;
  }

  LicosMainBlock::LicosMainBlock()
    : StandardLicosBlock("main")
  {
    typedef GlobalCouplingItemBlock LGCIB;
    this->addKeys(QStringList()
		  << "CastemNames"
		  << "Date"
		  << "Dependencies"
		  << "InputFile"
		  << "GlobalCurve"
		  << "GlobalLoadingEvolutionFromTextFile"
		  << "PostProcessingTimes"
		  << "Restore"
		  << "Save" 
		  << "StarterTask"
		  << "SubMeshDispatch"
		  << "SubMeshNames"
		  << "SubMeshesNames"
		  << "TerminateTask"
		  << "TimeStepController"
		  << "Times"
		  << LicosSpecificSectionBlock::getKeys()
		  << LicosProcessBlock::getKeys());
    this->addBlock(new LicosSpecificSectionBlock);
    this->addBlocks(LicosSpecificSectionBlock::getBlocks());
    this->addBlock(new LicosProcessBlock);
    this->addBlocks(LicosProcessBlock::getBlocks());
    this->addBlock(new GlobalLoadingEvolutionBlock);
    this->addBlock(new GlobalCouplingBlock);
    this->addBlock(new LGCIB("GlobalModel"));
    this->addBlock(new LGCIB("GlobalAuxiliaryTask"));
    this->addBlock(new LGCIB("GlobalCriterium"));
    this->addBlock(new LGCIB("GlobalPostProcessing"));
    this->addBlock(new LicosGlobalGaseousMaterialBlock);
    this->addBlock(new SubMeshesDataBlock);
    this->addBlock("Study",
		   QStringList() << "Array");
    this->addBlock("Plenum",
		   QStringList() << "TemperatureEvolution"
		   << "Volume" << "InitialPressure" 
		   << "ChemicalComposition");
    this->addBlock("GlobalGasGroup",
		   QStringList() << "Gases"
		   << "Plenums");
  } // end of LicosMainBlock::LicosMainBlock

} // end of namespace qemacs
