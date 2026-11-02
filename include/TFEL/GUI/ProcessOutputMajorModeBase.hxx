/*!
 * \file   ProcessOutputMajorModeBase.hxx
 * \brief base class of the major modes of a process output.
 * \author Thomas Helfer
 * \date   7/02/2018
 */

#ifndef LIB_TFEL_GUI_PROCESSOUTPUTMAJORMODEBASE_HXX
#define LIB_TFEL_GUI_PROCESSOUTPUTMAJORMODEBASE_HXX

#include <QtCore/QProcess>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  /*!
   * \brief a base class for major mode associated with external process
   * output.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT ProcessOutputMajorModeBase : MajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: widget
     * \param[in] b: buffer
     * \param[in] t: text edit widget
     * \param[in] p: parent widget
     */
    ProcessOutputMajorModeBase(EditorWidget&,
                               Buffer&,
                               TextEditBase&,
                               QWidget* const);
   public slots:

    /*!
     * \brief start the process the mode displays the output of
     *
     * The text edit is cleared first, so the previous run output is lost, and
     * the icon of the secondary task shows that a process is running.
     *
     * \note this is what the \c Restart entry of the context menu and the
     * \c Alt-m key trigger
     *
     * \note nothing is started and a message is reported in the minibuffer
     * when the text edit is not a \c ProcessOutputFrame, when a process is
     * already running or when the command is empty
     */
    virtual void restart();

    /*!
     * \brief terminate the process the mode displays the output of
     *
     * \note this is what the \c Stop entry of the context menu triggers
     *
     * \note nothing is terminated and a message is reported in the minibuffer
     * when the text edit is not a \c ProcessOutputFrame or when no process is
     * running
     */
    virtual void stop();

   public:
    /*!
     * \brief set the working directory the process was started in
     * \param[in] c : directory
     *
     * \note this is only a record for the modes: it is the \c QProcess of the
     * frame, which \c restart reuses, that holds the working directory
     */
    virtual void setDirectory(const QString&);
    /*!
     * \brief set the program \c restart starts
     * \param[in] c : command
     */
    virtual void setCommand(const QString&);
    /*!
     * \brief set the arguments \c restart passes to the command
     * \param[in] a : arguments
     *
     * \note \c restart starts the command without any argument when this list
     * is empty
     */
    virtual void setArguments(const QStringList&);
    /*!
     * \brief set the name of the major mode the output is displayed with
     * \param[in] m : major mode name
     *
     * \note this is only a record for the modes: the frame already displays
     * its output with that mode
     */
    virtual void setMajorMode(const QString&);
    /*!
     * \brief handle key event
     * \param[in] e: event
     *
     * \c Alt-m restarts the process.
     *
     * \return true if the mode handled the key, that is on \c Alt-m only; any
     * other key is left to \c MajorModeBase, which handles none
     */
    bool keyPressEvent(QKeyEvent* const) override;

    /*!
     * \brief add a \c Restart and a \c Stop entry to the context menu
     * \param[in] m : complete the context menu
     * \param[in] tc : text cursor at the position where the menu will
     * appear
     *
     * \note both entries are wired to the \c restart and \c stop slots
     */
    void completeContextMenu(QMenu* const, const QTextCursor&) override;
    /*!
     * \brief method called when the process is finished
     * \param[in] s: exit code of the process, 0 when it succeeds
     * \param[in] e: exit status of the process
     *
     * \note \c ProcessOutputFrame calls this method, through its own
     * \c processFinished slot, as soon as the process ends. A status of 0
     * means success: the frame has already set the icon of the secondary task
     * accordingly before calling the mode
     */
    virtual void processFinished(int, QProcess::ExitStatus) = 0;
    //! destructor
    ~ProcessOutputMajorModeBase() override;

   protected:
    //! working directory the process was started in
    QString directory;
    //! program the process runs, as typed by the user
    QString command;
    //! arguments of the program
    QStringList arguments;
    //! name of the major mode the output is displayed with
    QString mode;

   private:
    Q_OBJECT
  };  // end of struct ProcessOutputMajorModeBase

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PROCESSOUTPUTMAJORMODEBASE_HXX */
