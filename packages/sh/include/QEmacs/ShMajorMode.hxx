/*!
 * \file  ShMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#ifndef LIB_QEMACS_SHMAJORMODE_HXX
#define LIB_QEMACS_SHMAJORMODE_HXX

#include"TFEL/Config/TFELConfig.hxx"
#include "QEmacs/QEmacsMajorModeBase.hxx"

#if defined _WIN32 || defined _WIN64 ||defined __CYGWIN__
#  if defined ShQEmacsMode_EXPORTS
#    define SHQEMACSMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#  else
#  if !defined TFEL_STATIC_BUILD
#    define SHQEMACSMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#  else 
#    define SHQEMACSMODE_VISIBILITY_EXPORT 
#  endif
#  endif
#else
#  define SHQEMACSMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

namespace qemacs {

  //! \brief a major mode for editing `sh` files
  struct SHQEMACSMODE_VISIBILITY_EXPORT ShMajorMode
      : public QEmacsMajorModeBase {
    ShMajorMode(QEmacsWidget &, QEmacsBuffer &, QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;

    //! \brief format a paragraph
    void format() override;

    void setSyntaxHighlighter(QTextDocument *const) override;

    QString getCommentSyntax() override;

    ~ShMajorMode() override;

   private:
    Q_OBJECT
  };  // end of struct ShMajorMode

}  // end of namespace qemacs

#endif /* LIB_QEMACS_SHMAJORMODE_HXX */
