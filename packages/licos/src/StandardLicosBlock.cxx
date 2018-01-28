/*! 
 * \file  StandardLicosBlock.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#include"QEmacs/StandardLicosBlock.hxx"

namespace qemacs
{

  StandardLicosBlock::StandardLicosBlock(const QString& n_)
    : n(n_)
  {}

  void
  StandardLicosBlock::treatParameters(const QStringList&)
  {} // end of StandardLicosBlock::treatParameters

  void
  StandardLicosBlock::treatArguments(const CxxTokenizer::const_iterator,
				     const CxxTokenizer::const_iterator)
  {} // end of StandardLicosBlock::treatArguments

  QString
  StandardLicosBlock::name() const
  {
    return this->n;
  }

  QString
  StandardLicosBlock::blockEnd() const
  {
    return "EndOf"+this->n;
  }

  bool
  StandardLicosBlock::isSubKey(const QString& k) const
  {
    return this->keys.contains(k);
  }

  bool
  StandardLicosBlock::isSubBlock(const QString& b) const
  {
    return this->blocks.contains(b);
  }

  QSharedPointer<LicosBlock>
  StandardLicosBlock::getSubBlock(const QString& b) const
  {
    if(!this->isSubBlock(b)){
      return QSharedPointer<LicosBlock>(new StandardLicosBlock(b));
    }
    return this->blocks[b];
  } // end of StandardLicosBlock::getSubBlock

  void
  StandardLicosBlock::addKey(const QString& k)
  {
    this->keys << k;
  }

  void
  StandardLicosBlock::addKeys(const QStringList& k)
  {
    this->keys << k;
  }

  void
  StandardLicosBlock::addBlocks(const QMap<QString,QSharedPointer<LicosBlock>>& b)
  {
    QMapIterator<QString,QSharedPointer<LicosBlock>> p(b);
    while (p.hasNext()) {
      p.next();
      this->blocks.insert(p.key(),p.value());
    }

  }

  void StandardLicosBlock::addBlock(LicosBlock * const b)
  {
    this->addBlock(QSharedPointer<LicosBlock>(b));
  }

  void StandardLicosBlock::addBlock(const QSharedPointer<LicosBlock> b)
  {
    this->blocks[b->name()] = b;
  }

  void
  StandardLicosBlock::addBlock(const QString& n_,
			       const QStringList& k)
  {
    auto *b =  new StandardLicosBlock(n_);
    b->addKeys(k);
    this->blocks[n_] = QSharedPointer<LicosBlock>(b);
  }

} // end of namespace qemacs
