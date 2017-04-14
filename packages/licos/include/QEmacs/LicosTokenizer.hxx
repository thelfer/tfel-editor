/*! 
 * \file  LicosTokenizer.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 18 sept. 2012
 */

#ifndef _LIB_QEMACS_LICOSTOKENIZER_H_
#define _LIB_QEMACS_LICOSTOKENIZER_H_ 

#include<QVector>

#include"QEmacs/CxxTokenizer.hxx"

namespace qemacs
{

  /*!
   * a tokenizer for licos input file
   */
  struct LicosTokenizer
    : public CxxTokenizer
  {
    enum State {
      STANDARD,
      FAILED,
      ARRAYOPENED
    };

    LicosTokenizer();

    /*!
     * \brief analyse the string given
     * all previous tokens are erased
     * \param[in] s : string to be parsed
     * \param[in] l : line number
     */ 
    virtual void
    parseString(const QString&,
		const unsigned short = 0u);

    /*!
     * \return the curret state
     */
    State
    getState(void) const;

    /*!
     * \return the beginning of currently opened arrays
     */
    QVector<unsigned short>
    getBeginningOfArrays(void) const;

    /*!
     * \return the error message
     */
    virtual QString
    getErrorMessage();

    /*!
     * \param[in] e : error message
     */
    virtual void
    setFailedState(const QString&);

    virtual void
    reset(void);

  protected:

    State state;

    /*!
     * position of the beginning of opened arrays
     */
    QVector<unsigned short> apos;

    QString error;

  }; // end of struct LicosTokenizer

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LICOSTOKENIZER_H */

