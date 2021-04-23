/*!
 * \file  Buffer.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#ifndef LIB_TFEL_GUI_BUFFER_HXX
#define LIB_TFEL_GUI_BUFFER_HXX

#include <vector>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAbstractScrollArea>
#include "TFEL/GUI/Config.hxx"

namespace tfel {

  namespace gui {

    //! forward declaration
    struct EditorWidget;

    //! forward declaration
    struct SecondaryTaskManager;

    //! forward declaration
    struct PlainTextEdit;

    /*!
     * \brief class in charge of managing one buffer.
     *
     * A buffer is made of:
     * - a main widget (most of the time an instance of
     *   `PlainTextEdit`)
     * - SecondaryTasks of that main widget
     *
     *  \note: a SecondaryTask can be shared between several buffers.
     */
    struct TFEL_GUI_VISIBILITY_EXPORT Buffer : public QWidget {
      Buffer(const int, EditorWidget &);

      Buffer(const QString &, const int, EditorWidget &);

      virtual int getId() const;

      virtual QString getBufferName() const;

      virtual QString getBufferNameSuffix() const;

      virtual QString getBufferRawName() const;

      virtual PlainTextEdit &getMainFrame();
      //! \return the current SecondaryTask if any, nullptr otherwise
      virtual QWidget *getCurrentSecondaryTask();
      /*!
       * \return the title of the current SecondaryTask if any,
       * an empty string otherwise
       */
      virtual QString getCurrentSecondaryTaskTitle() const;

      virtual std::vector<QMenu *> getSpecificMenus();

      virtual QIcon getIcon() const;

      int getSecondaryTaskIndex(QWidget *const p) const;

      virtual void refreshSecondaryTaskTabWidget();

      /*!
       * \brief attach a secondary task to the buffer.
       * \param[in] t: title
       * \param[in] w: associated widget
       */
      virtual QWidget *attachSecondaryTask(const QString &, QWidget *const);
      /*!
       * \brief attach an existing task to the buffer
       * \param[in] w: widget
       */
      virtual void attachSecondaryTask(QWidget *const);

      virtual void removeSecondaryTask(QWidget *const);

      virtual QString getSecondaryTaskTitle(QWidget *const) const;

      virtual void setSecondaryTaskTitle(QWidget *const, const QString &);

      virtual void setSecondaryTaskIcon(QWidget *const, const QIcon &);

      virtual void showSecondaryTask(QWidget *const);

      virtual void hideSecondaryTask(QWidget *const);

      virtual bool hasSecondaryTasks() const;

      virtual bool areSecondaryTasksVisible() const;

      virtual bool isOkToClose() const;

      bool eventFilter(QObject *, QEvent *) override;
      //! destructor
      ~Buffer() override;

     public slots:

      virtual void showSecondaryTasks();

      virtual void setSecondaryTasksOrientation(const Qt::Orientation);

      virtual void hideSecondaryTasks();

      virtual void focusCurrentSecondaryTask();

      virtual void focusMainFrame();

      virtual void closeCurrentSecondaryTask();

      virtual void updateMenu();

     signals:

      void mainFrameMajorModeChanged();
      //! signal launched when the menu of the buffer shall be updated
      void updatedMenu();
      /*!
       * \param[out] b : reference to this
       * \param[out] o : old name
       * \param[out] n : new name
       */
      void bufferNameChanged(Buffer *, const QString &, const QString &);

      void newTreatedFile(const QString &);

     protected slots:

      void focusInEvent(QFocusEvent *) override;

      virtual void updatePosition();

      virtual void updateDate();

      virtual void updateBufferInformations();

      virtual void updateBufferName();

      virtual void closeSecondaryTask(int);

      virtual void emitNewTreatedFile(const QString &);

     protected:
      struct SecondaryTaskTabWidget;

      void initialize();

      EditorWidget &editor;

      //! widget handling the main buffer and its SecondaryTasks
      QSplitter *splitter;
      //! SecondaryTask widgets
      SecondaryTaskTabWidget *stw;
      //! main widget
      PlainTextEdit *e;
      // timer to update the date
      QTimer *timer;
      //! display information about the current buffer
      QHBoxLayout *info;
      //! buffer name
      QLabel *bni;
      //! relative position
      QLabel *rpi;
      //! absolute position
      QLabel *api;
      //! icon
      QLabel *ii;
      //! mode name
      QLabel *mi;
      //! time
      QLabel *ti;
      //! buffer suffix
      QString bufferNameSuffix;
      //! buffer id
      const int id;

     private:
      Q_OBJECT

      friend struct SecondaryTaskManager;

    };  // end of Buffer

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_BUFFER_HXX */
