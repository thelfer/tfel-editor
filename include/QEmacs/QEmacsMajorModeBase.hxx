/*! 
 * \file  QEmacsMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 04 juil. 2012
 */

#ifndef _LIB_QEMACS_QEMACSMAJORMODEBASE_H_
#define _LIB_QEMACS_QEMACSMAJORMODEBASE_H_ 

#include<QtCore/QMap>
#include<QtCore/QString>

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsMajorMode.hxx"

namespace qemacs
{

  //! forward declaration
  class QEmacsWidget;

  //! forward declaration
  class QEmacsBuffer;

  /*!
   * a helper class to build major modes
   */
  class QEMACS_VISIBILITY_EXPORT QEmacsMajorModeBase
    : public QEmacsMajorMode
  {

    Q_OBJECT

  public:

    QEmacsMajorModeBase(QEmacsWidget&,
			QEmacsBuffer&,
			QEmacsTextEditBase&,
			QWidget *const);

    /*!
     * default implementation of the getCompleter method
     * \return a NULL pointer
     */
    virtual QCompleter*
    getCompleter(void) override;
    /*!
     * \return this->textEdit.getCurrentWord()
     */
    virtual QString
    getCompletionPrefix(void) override;
    /*!
     * \return the minimal number of characters for a completion to be
     * considered
     */
    virtual int
    getMinimalCompletionLength(void) override;

    virtual void
    completeCurrentWord(QEmacsTextEditBase&,
			const QString&) override;
    /*!
     * default implementation of the getSpecificMenu method
     * \return a NULL pointer
     */
    virtual QMenu*
    getSpecificMenu() override;
    /*!
     * default implementation of the getIcon method
     * \return a default constructed QICon
     */
    virtual QIcon
    getIcon(void) const override;
    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default shortcuts
     */
    virtual bool
    keyPressEvent(QKeyEvent * const) override;
    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default behaviour
     */
    virtual bool
    mousePressEvent(QMouseEvent * const) override;
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
		   const int) override;
    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    virtual void
    completeContextMenu(QMenu * const,
			const QTextCursor&) override;
    /*!
     * indent the current line
     */    
    virtual void
    indentLine(const QTextCursor&) override;
    /*!
     * indent selected region
     */    
    virtual void
    indentRegion(const QTextCursor&) override;

    virtual void
    setSpellCheckLanguage(const QString&) override;


    virtual void
    comment(void) override;

    virtual QString
    getCommentSyntax(void);

    virtual ~QEmacsMajorModeBase();

  protected:

    int
    positionInCurrentBlock(const QTextCursor&) const;

    /*!
     * \return recursively search the given file in the given
     * directory
     * \param[in] d : directory
     * \param[in] f : file name
     * \param[in] m : max depth
     * \param[in] s : current depth
     */
    QStringList
    findFiles(const QString&,
	      const QString&,
	      const int m = 10,
	      const int s = 0);

    /*!
     * indent the line under the given text cursor by i spaces. This
     * ensure the cursor remains correctly positionned.
     * \param[in] tc : text cursor
     * \param[in] i  : indentation
     */
    void
    doIndentLine(const QTextCursor&,
		 const int) const;

    /*!
     * an helper function which returns the beginning b and the end e
     * of the selection of the cursor tc. This function guarantees
     * that b<=e. b and e are positioned at the beginning of their
     * blocks.
     * \param[out] b  : beginning of the selection
     * \param[out] e  : end of the selection
     * \param[in]  tc : text cursor
     */
    void
    beginAndEndOfSelection(QTextCursor&,
			   QTextCursor&,
			   const QTextCursor&) const;

    /*!
     * an helper function which returns the beginning and the end of
     * the current paragragraph
     */
    void
    findBeginningAndEndOfTheParagraph(QTextCursor&,
				      QTextCursor&) const;

    QVector<QString>
    getSelectedLines(const QTextCursor&) const;

    QEmacsWidget& qemacs;

    QEmacsBuffer& buffer;

    QEmacsTextEditBase& textEdit;

  }; // end of struct CppMajorMode

} // end of namespace qemacs


#endif /* _LIB_QEMACS_QEMACSMAJORMODEBASE_H */

