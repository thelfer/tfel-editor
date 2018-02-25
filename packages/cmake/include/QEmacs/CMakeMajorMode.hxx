/*!
 * \file  CMakeMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#ifndef LIB_QEMACS_CMAKEMAJORMODE_HXX
#define LIB_QEMACS_CMAKEMAJORMODE_HXX

#include "QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs {

  //! \brief a major mode for editing `CMakeLists.txt` files
  struct CMakeMajorMode : public QEmacsMajorModeBase {
    CMakeMajorMode(QEmacsWidget &,
                       QEmacsBuffer &,
                       QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;
    /*!
     * This method is called before any treatment by
     * QEmacsTextEditBase and allows the mode to override default
     * shortcuts
     */
    bool keyPressEvent(QKeyEvent *const) override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    void format() override;

    QString getCommentSyntax() override;

    ~CMakeMajorMode() override;

   protected:
    virtual void runCompilation();

    virtual void actionTriggered(QAction *);

    void completeContextMenu(QMenu *const,
                             const QTextCursor &) override;

    //! help action
    QAction *ha = nullptr;

   private:
    Q_OBJECT
  };  // end of struct CMakeMajorMode

}  // end of namespace qemacs

#endif /* LIB_QEMACS_CMAKEMAJORMODE_HXX */
