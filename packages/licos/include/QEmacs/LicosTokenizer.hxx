/*! 
 * \file  LicosTokenizer.hxx
 * \brief
 * \author Helfer Thomas
 * \date   18/09/2012
 */

#ifndef LIB_QEMACS_LICOSTOKENIZER_HXX
#define LIB_QEMACS_LICOSTOKENIZER_HXX 

#include<QVector>
#include"TFEL/Utilities/CxxTokenizer.hxx"

namespace qemacs
{

  /*!
   * a tokenizer for licos input file
   */
  struct LicosTokenizer
    : public tfel::utilities::CxxTokenizer
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
     */ 
    void parseString(const QString&);

    /*!
     * \return the curret state
     */
    State getState() const;

    /*!
     * \return the beginning of currently opened arrays
     */
    QVector<unsigned short> getBeginningOfArrays() const;

    /*!
     * \return the error message
     */
    virtual QString getErrorMessage();

    /*!
     * \param[in] e : error message
     */
    virtual void setFailedState(const QString&);

    virtual void reset();

  protected:

    State state;

    /*!
     * position of the beginning of opened arrays
     */
    QVector<unsigned short> apos;

    QString error;

  }; // end of struct LicosTokenizer

} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSTOKENIZER_H */

