/*!
 * \file  TextEditReplaceFilter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX
#define LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX

#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEdit;
  // forward declaration
  struct TextEditQueryReplace;

  /*!
   * \brief a key press event filter which forward the user
   */
  struct TextEditReplaceFilter : public TextEditKeyPressEventFilter {
    TextEditReplaceFilter(EditorWidget&,
                          TextEditBase&,
                          TextEditQueryReplace&,
                          const QString&,
                          const QString&);

    bool isOk() const override;

    bool filterKeyPressEvent(QKeyEvent* const) override;
    //! destructor
    ~TextEditReplaceFilter() override;

   protected:
    /*!
     * \brief find the next occurrence of s1
     * \return a boolean stating if a occurrence has been found
     */
    bool findNext();

    EditorWidget& editor;

    TextEditBase& textEdit;

    TextEditQueryReplace& qr;
    //! beginning of the working region
    QTextCursor bc;
    //! end of the working region
    QTextCursor ec;
    //! string to be replaced
    const QString s1;
    //! replacement string
    const QString s2;
    //! number of replacements done
    int nb = 0;
    //! options used for searching the replaced string
    QTextDocument::FindFlag opts = QTextDocument::FindCaseSensitively;
    //! states if the filter is called on a portion of the file
    bool hasSelection = false;
    //! current status
    bool bOK = true;

  };  // end of TextEditReplaceFilter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX */
