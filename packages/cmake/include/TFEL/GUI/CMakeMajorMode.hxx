/*!
 * \file  CMakeMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_CMAKEMAJORMODE_HXX
#define LIB_TFEL_GUI_CMAKEMAJORMODE_HXX

#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  //! \brief a major mode for editing `CMakeLists.txt` files
  struct CMakeMajorMode : public MajorModeBase {
    CMakeMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;
    /*!
     * This method is called before any treatment by
     * TextEditBase and allows the mode to override default
     * shortcuts
     */
    bool keyPressEvent(QKeyEvent *const) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    void format() override;

    QString getCommentSyntax() override;

    ~CMakeMajorMode() override;

   protected:
    virtual void runCompilation();

    void completeContextMenu(QMenu *const, const QTextCursor &) override;

   private:
    Q_OBJECT
  };  // end of struct CMakeMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CMAKEMAJORMODE_HXX */
