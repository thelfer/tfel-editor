/*! 
 * \file  LicosSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \brief  03/07/2012
 */

#include<QtCore/QDebug>
#include<QtCore/QRegExp>
#include"TFEL/Utilities/CxxTokenizer.hxx"
#include"QEmacs/LicosMajorMode.hxx"
#include"QEmacs/LicosSyntaxHighlighter.hxx"

namespace qemacs
{

  LicosData::~LicosData() = default;
  
  LicosSyntaxHighlighter::LicosSyntaxHighlighter(QTextDocument *p)
    : CSyntaxHighlighterBase(p)
  {
    // parsing options
    this->options.charAsString = true;
    this->options.bKeepCommentBoundaries = true;
    // highlighting
    this->keyFormat.setForeground(Qt::blue);
    this->optFormat.setForeground(Qt::darkCyan);
    this->importFormat = this->quotationFormat;
    this->importFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    this->importFormat.setToolTip("open in new buffer");
    const auto keys   = LicosSyntaxHighlighter::getKeys();
    const auto blocks = LicosSyntaxHighlighter::getBlocks();
    
    for(const std::string &k : keys) {
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->keyFormat;
      highlightingRules.push_back(rule);
    }
    for(const std::string &k : blocks) {
      HighlightingRule rule;
      rule.format  = this->keyFormat;
      rule.key     = k;
      highlightingRules.push_back(rule);
      rule.key      = "EndOf"+k;
      highlightingRules.push_back(rule);
    }
    // override C highligthing rules for links to other files
    const auto exts = LicosMajorMode::getLicosExtensionsSuffix();
    for(const QString& e : exts){
      this->importExprs.push_back(QRegExp("'([\\w\\./-]+\\."+e+")'"));
      this->importExprs.push_back(QRegExp("\"([\\w\\./-]+\\."+e+")\""));
    }

  }

  void LicosSyntaxHighlighter::highlightBlock(const QString & text)
  {
    using tfel::utilities::CxxTokenizer;
    using tfel::utilities::Token;
    CxxTokenizer t(this->options);
    t.setCStyleCommentOpened(this->previousBlockState()==1);
    try{
      t.parseString(text.toStdString());
    } catch(std::exception&){}
    // treating the first word of line
    auto p=t.begin();
    if(p!=t.end()){
      if(p->flag==Token::Standard){
	for(int i=0;i!=highlightingRules.size();++i){
	  if(highlightingRules[i].key==p->value){
	    this->setFormat(p->offset,p->value.size(),
			    highlightingRules[i].format);
	    break;
	  }
	}
	++p;
      }
    }
    // treating options;
    if((p!=t.end())&&(p->flag==Token::Standard)&&(p->value=="<")){
      this->setFormat(p->offset,p->value.size(),
		      this->optFormat);
      ++p;
      while((p!=t.end())&&(p->value!=">")&&(p->flag==Token::Standard)){
	this->setFormat(p->offset,p->value.size(),
			this->optFormat);
	++p;
      }
      if((p!=t.end())&&(p->value==">")){
	this->setFormat(p->offset,p->value.size(),
			this->optFormat);
	++p;
      }
    }
    for(;p!=t.end();++p){
      QTextCharFormat f;
      bool b = false;
      if((p->flag==Token::String)||(p->flag==Token::Char)){
	for(int i=0;(i!=this->importExprs.size())&&(!b);++i){
	  if(this->importExprs[i].indexIn(QString::fromStdString(p->value))!=-1){
	    f = this->importFormat;
	    b = true;
	  }
	}
	if(!b){
	  f = this->quotationFormat;
	  b = true;
	}
	// } else if (p->flag==Token::Number){
	// 	f = this->numberFormat;
	// 	b = true;
      } else if (tfel::utilities::isComment(*p)){
      	f = this->commentFormat;
      	b = true;
      }
      if(b){
	if((p->flag==Token::String)||(p->flag==Token::Char)){
	  this->setFormat(p->offset+1,p->value.size()-2,f);
	} else {
	  this->setFormat(p->offset,p->value.size(),f);
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
      auto *d = new LicosData;
      d->library  = QString::fromStdString((p+5)->value.substr(1,(p+5)->value.size()-2));
      d->function = QString::fromStdString((p+6)->value.substr(1,(p+6)->value.size()-2));
      d->pos      = (p+6)->offset+1;
      this->setCurrentBlockUserData(d);
      this->setFormat((p+6)->offset+1,(p+6)->value.size()-2,
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
      auto *d = new LicosData;
      d->library  = QString::fromStdString((p+5)->value.substr(1,(p+5)->value.size()-2));
      d->function = QString::fromStdString((p+6)->value.substr(1,(p+6)->value.size()-2));
      d->pos      = (p+6)->offset+1;
      this->setCurrentBlockUserData(d);
      this->setFormat((p+6)->offset+1,(p+6)->value.size()-2,
		      this->importFormat);
    }
    p=t.begin();
    if((t.size()==4)&&
       (p->value=="MechanicalBehaviour")&&
       ((p+1)->flag==Token::String)&& 
       ((p+2)->flag==Token::String)&& 
       ((p+3)->flag==Token::String)){
      auto *d = new LicosData;
      d->library  = QString::fromStdString((p+2)->value.substr(1,(p+2)->value.size()-2));
      d->function = QString::fromStdString((p+3)->value.substr(1,(p+3)->value.size()-2));
      d->pos      = (p+3)->offset+1;
      this->setCurrentBlockUserData(d);
      this->setFormat((p+3)->offset+1,(p+3)->value.size()-2,
		      this->importFormat);
    }
    if(t.isCStyleCommentOpened()){
      this->setCurrentBlockState(1);
    } else {
      this->setCurrentBlockState(0);
    }
  } // end of LicosSyntaxHighlighter::highlightBlock

  const std::vector<std::string>& LicosSyntaxHighlighter::getKeys()
  {
    static const auto keys = LicosSyntaxHighlighter::buildKeysList();
    return keys;
  } // end of LicosSyntaxHighlighter::getKeys

  const std::vector<std::string>& LicosSyntaxHighlighter::getBlocks()
  {
    static const auto blocks = LicosSyntaxHighlighter::buildBlocksList();
    return blocks;
  } // end of LicosSyntaxHighlighter::getBlocks

  std::vector<std::string> LicosSyntaxHighlighter::buildKeysList()
  {
    return {"Mesh","TL","TBC","ML","MBC","MPC","MPP",
	"CastemGraphicalOutput","CastemMemoryManagementPolicy",
	"CastemMemorySize","CastemMeshDensity" ,"CastemNames",
	"CastemSwapSize","Date","Function","Dependencies",
	"Event","ExportToMED","ExportToTxt","ExportToVTK",
	"ExternalLibrary","GaseousMaterialDescription",
	"GeneralisedPlaneStrainReferencePoints",
	"Global","GlobalCriterium","GlobalCurve",
	"GlobalLoadingEvolutionFromTextFile",
	"GlobalModel","GlobalPostProcessing",
	"Import","ImportExternalData",
	"LoadingEvolutionFromTextFile","MPScript",
	"MPScriptII","MPScriptIII","MPScriptIV",
	"Materials","MechanicalMaterials",
	"MechanicalPostComputation",
	"MechanicalPostProcessingScript",
	"MechanicalPostProcessingScriptII",
	"MechanicalPostProcessingScriptIII",
	"MechanicalPostProcessingScriptIV",
	"ModellingHypothesis","OutOfBoundsPolicy",
	"ParserFunction","PostProcessingTimes",
	"Restore","Save" ,"ScalarValueTransformation",
	"StarterTask","SubMeshDispatch","SubMeshNames",
	"SubMeshesNames","TPScript","TPScriptII","TPScriptIII",
	"TPScriptIV","TerminateTask","ThermalMaterials",
	"ThermalPostProcessingScript","ThermalPostProcessingScriptII",
	"ThermalPostProcessingScriptIII","ThermalPostProcessingScriptIV",
	"TimeStepController",
	"Times","AnisotropicAxises","UniformLoadingFromEvolution",
	"VerboseMode","WarningMode","Block","MaterialProperty",
	"ChemicalSpecies","InternalStateVariable","IntegrationScheme",
	"ActivatingEvent","DesactivatingEvent","ActivatingEvents",
	"DesactivatingEvents","Active","StateVariable",
	"Array","Bool","Dictionary",
	"Int","Map","Real","Double","String",
	"Curve","Field","Evolution","LineCurve"};
  }

  std::vector<std::string> LicosSyntaxHighlighter::buildBlocksList(){
    return {"Process","Study","MeshStudy","GaseousMaterial",
	"GasGroup","GlobalCoupling","GlobalGasGroup",
	"GlobalGaseousMaterial","GlobalLoadingEvolution",
	"Loading","Material","MaterialDescription","MultipleCurves",
	"MultipleLineCurves","Plenum","Plot","RamsesStudy",
	"SpecificSection","StressProfile","StressInterpolationPolicy",
	"SubMeshesData","MechanicalBoundaryCondition",
	"MechanicalBoundaryConditions","MechanicalLoadings",
	"MechanicalLoading","MechanicalPostProcessing",
	"MechanicalPostComputations","ThermalLoading",
	"ThermalBoundaryCondition","ThermalBoundaryConditions",
	"ThermalLoadings","ThermalBehaviour","MechanicalBehaviour",
	"AuxiliaryTask","Coupling","Criterium","PostProcessing",
	"Model","SpecializedAuxiliaryTask","SpecialisedAuxiliaryTask",
	"SpecializedModel","SpecialisedModel","SpecializedCriterium",
	 "SpecialisedCriterium","SpecializedPostProcessing",
	"SpecialisedPostProcessing","LoadingEvolution",
	"LocalResolution","InputFile","SpecializedTimeStepController"};
  } // end of LicosSyntaxHighlighter::buildBlocksList

} // end of namespace qemacs

