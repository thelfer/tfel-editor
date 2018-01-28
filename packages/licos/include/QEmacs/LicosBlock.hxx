/*! 
 * \file  LicosBlock.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#ifndef LIB_QEMACS_LICOSBLOCK_HXX
#define LIB_QEMACS_LICOSBLOCK_HXX 

#include<QtCore/QString>
#include<QtCore/QStringList>
#include<QtCore/QSharedPointer>

#include"QEmacs/CxxTokenizer.hxx"

namespace qemacs
{

  /*!
   * Base class for all licos blocks
   */
  struct LicosBlock
  {

    /*!
     * return the name of the block
     */
    virtual QString name() const = 0;

    /*!
     * return the end of the block
     */
    virtual QString blockEnd() const = 0;

    /*!
     * \param[in] params : parameters of the block
     */
    virtual void treatParameters(const QStringList&) = 0;

    /*!
     * \param[in] args : arguments of the block
     */
    virtual void treatArguments(const CxxTokenizer::const_iterator,
				const CxxTokenizer::const_iterator) = 0;

    /*!
     * \param[in] k : key name
     * \return true if the given word is a valid key of this block
     */
    virtual bool isSubKey(const QString&) const = 0;

    /*!
     * \param[in] b : block name
     * \return true if the given word is a valid sub block name
     */
    virtual bool isSubBlock(const QString&) const = 0;
    
    /*!
     * return a block associated with the given name
     * \throw std::runtime_error if the given name is not handled
     * \see isSubBlcok
     */
    virtual QSharedPointer<LicosBlock>
    getSubBlock(const QString&) const = 0;

    /*!
     * destructor
     */
    virtual ~LicosBlock();

  }; // end of struct LicosBlock

  /*!
   * \return a list of keys that can introduce parameters
   * This list contains Bool, Int, Double, Array, Block
   */
  const QStringList&
  getParameterKeys();
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSBLOCK_H */

