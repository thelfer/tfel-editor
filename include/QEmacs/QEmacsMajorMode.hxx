/*! 
 * \file  QEmacsMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 28 juin 2012
 */

#ifndef _LIB_QEMACS_QEMACSMAJORMODE_H_
#define _LIB_QEMACS_QEMACSMAJORMODE_H_ 

#include<QtCore/QStringList>

#ifdef QEMACS_QT4
#include<QtGui/QMenu>
#include<QtGui/QCompleter>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QMenu>
#include<QtWidgets/QCompleter>
#endif /* QEMACS_QT5 */
#include<QtGui/QTextDocument>

#include"QEmacs/Config.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
    
  /*!
   * base class for major mode
   */
  class QEMACS_VISIBILITY_EXPORT QEmacsMajorMode
    : public QObject
  {
    
    Q_OBJECT

  public:

    /*!
     * constructor
     */
    QEmacsMajorMode(QWidget *const);
    
    /*!
     * return the name of the major mode
     */
    virtual QString
    getName(void) const = 0;

    /*!
     * return the description of the major mode
     */
    virtual QString
    getDescription(void) const = 0;

    /*!
     * hightlight the current document
     */
    virtual void
    setSyntaxHighlighter(QTextDocument *const) = 0;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    virtual void
    completeContextMenu(QMenu * const,
			const QTextCursor&) = 0;

    virtual QCompleter*
    getCompleter(void) = 0;
    /*!
     * \return the completion prefix
     * \note most of the time, it is this->textEdit.getCurrentWord()
     */
    virtual QString
    getCompletionPrefix(void) = 0;
    /*!
     * \return the minimal number of characters for a completion to be
     * considered
     */
    virtual int
    getMinimalCompletionLength(void) = 0;

    virtual void
    completeCurrentWord(QEmacsTextEditBase&,
			const QString&) = 0;

    virtual QMenu*
    getSpecificMenu(void) = 0;

    virtual QIcon
    getIcon(void) const = 0;

    /*!
     * format a paragraph
     */
    virtual void
    format(void) = 0;

    /*!
     * comment/uncomment a region
     */
    virtual void
    comment(void) = 0;

    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default shortcuts
     */
    virtual bool
    keyPressEvent(QKeyEvent * const) = 0;

    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default behaviour
     */
    virtual bool
    mousePressEvent(QMouseEvent * const) = 0;

    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by QEmacsTextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    virtual bool
    handleShortCut(const int,
		   const Qt::KeyboardModifiers,
		   const int) = 0;

    /*!
     * indent the current line
     */    
    virtual void
    indentLine(const QTextCursor&) = 0;

    /*!
     * indent selected region
     */    
    virtual void
    indentRegion(const QTextCursor&) = 0;

    virtual void
    setSpellCheckLanguage(const QString&) = 0;
    
    /*!
     * destructor
     */
    virtual ~QEmacsMajorMode();

  }; // end of class QEmacsMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSMAJORMODE_H */

