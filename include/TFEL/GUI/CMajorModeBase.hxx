/*!
 * \file  CMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   05/09/2012
 */

#ifndef LIB_TFEL_GUI_CMAJORMODEBASE_HXX
#define LIB_TFEL_GUI_CMAJORMODEBASE_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CompiledLanguageMajorModeBase.hxx"

namespace tfel::gui {

  /*!
   * \brief base class for C-based language.
   * This mainly deals with indentation by relying on `clang-format`
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CMajorModeBase
      : public CompiledLanguageMajorModeBase {
    /*!
     * \brief constructor
     */
    CMajorModeBase(EditorWidget&, Buffer&, TextEditBase&);
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

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CMAJORMODEBASE_HXX */
