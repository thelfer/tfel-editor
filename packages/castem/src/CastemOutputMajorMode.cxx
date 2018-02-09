/*!
 * \file  CastemOutputMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QRegExp>

#include <QtGui/QSyntaxHighlighter>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"

#include "QEmacs/ProcessOutputMajorModeBase.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs {

  //! \brief syntax highlighter for `Cast3M` output
  struct CastemOutputSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param[in] p: document to be highlighted
     */
    explicit CastemOutputSyntaxHighlighter(QTextDocument *p)
        : QSyntaxHighlighter(p) {
      this->error.setForeground(Qt::red);
      this->echo.setForeground(Qt::lightGray);
    }  // end of CastemOutputSyntaxHighlighter

    void highlightBlock(const QString &l) override {
      static auto e = [] {
        QRegExp r("^ \\*\\*\\*\\*\\*  ERREUR (\\d+) \\*\\*\\*\\*\\*");
        r.setMinimal(true);
        return r;
      }();
      if (e.indexIn(l) == 0) {
        this->setFormat(0, l.size(), this->error);
      } else if (l.startsWith(" $") || l.startsWith(" *")) {
        this->setFormat(0, l.size(), this->echo);
      }
    }  // end of highlightBlock

   protected:
    QTextCharFormat error;
    QTextCharFormat echo;

  };  // end of struct CastemOutputSyntaxHighlighter

  //! \brief A major mode to display the results of `Cast3M` outputs
  struct CastemOutputMajorMode : public ProcessOutputMajorModeBase {
    //! constructor
    CastemOutputMajorMode(QEmacsWidget &w,
                          QEmacsBuffer &b,
                          QEmacsTextEditBase &t)
        : ProcessOutputMajorModeBase(w, b, t, &t) {
    }  // end of CastemOutputMajorMode

    QString getName() const override {
      return "castem-output";
    }  // end of getName

    QString getDescription() const override {
      return "major mode dedicated to the output of castem "
             "process";
    }  // end of getDescription

    void setSyntaxHighlighter(QTextDocument *d) override {
      new CastemOutputSyntaxHighlighter(d);
    }  // end of setSyntaxHighlighter

    void format() override {}  // end of format

    void processFinished(int, QProcess::ExitStatus) override {
    }  // end of processFinished
    //! destructor
    ~CastemOutputMajorMode() override = default;
  };  // end of CastemOutputMajorMode

  static StandardQEmacsMajorModeProxy<CastemOutputMajorMode> proxy(
      "castem output");

}  // end of namespace qemacs
