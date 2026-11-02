/*!
 * \file   include/TFEL/GUI/CompletionFrame.hxx
 * \brief unused empty subclass of QTextEdit for the completions.
 * \author Thomas Helfer
 * \date   17/08/2012
 */

#ifndef LIB_TFEL_GUI_COMPLETIONFRAME_HXX
#define LIB_TFEL_GUI_COMPLETIONFRAME_HXX

#include <QtCore/QString>
#include <QtGui/QTextEdit>

namespace tfel::gui {

  /*!
   * \brief empty subclass of `QTextEdit`, named after the frame the
   * completions were meant to be displayed in
   *
   * \note the class adds neither data nor behaviour to `QTextEdit`
   *
   * \note no code of the editor uses it: the completions are displayed in a
   * `TextEdit` area attached to the buffer as a secondary task, as
   * `CommandLine::showCompletions` does
   */
  struct CompletionFrame : public QTextEdit {};

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_COMPLETIONFRAME_HXX */
