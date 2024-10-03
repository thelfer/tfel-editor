/*!
 * \file  RectangleMode.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/09/2012
 */

#ifndef LIB_TFEL_GUI_RECTANGLEMODE_HXX
#define LIB_TFEL_GUI_RECTANGLEMODE_HXX

#include "TFEL/GUI/CommandLine.hxx"

namespace tfel::gui {

  struct TextEditBase;

  /*!
   *
   */
  struct RectangleMode : public CommandLine {
    RectangleMode(TextEditBase&, EditorWidget&);

    virtual bool isBlocking() const override;

    //! \brief method called when processing is finished
    virtual void treatUserInput() override;

   protected:
    static QVector<QString> killedRectangle;

    struct RectangleModeLineEdit;

    void killRectangle();

    void pasteRectangle();

    TextEditBase& textEdit;

  };  // end of struct RectangleMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_RECTANGLEMODE_HXX */
