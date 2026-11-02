/*!
 * \file  LicosTokenizer.hxx
 * \brief LicosTokenizer: a licos file parser for indentation and comments.
 * \author Thomas Helfer
 * \date   18/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSTOKENIZER_HXX
#define LIB_TFEL_GUI_LICOSTOKENIZER_HXX

#include <QVector>
#include "TFEL/Utilities/CxxTokenizer.hxx"

namespace tfel::gui {

  /*!
   * \brief a tokenizer for licos input file
   */
  struct LicosTokenizer : public tfel::utilities::CxxTokenizer {
    /*!
     * \brief state of the tokenizer, that is of the file it has parsed so far
     *
     * \c STANDARD is the normal state, \c FAILED means an invalid line has been
     * met and \c ARRAYOPENED means an array, that is a brace, is not closed
     * yet
     *
     * \note the licos mode gives up indenting or commenting as soon as the
     * state is \c FAILED, the rest of the file being undecidable
     */
    enum State { STANDARD, FAILED, ARRAYOPENED };

    /*!
     * \brief constructor: the state is \c STANDARD
     *
     * \note characters are treated as strings and the boundaries of the
     * comments are kept, as the mode has to remove the comments it has found
     */
    LicosTokenizer();

    /*!
     * \brief analyse the string given
     * all previous tokens are erased
     * \param[in] s : string to be parsed
     *
     * \note the state of the previously opened C style comment is kept, so that
     * the lines of a file can be parsed one after the other
     *
     * \note the braces which open and close an array are detected: the state
     * becomes \c ARRAYOPENED while an array is opened and a brace which closes
     * no array sets the \c FAILED state
     *
     * \note an invalid line does not throw: the tokenizer goes to the \c FAILED
     * state and keeps the message of the exception
     *
     * \note nothing is parsed at all once the state is \c FAILED
     */
    void parseString(const QString&);

    /*!
     * \return the curret state
     */
    State getState() const;

    /*!
     * \return the beginning of currently opened arrays
     *
     * \note those are the positions of the opening braces of the arrays which
     * are not closed yet, in the order in which they have been opened, each of
     * them being its position in the line where it was met
     *
     * \note the indentation uses the last one to align the lines which continue
     * an array
     */
    QVector<unsigned short> getBeginningOfArrays() const;

    /*!
     * \return the error message
     *
     * \note it is empty as long as no error has been met; it is only meaningful
     * when the state is \c FAILED
     */
    virtual QString getErrorMessage();

    /*!
     * \brief put the tokenizer in the \c FAILED state with the given message,
     * so that the rest of the file is not parsed anymore
     * \param[in] e : error message
     */
    virtual void setFailedState(const QString&);

    /*!
     * \brief forget the error and the opened arrays: the state comes back to
     * \c STANDARD
     *
     * \note the tokens of the last parsed string and the state of the C style
     * comments are left untouched
     */
    virtual void reset();

   protected:
    //! state of the file which has been parsed so far
    State state;

    /*!
     * position of the beginning of opened arrays
     */
    QVector<unsigned short> apos;

    //! message of the error which set the \c FAILED state
    QString error;

  };  // end of struct LicosTokenizer

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSTOKENIZER_H */
