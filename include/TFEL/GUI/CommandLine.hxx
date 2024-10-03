/*!
 * \file  CommandLine.hxx
 * \brief
 * \author Thomas Helfer
 * \date   05/08/2012
 */

#ifndef LIB_TFEL_GUI_COMMANDLINE_HXX
#define LIB_TFEL_GUI_COMMANDLINE_HXX

#include <QtCore/QPointer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/LineEdit.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  // forward declaration
  struct TextEdit;

  struct TFEL_GUI_VISIBILITY_EXPORT CommandLine : public QWidget {
    /*!
     * \param[in] l : label
     * \param[in] p : parent
     * \param[in] b : if true, set the default custom line edit
     */
    CommandLine(const QString &, EditorWidget &p, const bool = true);

    virtual void setFocus();

    virtual void setInputHistorySettingAddress(const QString &);

    virtual void setInputHistory(const QStringList &);

    void keyPressEvent(QKeyEvent *) override;

    virtual void setLabel(const QString &);

    virtual bool isBlocking() const;
    //! destructor
    ~CommandLine() override;

   public slots:

    virtual void userEditingFinished();

    //! cancel editing
    virtual void cancel();

    virtual void showCompletions(const QString &, const QStringList &);

    virtual void hideCompletions();

   signals:

    void finished(CommandLine *);

    void destroyed(CommandLine *);

    void textChanged(const QString &);

    void textEdited(const QString &);

   protected slots:

    /*!
     * processing is finished
     */
    virtual void treatUserInput() = 0;

    virtual void inputTextChanged(const QString &);

    virtual void inputTextEdited(const QString &);

   protected:
    struct CustomLineEdit : public LineEdit {
      CustomLineEdit(EditorWidget &, CommandLine &);

      bool event(QEvent *) override;

      void keyPressEvent(QKeyEvent *) override;
      /*!
       * \param[in] b : if true, call the QLineEdit::setCompleter
       */
      virtual void setCompleter(QCompleter *const, const bool);

      virtual QCompleter *completer() const;

      ~CustomLineEdit() override;

     protected:
      /*!
       * treat "Ctrl-k1 Mod-k2"
       * where k1 is either Qt::Key_X or Qt::Key_C.
       * \param[in] k1 : first  key
       * \param[in] m  : second key modifier
       * \param[in] k2 : second key
       * \return true if the short cut is handled by this mode
       */
      void handleShortCut(const int,
                          const Qt::KeyboardModifiers,
                          const int) override;

      virtual void complete();

      /*!
       * \return a completion for the given input. An empty string is
       * returned if no completion is available.
       * \param[out] b : set to true if only one completion is
       * avaiable
       */
      virtual QString findCompletion(bool &);

      virtual QString extractBaseForCompletion(const QString &);

      CommandLine &lineEdit;
      QCompleter *c_ = nullptr;
      bool completerHandledByQLineEdit = false;
    };

    void setLineEdit(CustomLineEdit *const);

    EditorWidget &editor;
    Buffer &buffer;
    QVBoxLayout *vl;
    QHBoxLayout *hl;
    QLabel *label;
    CustomLineEdit *input;
    QPointer<TextEdit> completions;
    QWidget *scompletions;
    QString inputHistorySettingAddress;
    bool isUserEditingFinished;

   private:
    Q_OBJECT
  };  // end of CommandLine

  /*!
   * An helper class asking the user to type y (yes) or n (no)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT YesOrNoUserInput : public CommandLine {
    YesOrNoUserInput(const QString &, EditorWidget &p);

   protected:
    struct YesOrNoLineEdit;

  };  // end of struct YesOrNoUserInput

  //! An helper class asking the user for a file path
  struct TFEL_GUI_VISIBILITY_EXPORT FilePathUserInput : public CommandLine {
    FilePathUserInput(const QString &, EditorWidget &);

   protected:
    struct FilePathLineEdit;

  };  // end of struct YesOrNoUserInput

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_COMMANDLINE_HXX */
