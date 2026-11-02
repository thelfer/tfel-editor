/*!
 * \file  StandardLicosBlock.hxx
 * \brief a licos block described by its name, its keys and its sub blocks.
 * \author Thomas Helfer
 * \date   17/09/2012
 */

#ifndef LIB_TFEL_GUI_STANDARDLICOSBLOCK_HXX
#define LIB_TFEL_GUI_STANDARDLICOSBLOCK_HXX

#include <map>
#include "TFEL/GUI/LicosBlock.hxx"

namespace tfel::gui {

  /*!
   * \brief a licos block described by its name, the keys it accepts and the
   * sub blocks it contains
   *
   * Most blocks of a licos input file need no specific behaviour: they are
   * instances of a subclass of this one which only fills the keys and the sub
   * blocks in its constructor, as `LicosMainBlock` and the blocks it builds
   * do.
   *
   * \note the content of a block is never parsed: the block is a description
   * of what the syntax accepts, used to indent the buffer
   */
  struct StandardLicosBlock : public LicosBlock {
    /*!
     * \brief build a block with the given name, no key and no sub block
     * \param[in] n_ : name of the block, as it is written in the input file
     *
     * \see addKey, addBlock
     */
    StandardLicosBlock(const QString &);

    //! \return the name of the block, that is the name given to the constructor
    QString name() const override;

    //! \return the name of the block preceded by `EndOf`, as `EndOfPlenum`
    QString blockEnd() const override;

    /*!
     * \brief do nothing: a standard block gives no meaning to the arguments
     * written between angle brackets
     * \param[in] params : parameters of the block, ignored
     *
     * \see LicosBlock::treatParameters
     */
    void treatParameters(const QStringList &) override;

    /*!
     * \brief do nothing: a standard block gives no meaning to the tokens
     * which follow the name of the block on the line which opens it
     * \param[in] args : begin of the range of tokens to read; the end of the
     * range which follows is ignored too
     *
     * \see LicosBlock::treatArguments
     */
    void treatArguments(const CxxTokenizer::const_iterator,
                        const CxxTokenizer::const_iterator) override;

    /*!
     * \param[in] k : key name
     * \return true if the given key has been declared by `addKey` or
     * `addKeys`; the comparison is case sensitive
     */
    bool isSubKey(const QString &) const override;

    /*!
     * \param[in] b : block name
     * \return true if a sub block has been declared under the given name by
     * one of the `addBlock` functions
     */
    bool isSubBlock(const QString &) const override;

    /*!
     * \param[in] b : block name
     * \return the sub block declared under the given name
     *
     * \note unlike the base class, this function never throws: an unknown
     * name yields a brand new block of that name and without any key, which
     * accepts nothing; check `isSubBlock` to tell the two cases apart
     */
    QSharedPointer<LicosBlock> getSubBlock(const QString &) const override;

    /*!
     * \brief declare all the given sub blocks
     * \param[in] b : sub blocks, associated with the name they are stored
     * under
     *
     * \note as in `std::map`, a sub block already declared under the same name
     * is kept: this function never replaces anything
     */
    virtual void addBlocks(
        const std::map<QString, QSharedPointer<LicosBlock>> &);

    /*!
     * \brief declare the given block, whose ownership is taken: it is wrapped
     * in a `QSharedPointer` and deleted when the last shared pointer to it
     * dies
     * \param[in] b : block to declare, stored under its own name, which
     * replaces any sub block declared under that name
     */
    virtual void addBlock(LicosBlock *const);

    /*!
     * \brief declare the given block
     * \param[in] b : block to declare, shared with the caller and stored under
     * its own name, which replaces any sub block declared under that name
     */
    virtual void addBlock(const QSharedPointer<LicosBlock>);

    /*!
     * \brief declare a new standard block with the given name and keys
     * \param[in] n_ : name of the sub block
     * \param[in] k : keys the sub block accepts
     *
     * \note any sub block previously declared under that name is replaced
     */
    virtual void addBlock(const QString &, const QStringList &);

    /*!
     * \brief declare all the given keys as valid keys of this block
     * \param[in] k : keys to declare
     *
     * \note the keys are appended, so declaring the same key twice is
     * possible and harmless
     */
    virtual void addKeys(const QStringList &);

    //! \brief declare the given key as a valid key of this block, see `addKeys`
    virtual void addKey(const QString &);

   protected:
    //! name
    QString n;
    //! sub keys
    QStringList keys;
    //! blocks
    std::map<QString, QSharedPointer<LicosBlock>> blocks;
  };  // end of struct StandardLicosBlock

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_STANDARDLICOSBLOCK_H */
