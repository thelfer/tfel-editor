/*!
 * \file  LicosTokenizer.hxx
 * \brief
 * \author Helfer Thomas
 * \date   18/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSTOKENIZER_HXX
#define LIB_TFEL_GUI_LICOSTOKENIZER_HXX

#include <QVector>
#include "TFEL/Utilities/CxxTokenizer.hxx"

namespace tfel {

  namespace gui {

    /*!
     * a tokenizer for licos input file
     */
    struct LicosTokenizer : public tfel::utilities::CxxTokenizer {
      enum State { STANDARD, FAILED, ARRAYOPENED };

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

    };  // end of struct LicosTokenizer

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_LICOSTOKENIZER_H */
