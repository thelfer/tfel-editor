/*! 
 * \file  LicosSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QRegExp>

#include"QEmacs/CxxTokenizer.hxx"
#include"QEmacs/LicosMajorMode.hxx"
#include"QEmacs/LicosSyntaxHighlighter.hxx"

namespace qemacs
{

  LicosData::~LicosData()
  {} // end of LicosData::~LicosData
  
  LicosSyntaxHighlighter::LicosSyntaxHighlighter(QTextDocument *p)
    : CSyntaxHighlighterBase(p)
  {
    this->cCharAsString = true;
    this->keyFormat.setForeground(Qt::blue);
    this->optFormat.setForeground(Qt::darkCyan);
    this->importFormat = this->quotationFormat;
    this->importFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    this->importFormat.setToolTip("open in new buffer");
    QStringList keys   = LicosSyntaxHighlighter::getKeys();
    QStringList blocks = LicosSyntaxHighlighter::getBlocks();
    
    foreach (const QString &k,keys) {
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->keyFormat;
      highlightingRules.append(rule);
    }

    foreach (const QString &k,blocks) {
      HighlightingRule rule;
      rule.format  = this->keyFormat;
      rule.key     = k;
      highlightingRules.append(rule);
      rule.key      = "EndOf"+k;
      highlightingRules.append(rule);
    }

    // override C highligthing rules for links to other files
    const QStringList exts = LicosMajorMode::getLicosExtensionsSuffix();
    foreach(const QString& e,exts){
      this->importExprs.push_back(QRegExp("'([\\w\\./-]+\\."+e+")'"));
      this->importExprs.push_back(QRegExp("\"([\\w\\./-]+\\."+e+")\""));
    }

  }

  void
  LicosSyntaxHighlighter::highlightBlock(const QString & text)
  {
    using namespace std;
    CxxTokenizer t;
    CxxTokenizer::const_iterator p;
    CxxTokenizer::const_iterator pe;
    t.treatCharAsString(this->cCharAsString);
    t.setCCommentOpened(this->previousBlockState()==1);
    try{
      t.parseString(text);
    } catch(exception&){}
    // treating the first word of line
    p=t.begin();
    if(p!=t.end()){
      if(p->flag==Token::Standard){
	const QString& k = p->value;
	for(int i=0;i!=highlightingRules.size();++i){
	  if(highlightingRules[i].key==k){
	    this->setFormat(p->pos,p->value.size(),
			    highlightingRules[i].format);
	    break;
	  }
	}
	++p;
      }
    }
    // treating options;
    if((p!=t.end())&&(p->flag==Token::Standard)&&(p->value=="<")){
      this->setFormat(p->pos,p->value.size(),
		      this->optFormat);
      ++p;
      while((p!=t.end())&&(p->value!=">")&&(p->flag==Token::Standard)){
	this->setFormat(p->pos,p->value.size(),
			this->optFormat);
	++p;
      }
      if((p!=t.end())&&(p->value==">")){
	this->setFormat(p->pos,p->value.size(),
			this->optFormat);
	++p;
      }
    }
    for(;p!=t.end();++p){
      QTextCharFormat f;
      bool b = false;
      if((p->flag==Token::String)||
	 (p->flag==Token::Char)){
	for(int i=0;(i!=this->importExprs.size())&&(!b);++i){
	  if(this->importExprs[i].indexIn(p->value)!=-1){
	    f = this->importFormat;
	    b = true;
	  }
	}
	if(!b){
	  f = this->quotationFormat;
	  b = true;
	}
      } else if (p->flag==Token::Number){
	f = this->numberFormat;
	b = true;
      } else if (CxxTokenizer::isComment(*p)){
	f = this->commentFormat;
	b = true;
      }
      if(b){
	if((p->flag==Token::String)||
	   (p->flag==Token::Char)){
	  this->setFormat(p->pos+1,p->value.size()-2,f);
	} else {
	  this->setFormat(p->pos,p->value.size(),f);
	}
      }
    }
    // special treatment for castem material properties
    p=t.begin();
    if((t.size()>=7)&&
       (p->value=="MaterialProperty")&&((p+1)->value=="<")&&
       ((p+2)->value=="castem")&&((p+3)->value==">")&&
       ((p+4)->flag==Token::String)&& 
       ((p+5)->flag==Token::String)&& 
       ((p+6)->flag==Token::String)){
      LicosData *d = new LicosData;
      d->library  = (p+5)->value.mid(1,(p+5)->value.size()-2);
      d->function = (p+6)->value.mid(1,(p+6)->value.size()-2);
      d->pos      = (p+6)->pos+1;
      this->setCurrentBlockUserData(d);
      this->setFormat((p+6)->pos+1,(p+6)->value.size()-2,
		      this->importFormat);
    }
    // special treatment for umat mechanical behaviours
    p=t.begin();
    if((t.size()==7)&&
       (p->value=="MechanicalBehaviour")&&((p+1)->value=="<")&&
       (((p+2)->value=="Isotropic")||((p+2)->value=="Orthotropic"))
       &&((p+3)->value==">")&&
       ((p+4)->flag==Token::String)&& 
       ((p+5)->flag==Token::String)&& 
       ((p+6)->flag==Token::String)){
      LicosData *d = new LicosData;
      d->library  = (p+5)->value.mid(1,(p+5)->value.size()-2);
      d->function = (p+6)->value.mid(1,(p+6)->value.size()-2);
      d->pos      = (p+6)->pos+1;
      this->setCurrentBlockUserData(d);
      this->setFormat((p+6)->pos+1,(p+6)->value.size()-2,
		      this->importFormat);
    }
    p=t.begin();
    if((t.size()==4)&&
       (p->value=="MechanicalBehaviour")&&
       ((p+1)->flag==Token::String)&& 
       ((p+2)->flag==Token::String)&& 
       ((p+3)->flag==Token::String)){
      LicosData *d = new LicosData;
      d->library  = (p+2)->value.mid(1,(p+2)->value.size()-2);
      d->function = (p+3)->value.mid(1,(p+3)->value.size()-2);
      d->pos      = (p+3)->pos+1;
      this->setCurrentBlockUserData(d);
      this->setFormat((p+3)->pos+1,(p+3)->value.size()-2,
		      this->importFormat);
    }
    if(t.isCCommentOpened()){
      this->setCurrentBlockState(1);
    } else {
      this->setCurrentBlockState(0);
    }
  } // end of LicosSyntaxHighlighter::highlightBlock

  const QStringList&
  LicosSyntaxHighlighter::getKeys(void)
  {
    static QStringList keys(LicosSyntaxHighlighter::buildKeysList());
    return keys;
  } // end of LicosSyntaxHighlighter::getKeys

  const QStringList&
  LicosSyntaxHighlighter::getBlocks(void)
  {
    static QStringList blocks(LicosSyntaxHighlighter::buildBlocksList());
    return blocks;
  } // end of LicosSyntaxHighlighter::getBlocks

  QStringList
  LicosSyntaxHighlighter::buildKeysList(void)
  {
    QStringList keys;
    keys << "Mesh"
	 << "TL" << "TBC"
	 << "ML" << "MBC" << "MPC" << "MPP"
	 << "CastemGraphicalOutput"
	 << "CastemMemoryManagementPolicy"
	 << "CastemMemorySize"
	 << "CastemMeshDensity" 
	 << "CastemNames"
	 << "CastemSwapSize"
	 << "Date" << "Function"
	 << "Dependencies"
	 << "Event"
	 << "ExportToMED"
	 << "ExportToTxt"
	 << "ExportToVTK"
	 << "ExternalLibrary"
	 << "GaseousMaterialDescription"
	 << "GeneralisedPlaneStrainReferencePoints"
	 << "Global"
	 << "GlobalCriterium"
	 << "GlobalCurve"
	 << "GlobalLoadingEvolutionFromTextFile"
	 << "GlobalModel"
	 << "GlobalPostProcessing"
	 << "Import"
	 << "ImportExternalData"
	 << "LoadingEvolutionFromTextFile"
	 << "MPScript"
	 << "MPScriptII"
	 << "MPScriptIII"
	 << "MPScriptIV"
	 << "Materials"
	 << "MechanicalMaterials"
	 << "MechanicalPostComputation"
	 << "MechanicalPostProcessingScript"
	 << "MechanicalPostProcessingScriptII"
	 << "MechanicalPostProcessingScriptIII"
	 << "MechanicalPostProcessingScriptIV"
	 << "ModellingHypothesis"
	 << "OutOfBoundsPolicy"
	 << "ParserFunction"
	 << "PostProcessingTimes"
	 << "Restore"
	 << "Save" 
	 << "ScalarValueTransformation"
	 << "StarterTask"
	 << "SubMeshDispatch"
	 << "SubMeshNames"
	 << "SubMeshesNames"
	 << "TPScript"
	 << "TPScriptII"
	 << "TPScriptIII"
	 << "TPScriptIV"
	 << "TerminateTask"
	 << "ThermalMaterials"
	 << "ThermalPostProcessingScript"
	 << "ThermalPostProcessingScriptII"
	 << "ThermalPostProcessingScriptIII"
	 << "ThermalPostProcessingScriptIV"
	 << "TimeStepController"
	 << "Times"
	 << "AnisotropicAxises"
	 << "UniformLoadingFromEvolution"
	 << "VerboseMode"
	 << "WarningMode"
	 << "Block"
	 << "MaterialProperty"
	 << "ChemicalSpecies"
	 << "InternalStateVariable"
	 << "IntegrationScheme"
	 << "ActivatingEvent"
	 << "DesactivatingEvent"
	 << "ActivatingEvents"
	 << "DesactivatingEvents"
	 << "Active"
	 << "StateVariable"
	 << "Array"
	 << "Bool"
	 << "Dictionary"
	 << "Int"
	 << "Map"
	 << "Real"
	 << "Double"
	 << "String"
	 << "Curve"
	 << "Field"
	 << "Evolution"
	 << "LineCurve";
    return keys;
  }

  QStringList
  LicosSyntaxHighlighter::buildBlocksList(void)
  {
    QStringList keys;
    keys << "Process"
	 << "Study"
	 << "MeshStudy"
	 << "GaseousMaterial"
	 << "GasGroup"
	 << "GlobalCoupling"
	 << "GlobalGasGroup"
	 << "GlobalGaseousMaterial"
	 << "GlobalLoadingEvolution"
	 << "Loading"
	 << "Material"
	 << "MaterialDescription"
	 << "MultipleCurves"
	 << "MultipleLineCurves"
	 << "Plenum"
	 << "Plot"
	 << "RamsesStudy"
	 << "SpecificSection"
	 << "StressProfile"
	 << "StressInterpolationPolicy"
	 << "SubMeshesData"
	 << "MechanicalBoundaryCondition"
	 << "MechanicalBoundaryConditions"
	 << "MechanicalLoadings"
	 << "MechanicalLoading"
	 << "MechanicalPostProcessing"
	 << "MechanicalPostComputations"
	 << "ThermalLoading"
	 << "ThermalBoundaryCondition"
	 << "ThermalBoundaryConditions"
	 << "ThermalLoadings"
	 << "ThermalBehaviour"
	 << "MechanicalBehaviour"
	 << "AuxiliaryTask"
	 << "Coupling"
	 << "Criterium"
	 << "PostProcessing"
	 << "Model"
      	 << "SpecializedAuxiliaryTask"
      	 << "SpecialisedAuxiliaryTask"
	 << "SpecializedModel"
	 << "SpecialisedModel"
	 << "SpecializedCriterium"
	 << "SpecialisedCriterium"
	 << "SpecializedPostProcessing"
	 << "SpecialisedPostProcessing"
	 << "LoadingEvolution"
	 << "LocalResolution"
	 << "InputFile"
	 << "SpecializedTimeStepController";
    return keys;
  } // end of LicosSyntaxHighlighter::buildBlocksList

} // end of namespace qemacs

