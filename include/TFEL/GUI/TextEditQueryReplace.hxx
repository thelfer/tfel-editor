/*!
 * \file   include/TFEL/GUI/TextEditQueryReplace.hxx
 * \brief
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX
#define LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX

#include "TFEL/GUI/CommandLine.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEdit;

  struct TextEditQueryReplace : public CommandLine {
    TextEditQueryReplace(TextEditBase&, EditorWidget&);
    //! desctructor
    ~TextEditQueryReplace() override;

   protected:
    static QStringList getHistory();

    static void addToHistory(const QString&);

    TextEditQueryReplace(TextEditBase&, EditorWidget&, const QString&);

    TextEditQueryReplace(TextEditBase&,
                         EditorWidget&,
                         const QString&,
                         const QString&);

    void treatUserInput() override;

    TextEditBase& textEdit;
    QString s1;
    int stage;

  };  // end of struct TextEditQueryReplace

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX */
