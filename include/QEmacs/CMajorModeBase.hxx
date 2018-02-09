/*!
 * \file  CMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   05/09/2012
 */

#ifndef LIB_QEMACS_CMAJORMODEBASE_HXX
#define LIB_QEMACS_CMAJORMODEBASE_HXX

#include "QEmacs/Config.hxx"
#include "QEmacs/CompiledLanguageMajorModeBase.hxx"

namespace qemacs {

  /*!
   * \brief base class for C-based language.
   * This mainly deals with indentation by relying on `clang-format`
   */
  struct QEMACS_VISIBILITY_EXPORT CMajorModeBase
      : public CompiledLanguageMajorModeBase {
    /*!
     * \brief constructor
     */
    CMajorModeBase(QEmacsWidget&, QEmacsBuffer&, QEmacsTextEditBase&);
    //! \brief format a paragraph
    void format() override;
    //! \brief comment/uncomment a region
    void comment() override;

    void indentLine(const QTextCursor&) override;

    void indentRegion(const QTextCursor&) override;
    //! destructor
    ~CMajorModeBase() override;

   protected:
    /*!
     * \brief indent a region
     * \param[in] b: beginning of the region
     * \param[in] e: end of the region
     * \param[in] c: current position
     */
    virtual void indent(const QTextCursor&,
                        const QTextCursor&,
                        const QTextCursor&);

   private:
    Q_OBJECT
  };  // end of struct CMajorModeBase

}  // end of namespace qemacs

#endif /* LIB_QEMACS_CMAJORMODEBASE_HXX */
