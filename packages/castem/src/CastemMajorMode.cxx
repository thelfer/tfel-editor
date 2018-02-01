/*! 
 * \file  CastemMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 août 2012
 */

#include<QtCore/QDir>
#include<QtCore/QUrl>
#include<QtCore/QFileInfo>
#include<QtCore/QTemporaryFile>
#include<QtGui/QSyntaxHighlighter>
#include<QtGui/QDesktopServices>
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/CastemMajorMode.hxx"
#include"QEmacs/CastemSyntaxHighlighter.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{
  
  QStringList CastemMajorMode::buildKeysList()
  {
    return {"DROITE" , "MOT"    , "OPTION", "FIN",
	"DENSITE", "LAPLACE", "CERCLE", "COTE",
	"QUELCONQUE" , "INTERSECTION" ,
	"PARABOLE"   , "ET", "POINT",
	"PLUS" , "MOINS" , "TRANSLATION",
	"ROTATION" , "TRACER" , "INVERSE" ,
	"ELEMENT" , "CONTOUR" , "DIFF" , "CHANGER",
	"SURFACE" , "CONFONDRE" , "INFO" , "TOURNER",
	"HOMOTHETIE" , "AFFINITE" , "SYMETRIE" , "INCLUSION",
	"TITRE" , "RACCORD" , "TASSER" , "SORTIR",
	"LIRE" , "BARYCENTRE" , "DALLER" , "ORIENTER",
	"OUBLIER" , "COMPORTEMENT" , "COUTURE" , "PAVE",
	"COMMENTAIRE" , "NOEUD" , "MOT"  , "NBEL",
	"NOTICE" , "FACE" , "COORDONNEE" , "NORME",
	"TEMPS" , "VOLUME" , "LECT" , "SAUF",
	"DEPLACER" , "PSCAL" , "PVEC" , "PMIX",
	"LIAISON" , "REGLER" , "HOOKE" , "SOLS",
	"AFFECTE" , "RIGIDITE" , "BLOQUE" , "DEPIMPOSE",
	"HOTANGE" , "STRUCTURE" , "TEXTE" , "PROJETER",
	"ELST" , "JONC" , "RECO" , "MASSE",
	"CLST" , "SIGMA" , "RELATION" , "FORCE",
	"PRCH" , "BASE" , "DIMENSION" , "EXTRAIRE",
	"VERSENS" , "VIBRATION" , "MAXIMUM" , "XTMX",
	">EG"  , "<EG"  , "OU"   , "EGA",
	"NON"  , "NEG"  , "MULT" , "FLOTTANT",
	"PJBA" , "CRITERE" , "DIAGNEG" , "XTX" ,
	"CHOC" , "BSIGMA" , "PRHC" , "MAX1",
	"REPETER" , "QUITTER" , "SI"   , "SINO",
	"FINSI" , "IPOL" , "ABS"  , "SIN" ,
	"ATG"  , "ENVELOPPE" , "ISOV" , "DETRUIRE",
	"ENLEVER" , "REMPLACER" , "INSERER" , "COLI",
	"TABLE" , "REDUCTION" , "SYMT" , "ANTISYMETRIE",
	"RESULT" , "PRES" , "EXCO" , "NOMC",
	"DEFORME" , "APPUI" , "INVA" , "PRIN",
	"VMIS" , "KSIGMA" , "SIGN" , "SUITE",
	"VALPROPRE" , "ORDONNER" , "TIRE" , "REGENERER",
	"DESSIN" , "AMOR" , "CHARGEMENT" , "COULEUR",
	"AFCO" , "EVOLUTION" , "ORTHOGONALISER" , "THETA",
	"COMBTABLE" , "DEVERSOIR" , "VECTEUR" , "PICA",
	"COPIER" , "DIMNOYAU" , "SAUVER" , "RESTITUER",
	"CARACTERISTIQUE" , "MATERIAU" , "GENERATRICE" , "LOG" ,
	"CAPACITE" , "ELFE" , "JACOBIEN" , "PLAS",
	"GREEN" , "MODELISER" , "PROCEDURE" , "FINPROC",
	"DEBPROC" , "KTAN" , "FORME" , "MESSAGE",
	"NNOR" , "CUBP" , "CUBT" , "CER3",
	"SEISME" , "ENER" , "EPSI" , "INTG",
	"COURBE" , "REACTION" , "SUPER" , "ZERO",
	"EXCITER" , "KP"   , "ACTIVE" , "ELASTICITE",
	"ERREUR" , "CONGE" , "LUMP" , "OBTENIR",
	"VARI" , "MODIFIER" , "MASQUE" , "EXISTE",
	"MINIMUM" , "GRADIENT" , "ENSEMBLE" , "IFRE",
	"SIGSOL" , "MAPP" , "SOMME" , "BRUIT",
	"RTENS" , "DSPR" , "TFR"  , "TOTEMP",
	"GRAF" , "TRESCA" , "TYPE" , "OSCI",
	"SPO"  , "INDEX" , "CHSP" , "DFOURIER",
	"TAGR" , "PERMEABILITE" , "CABLE" , "FOFI",
	"WORK" , "QULX" , "DEBIT" , "ARTICULATION",
	"CMOY" , "COMT" , "CONDUCTIVITE" , "FLUX",
	"MODL" , "MATR" , "RIMP" , "FILTRE",
	"CONCAT" , "ITERER" , "ACQUERIR" , "SOURCE",
	"CONVECTION" , "PSMO" , "ECOULE" , "INSI",
	"MENAGE" , "SYNTHESE" , "ARGUMENT" , "GLIN",
	"DYNE" , "FONCTION" , "RESPRO" , "PLACE",
	"VALEUR" , "PROI" , "EXCELLENCE" , "ARETE",
	"CALP" , "INDI" , "ACT3" , "BIOT",
	"DEDU" , "CONNECTIVITE" , "NLOC" , "CHAINE",
	"COSI" , "CVOL" , "DIAD" , "HANN",
	"LSQF" , "LTL"  , "PERT" , "PRNS",
	"PSRS" , "SIAR" , "SPON" , "VISAVIS",
	"CCON" , "MESURE" , "PILEPS" , "SIMPLEX",
	"UTILISATEUR" , "MENU" , "COSH" , "SINH",
	"DEG3" , "AIDE" , "RACPOL" , "REFE",
	"KSOF" , "NSKE" , "KMAB" , "CNEQ",
	"NOEL" , "DOMA" , "FPU"  , "GMV" ,
	"EQPR" , "EQEX" , "VIBC" , "AVCT",
	"KDIA" , "KMTP" , "KMF"  , "MDIA",
	"DFDT" , "TCRR" , "TCNM" , "SQTP",
	"NLIN" , "CMCT" , "KCHT" , "LAPN",
	"RAFT" , "KLOP" , "KRES" , "CSON",
	"NUAGE" , "WEIP" , "KHIS" , "KOPS",
	"FSUR" , "FLAM" , "ELNO" , "TANH",
	"DBIT" , "NS"   , "TOIM" , "FIMP",
	"KMBT" , "KBBT" , "DUDW" , "FROT",
	"KONV" , "KCHA" , "MHYB" , "MATP",
	"HDEB" , "HVIT" , "HYBP" , "SMTP",
	"MOCU" , "CHAUSSETTE" , "TAILLE" , "ERF" ,
	"SENSIBILITE" , "IMPOSE" , "DANS" , "IMPF",
	"FRON" , "FUIT" , "EPTH" , "FPT" ,
	"KFPT" , "FPA"  , "KFPA" , "ECHI",
	"KPRO" , "FFOR" , "RAYE" , "RAYN",
	"VSUR" , "TRAJ" , "AJU1" , "AJU2",
	"EXCF" , "PREC" , "ONDE" , "CFL" ,
	"DEDOUBLE" , "DCOV" , "PARC" , "FRIG",
	"POLA" , "CHI1" , "CHI2" , "PENT",
	"PRET" , "METH" , "XXT"  , "CBLO",
	"ZLEG" , "MESM" , "FION" , "NEUT",
	"LOGK" , "COAC" , "RESISTANCE" , "MUTU",
	"DIRI" , "LIGN" , "OBJET" , "DEBMETH",
	"FINM" , "HERITE" , "DECO" , "EXTE",
	"DMTD" , "BMTD" , "SSCH" , "MREM",
	"ASSISTANT" , "FISS" , "PRIMITIVE" , "ANNU",
	"SAIS" , "CHOI" , "DETO" , "PARTITION",
	"CLMI" , "PMAT" , "EXCP" , "PROPHASE",
	"ALEA" , "GNFL" , "MPRO" , "SSTE",
	"ADVE" , "BGMO" , "ECFE" , "COUPER",
	"DFER" , "GYROSCOPIQUE" , "CORIOLIS" , "KENT",
	"FANTOME" , "ITRC" , "RETO" , "IJET",
	"MOCA" , "LEVM" , "RAVC" , "IDLI",
	"RAFF" , "CFND" , "ADET" , "PSIP",
	"ASIN" , "TAN"  , "TRIE" , "GANE",
	"HIST" , "ETG"  , "OTER" , "XFEM",
	"MOTA" , "LIST" , "ELIM" , "MANU",
	"NBNO" , "PROG" , "RESO" , "VENV",
	"MOME" , "YTMX" , "ENTI" , "MOTS",
	"COS"  , "TRIA" , "SAUT" , "CARB",
	"CHPO" , "CAPI" , "EXP"  , "XTY" ,
	"FDT " , "DEPB" , "TFRI" , "SOMT",
	"FIMP" , "TSCA" , "DIVU" , "NTAB",
	"QOND" , "GENJ" , "SORE" , "DMMU",
	"PROB" , "PHAJ" , "VERM" , "IMPE",
	"ACOS" , "MEPL"};
  } // end of buildKeysList

  const QStringList& CastemMajorMode::getKeysList()
  {
    static QStringList keys(buildKeysList());
    return keys;
  } // end of getKeysList

  struct CastemMajorModeRessourceLoader
  {
    CastemMajorModeRessourceLoader()
      : l(getLoader())
    {}
  private:
    struct Loader
    {
      Loader()
      {
	Q_INIT_RESOURCE(CastemModeResources);
      }
    };
    static Loader& getLoader()
    {
      static Loader l;
      return l;
    }
    Loader l;
  };
  
  CastemMajorMode::CastemMajorMode(QEmacsWidget& w,
				   QEmacsBuffer& b,
				   QEmacsTextEditBase& t)
    : QEmacsMajorModeBase(w,b,t,&t),
      co(nullptr),
      ha1(nullptr),
      ha2(nullptr)
  {
    this->c = new QCompleter(CastemMajorMode::getKeysList(),&t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,static_cast<void (QCompleter:: *)(const QString&)>(&QCompleter::activated),
		     &t,&QEmacsTextEditBase::insertCompletion);
  } // end of CastemMajorMode
  
  void CastemMajorMode::sendToCastem(const QString& l)
  {
    this->startCastem();
    if(this->co==nullptr){
      return;
    }
    auto& p = this->co->getProcess();
    if(p.state()!=QProcess::Running){
      return;
    }
    p.write((l+'\n').toLatin1());
  } // end of CastemMajorMode::sendToCastem

  void CastemMajorMode::startCastem(){
    if(this->co==nullptr){
      this->co = new ProcessInteractionFrame(this->qemacs,this->buffer);
      QFileInfo fn(this->textEdit.getCompleteFileName());
      QDir d(fn.dir());
      QProcess& p = co->getProcess();
      if(d.exists()){
	p.setWorkingDirectory(d.absolutePath());
      } else {
	p.setWorkingDirectory(QDir::current().absolutePath());
      }
      this->buffer.addSlave("* castem *",co);
    }
    QProcess& p = co->getProcess();
    if(p.state()!=QProcess::Running){
      p.start("castem2014_PLEIADES",QStringList());
      p.waitForStarted();
    }
  }

  QString CastemMajorMode::getName() const{
    return "Cast3M";
  }

  QString CastemMajorMode::getDescription() const{
    return "major mode dedicated to the "
      "castem finite element solver";
  }
  
  void CastemMajorMode::setSyntaxHighlighter(QTextDocument* d){
    new CastemSyntaxHighlighter(d);
  }
  
  QCompleter* CastemMajorMode::getCompleter(){
    return this->c;
  } // end of getCompleter
  
  void CastemMajorMode::completeCurrentWord(QEmacsTextEditBase& t,
					    const QString& w){
    QTextCursor tc = t.textCursor();
    tc.movePosition(QTextCursor::StartOfWord,
		    QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::EndOfWord,
		    QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    tc.insertText("'"+w.toUpper()+"' ");
    t.setTextCursor(tc);
  } // end of completeCurrentWord
  
  bool CastemMajorMode::isCastemKeyWord(const QString& w){
    const QStringList& keys = CastemMajorMode::getKeysList();
    foreach(const QString& k,keys){
      if(k.left(4)==w){
	return true;
      }
    }
    return false;
  }

  void CastemMajorMode::indentLine(const QTextCursor& c_){
    QTextCursor tc(c_);
    tc.beginEditBlock();
    tc.movePosition(QTextCursor::StartOfBlock,
		    QTextCursor::MoveAnchor);
    tc.insertText("  ");
    tc.endEditBlock();
  } // end of CastemMajorMode::indentLine

  bool CastemMajorMode::handleShortCut(const int k1,
				       const Qt::KeyboardModifiers m,
				       const int k2)
  {
    if((k1==Qt::Key_X)&&(m==Qt::NoModifier)&&(k2==Qt::Key_H)){
      const QString& w  = this->textEdit.getCurrentWord();
      const QString& w2 = w.left(4);
      if(this->isCastemKeyWord(w2)){
	this->displayHelp(w,w2);
	return true;
      }
    } else if((k1==Qt::Key_C)&&(k2==Qt::Key_L)&&
	      (m==Qt::ControlModifier)){
      QTextCursor tc = this->textEdit.textCursor();
      tc.select(QTextCursor::LineUnderCursor);
      this->sendToCastem(tc.selectedText());
      return true;
    }
    return false;
  } // end of CastemMajorMode::handleShortCut

  void CastemMajorMode::completeContextMenu(QMenu *const m,
					    const QTextCursor& tc)
  {
    QEmacsMajorModeBase::completeContextMenu(m,tc);
    QTextCursor mtc(tc);
    mtc.select(QTextCursor::WordUnderCursor);
    const QString w = mtc.selectedText();
    if(this->isCastemKeyWord(w.left(4))){
      delete this->ha1;
      delete this->ha2;
      this->ha1=new QAction(QObject::tr("Help on %1 (notice)").arg(w),this);
      this->ha1->setData(w);
      this->ha2=new QAction(QObject::tr("Help on %1 (web)").arg(w),this);
      this->ha2->setData(w);
      const auto& cactions = m->actions();	
      if(cactions.isEmpty()){
	m->addAction(this->ha1);
	m->addAction(this->ha2);
      } else {
	m->insertAction(*(cactions.begin()),this->ha1);
	m->insertAction(*(cactions.begin()),this->ha2);
      }
      QObject::connect(m,&QMenu::triggered,
		       this,&CastemMajorMode::actionTriggered);
    }
  } // end of CastemMajorMode::completeContextMenu

  void CastemMajorMode::actionTriggered(QAction *a)
  {
    if(a==this->ha1){
      const auto w = this->ha1->data().toString();
      this->displayHelp(w,w.left(4));
    } else if(a==this->ha2){
      const auto w = this->ha2->data().toString();
      this->openWebHelp(w.left(4));
    }
  }

  void CastemMajorMode::displayHelp(const QString& w,
				    const QString& w2){
    // creating a temporary directory
    QString path;
    {
      QTemporaryFile file;
      if(file.open()){
	path = file.fileName();
      } else {
	return;
      }
      file.close();
    }
    QDir dir(path);
    dir.mkdir(path);
    if(dir.exists()) {
      auto *nf = new ProcessOutputFrame(this->qemacs,this->buffer);
      this->buffer.addSlave(QObject::tr("notice %1").arg(w),nf);
      QProcess& p = nf->getProcess();
      p.setWorkingDirectory(dir.absolutePath());
      if(p.state()!=QProcess::Running){
	p.start("castem2014_PLEIADES",QStringList());
	p.waitForStarted();
	p.write(QString("INFO '%1';\n").arg(w2).toLatin1());
	p.write("FIN;\n");
	p.waitForFinished(1000);	
      }
      bool r = true;
      Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)){
	if (!info.isDir()) {
	  QFile::remove(info.absoluteFilePath());
	} else {
	  r = false;
	}
      }
      if(r){
	dir.rmdir(path);
      }
    }

  } // end of CastemMajorMode::displayHelp

  void CastemMajorMode::openWebHelp(const QString& w2){
    QDesktopServices::openUrl(QUrl("http://www-cast3m.cea.fr/index.php?page=notices&notice="+w2.toLower()));
  } // end of CastemMajorMode::openWebHelp

  int CastemMajorMode::getMinimalCompletionLength(){
    return 2;
  } // end of CastemMajorMode::getMinimalCompletionLength

  void CastemMajorMode::format()
  {} // end of CastemMajorMode::format

  QString CastemMajorMode::getCommentSyntax(){
    return "*";
  } // end of CastemMajorMode::getCommentSyntax

  QIcon CastemMajorMode::getIcon() const{
    static QIcon i(":/Cast3MIcon.png");
    return i;
  } // end of CastemMajorMode::getIcon()
  
  CastemMajorMode::~CastemMajorMode()
  {
    delete this->ha1;
    delete this->ha2;
  } 

  static StandardQEmacsMajorModeProxy<CastemMajorMode> proxy("Cast3M",
							     QVector<QRegExp>() << QRegExp("^.+\\.dgibi$"));
  
} // end of namespace qemacs
