/*!
 * \file  TextEditSearch.hxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITSEARCH_HXX
#define LIB_TFEL_GUI_TEXTEDITSEARCH_HXX

#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel {

  namespace gui {

    // forward declaration
    struct EditorWidget;
    // forward declaration
    struct TextEdit;
    // forward declaration
    struct TextEditSearch;

    /*!
     * \brief a key press event filter which forward the user
     */
    struct TFEL_GUI_VISIBILITY_EXPORT TextEditSearchFilter
        : public TextEditKeyPressEventFilter {
      /*!
       * \brief constructor
       */
      TextEditSearchFilter(TextEditBase&, TextEditSearch&);

      bool isOk() const override;

      bool filterKeyPressEvent(QKeyEvent* const) override;
      //! destructor
      ~TextEditSearchFilter() override;

     public slots:

      void lineEditDestroyed();

     protected:
      TextEditBase& textEdit;

      TextEditSearch& s;

     private:
      Q_OBJECT

    };  // end of TextEditSearchFilter

    struct TFEL_GUI_VISIBILITY_EXPORT TextEditSearch : public CommandLine {
      TextEditSearch(
          TextEditBase&,
          EditorWidget&,
          const QTextDocument::FindFlags = QTextDocument::FindCaseSensitively);

      virtual void findNext();

      virtual void setSearchOptions(const QTextDocument::FindFlags);

      virtual QTextDocument::FindFlags getSearchOptions() const;
      //! destructor
      ~TextEditSearch() override;

     protected slots:

      virtual void search(const QString&);

     public slots:

      virtual void searchFilterDestroyed();

     protected:
      void treatUserInput() override;

      TextEditBase& textEdit;

      TextEditSearchFilter* sf;

      //! position of the cursor at the beginning of the search
      QTextCursor cursor;
      //! options used for searching
      QTextDocument::FindFlags opts = QTextDocument::FindFlags();

     private:
      Q_OBJECT

    };  // end of struct TextEditSearch

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_TEXTEDITSEARCH_HXX */
