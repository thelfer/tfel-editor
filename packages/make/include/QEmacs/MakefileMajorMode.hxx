/*!
 * \file  MakefileMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 août 2012
 */

#ifndef LIB_QEMACS_MAKEFILEMAJORMODE_HXX
#define LIB_QEMACS_MAKEFILEMAJORMODE_HXX

#include "QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs {

  //! \brief a major mode for editing Makefile's
  struct MakefileMajorMode : public QEmacsMajorModeBase {
    MakefileMajorMode(QEmacsWidget &,
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

    ~MakefileMajorMode() override;

   private:
    virtual void runCompilation();

    Q_OBJECT
  };  // end of struct MakefileMajorMode

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MAKEFILEMAJORMODE_H */
