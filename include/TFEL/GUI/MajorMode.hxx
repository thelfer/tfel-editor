/*!
 * \file  MajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   28/06/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODE_HXX
#define LIB_TFEL_GUI_MAJORMODE_HXX

#include <QtCore/QStringList>
#include <QtGui/QTextDocument>
#include <QtWidgets/QMenu>
#include <QtWidgets/QCompleter>
#include "TFEL/GUI/Config.hxx"

namespace tfel{

  namespace gui{

  // forward declaration
  struct TextEditBase;

  //! \brief abstract interface for all major modes
  struct TFEL_GUI_VISIBILITY_EXPORT MajorMode : public QObject {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    MajorMode(QWidget* const);
    //! \return the name of the major mode
    virtual QString getName() const = 0;
    //! \return the description of the major mode
    virtual QString getDescription() const = 0;
   public slots:
    //! update the major mode
    virtual void update() = 0;
   signals:
    //! signal send when the menu associated with the major mode is
    //! updated
    void updatedMenu();
   public:
    /*!
     * \brief hightlight the current document
     */
    virtual void setSyntaxHighlighter(QTextDocument* const) = 0;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    virtual void completeContextMenu(QMenu* const,
                                     const QTextCursor&) = 0;

    virtual QCompleter* getCompleter() = 0;
    /*!
     * \return the completion prefix
     * \note most of the time, it is this->textEdit.getCurrentWord()
     */
    virtual QString getCompletionPrefix() = 0;
    /*!
     * \return the minimal number of characters for a completion to be
     * considered
     */
    virtual int getMinimalCompletionLength() = 0;

    virtual void completeCurrentWord(TextEditBase&,
                                     const QString&) = 0;
    /*!
     * \return a menu specific to the current mode
     * \note the caller will take ownership of the menu
     * and may free the memory associated, including the
     * underlying QAction' objects. Therefore, the
     * returned menu shall not reference a member. 
     */
    virtual QMenu* getSpecificMenu() = 0;
    //! \return the icon associated to the current mode
    virtual QIcon getIcon() const = 0;
    //! \brief format a paragraph
    virtual void format() = 0;
    //! \brief comment/uncomment a region
    virtual void comment() = 0;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     */
    virtual bool keyPressEvent(QKeyEvent* const) = 0;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default behaviour
     */
    virtual bool mousePressEvent(QMouseEvent* const) = 0;
    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
     * TextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    virtual bool handleShortCut(const int,
                                const Qt::KeyboardModifiers,
                                const int) = 0;
    //! \brief indent the current line
    virtual void indentLine(const QTextCursor&) = 0;
    //! \brief indent selected region
    virtual void indentRegion(const QTextCursor&) = 0;

    virtual void setSpellCheckLanguage(const QString&) = 0;
    //! destructor
    virtual ~MajorMode();

   private:
    Q_OBJECT

  };  // end of class MajorMode

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MAJORMODE_HXX */
