/*!
 * \file  MakefileMajorMode.hxx
 * \brief
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_MAKEFILEMAJORMODE_HXX
#define LIB_TFEL_GUI_MAKEFILEMAJORMODE_HXX

#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  //! \brief a major mode for editing Makefile's
  struct MakefileMajorMode : public MajorModeBase {
    MakefileMajorMode(EditorWidget &, Buffer &, TextEditBase &);

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

    ~MakefileMajorMode() override;

   private:
    virtual void runCompilation();

    Q_OBJECT
  };  // end of struct MakefileMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAKEFILEMAJORMODE_H */
