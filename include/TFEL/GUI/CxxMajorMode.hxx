/*!
 * \file  CxxMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_CXXMAJORMODE_HXX
#define LIB_TFEL_GUI_CXXMAJORMODE_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CMajorModeBase.hxx"

namespace tfel {

  namespace gui {

    // forward declaration
    struct TextEditBase;

    //! major mode dedicated to the C++ language
    struct TFEL_GUI_VISIBILITY_EXPORT CxxMajorMode : public CMajorModeBase {
      CxxMajorMode(EditorWidget &, Buffer &, TextEditBase &);

      QString getName() const override;

      QString getDescription() const override;

      QIcon getIcon() const override;

      /*!
       * This method is called before any treatment by TextEditBase
       * and allows the mode to override default shortcuts
       */
      bool keyPressEvent(QKeyEvent *const) override;

      void setSyntaxHighlighter(QTextDocument *const) override;
      //! destructor
      ~CxxMajorMode() override;

     protected:
      QString getLanguageName() const override;

     private:
      Q_OBJECT
    };  // end of struct CxxMajorMode

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_CXXMAJORMODE_HXX */
