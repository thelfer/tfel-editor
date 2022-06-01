/*!
 * \file  ShMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_SHMAJORMODE_HXX
#define LIB_TFEL_GUI_SHMAJORMODE_HXX

#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#if defined TFELEditorShMode_EXPORTS
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#else
#if !defined TFEL_STATIC_BUILD
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#else
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT
#endif
#endif
#else
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

namespace tfel::gui {

  //! \brief a major mode for editing `sh` files
  struct TFELEDITOR_SHMODE_VISIBILITY_EXPORT ShMajorMode
      : public MajorModeBase {
    ShMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    //! \brief format a paragraph
    void format() override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QString getCommentSyntax() override;

    ~ShMajorMode() override;

   private:
    Q_OBJECT
  };  // end of struct ShMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SHMAJORMODE_HXX */
