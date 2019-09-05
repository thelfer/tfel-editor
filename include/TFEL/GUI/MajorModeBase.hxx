/*!
 * \file  MajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   04/07/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODEBASE_HXX
#define LIB_TFEL_GUI_MAJORMODEBASE_HXX

#include <QtCore/QString>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/MajorMode.hxx"

namespace tfel{

  namespace gui{

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  /*!
   * \brief a helper class to build major modes
   */
  struct TFEL_GUI_VISIBILITY_EXPORT MajorModeBase
      : public MajorMode {
   public slots:

    void update() override;

   public:
    MajorModeBase(EditorWidget&,
                        Buffer&,
                        TextEditBase&,
                        QWidget* const);

    /*!
     * default implementation of the getCompleter method
     * \return a NULL pointer
     */
    QCompleter* getCompleter() override;
    /*!
     * \return this->textEdit.getCurrentWord()
     */
    QString getCompletionPrefix() override;
    /*!
     * \return the minimal number of characters for a completion to be
     * considered
     */
    int getMinimalCompletionLength() override;

    void completeCurrentWord(TextEditBase&,
                             const QString&) override;
    /*!
     * \brief default implementation of the getSpecificMenu method
     * \return a NULL pointer
     */
    QMenu* getSpecificMenu() override;
    /*!
     * default implementation of the getIcon method
     * \return a default constructed QICon
     */
    QIcon getIcon() const override;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     */
    bool keyPressEvent(QKeyEvent* const) override;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default behaviour
     */
    bool mousePressEvent(QMouseEvent* const) override;
    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
     * TextEditBase where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;
    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    void completeContextMenu(QMenu* const, const QTextCursor&) override;
    //! \brief indent the whole text
    void indentAll() override;
    /*!
     * indent the current line
     */
    void indentLine(const QTextCursor&) override;
    /*!
     * indent selected region
     */
    void indentRegion(const QTextCursor&) override;

    void setSpellCheckLanguage(const QString&) override;

    void comment() override;

    /*!
     * \brief report a message
     * \param[in] m: message
     */
    virtual void report(const QString&);

    virtual QString getCommentSyntax();
    //! destructor
    ~MajorModeBase() override;

   protected:
    int positionInCurrentBlock(const QTextCursor&) const;
    /*!
     * \return recursively search the given file in the given
     * directory
     * \param[in] d : directory
     * \param[in] f : file name
     * \param[in] m : max depth
     * \param[in] s : current depth
     */
    QStringList findFiles(const QString&,
                          const QString&,
                          const int = 10,
                          const int = 0);
    /*!
     * indent the line under the given text cursor by i spaces. This
     * ensure the cursor remains correctly positionned.
     * \param[in] tc : text cursor
     * \param[in] i  : indentation
     */
    void doIndentLine(const QTextCursor&, const int) const;
    /*!
     * an helper function which returns the beginning b and the end e
     * of the selection of the cursor tc. This function guarantees
     * that b<=e. b and e are positioned at the beginning of their
     * blocks.
     * \param[out] b  : beginning of the selection
     * \param[out] e  : end of the selection
     * \param[in]  tc : text cursor
     */
    void beginAndEndOfSelection(QTextCursor&,
                                QTextCursor&,
                                const QTextCursor&) const;

    /*!
     * an helper function which returns the beginning and the end of
     * the current paragragraph
     */
    void findBeginningAndEndOfTheParagraph(QTextCursor&,
                                           QTextCursor&) const;

    QVector<QString> getSelectedLines(const QTextCursor&) const;

    EditorWidget& editor;

    Buffer& buffer;

    TextEditBase& textEdit;

   private:
    Q_OBJECT

  };  // end of struct CppMajorMode

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MAJORMODEBASE_HXX */
