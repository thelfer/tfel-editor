/*! 
 * \file  StandardLicosBlock.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#ifndef _LIB_QEMACS_STANDARDLICOSBLOCK_H_
#define _LIB_QEMACS_STANDARDLICOSBLOCK_H_ 

#include<QtCore/QMap>

#include"QEmacs/LicosBlock.hxx"

namespace qemacs
{

  struct StandardLicosBlock
    : public LicosBlock
  {

    StandardLicosBlock(const QString&);
  
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

    virtual void
    addBlocks(const QMap<QString,QSharedPointer<LicosBlock> >&);

    virtual void
    addBlock(LicosBlock *const);

    virtual void
    addBlock(const QSharedPointer<LicosBlock>);

    virtual void
    addBlock(const QString&,
	     const QStringList&);

    virtual void
    addKeys(const QStringList&);

    virtual void
    addKey(const QString&);

  protected:

    //! name
    QString n;

    //! sub keys
    QStringList keys;

    //! blocks
    QMap<QString,QSharedPointer<LicosBlock> > blocks;

  }; // end of struct StandardLicosBlock

} // end of namespace qemacs

#endif /* _LIB_QEMACS_STANDARDLICOSBLOCK_H */

