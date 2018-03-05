/*!
 * \file  StandardLicosBlock.hxx
 * \brief
 * \author Helfer Thomas
 * \date   17/09/2012
 */

#ifndef LIB_QEMACS_STANDARDLICOSBLOCK_HXX
#define LIB_QEMACS_STANDARDLICOSBLOCK_HXX

#include <map>
#include "QEmacs/LicosBlock.hxx"

namespace qemacs {

  struct StandardLicosBlock : public LicosBlock {
    StandardLicosBlock(const QString &);

    QString name() const override;

    QString blockEnd() const override;

    void treatParameters(const QStringList &) override;

    void treatArguments(const CxxTokenizer::const_iterator,
                        const CxxTokenizer::const_iterator) override;

    bool isSubKey(const QString &) const override;

    bool isSubBlock(const QString &) const override;

    QSharedPointer<LicosBlock> getSubBlock(
        const QString &) const override;

    virtual void addBlocks(
        const std::map<QString, QSharedPointer<LicosBlock>> &);

    virtual void addBlock(LicosBlock *const);

    virtual void addBlock(const QSharedPointer<LicosBlock>);

    virtual void addBlock(const QString &, const QStringList &);

    virtual void addKeys(const QStringList &);

    virtual void addKey(const QString &);

   protected:
    //! name
    QString n;

    //! sub keys
    QStringList keys;

    //! blocks
    std::map<QString, QSharedPointer<LicosBlock>> blocks;

  };  // end of struct StandardLicosBlock

}  // end of namespace qemacs

#endif /* LIB_QEMACS_STANDARDLICOSBLOCK_H */
