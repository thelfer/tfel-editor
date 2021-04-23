/*!
 * \file  LicosMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_LICOSMAJORMODE_HXX
#define LIB_TFEL_GUI_LICOSMAJORMODE_HXX

#include <QtCore/QVector>

#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel {

  namespace gui {

    // forward declaration
    struct TextEditBase;
    // forward declaration
    struct LicosTokenizer;
    // forward declaration
    struct LicosBlock;

    //! major mode for Licos input files
    struct LicosMajorMode : public MajorModeBase {
      static const QStringList &getLicosExtensionsSuffix();

      LicosMajorMode(EditorWidget &, Buffer &, TextEditBase &);

      QString getName() const override;

      QString getDescription() const override;

      void setSyntaxHighlighter(QTextDocument *const) override;

      /*!
       * complete the context menu actions
       * \param[in] m : complete the context menu
       * \param[in] c : text cursor at the position where the menu will
       * appear
       */
      void completeContextMenu(QMenu *const, const QTextCursor &) override;

      QCompleter *getCompleter() override;

      void completeCurrentWord(TextEditBase &, const QString &) override;

      QMenu *getSpecificMenu() override;

      bool keyPressEvent(QKeyEvent *const) override;

      bool mousePressEvent(QMouseEvent *const) override;

      /*!
       * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
       * TextEditBase
       * where k1 is either Qt::Key_X or Qt::Key_C.
       * \param[in] k1 : first  key
       * \param[in] m  : second key modifier
       * \param[in] k2 : second key
       * \return true if the short cut is handled by this mode
       */
      bool handleShortCut(const int,
                          const Qt::KeyboardModifiers,
                          const int) override;

      /*!
       * indent the current line
       */
      void indentLine(const QTextCursor &) override;

      /*!
       * indent selected region
       */
      void indentRegion(const QTextCursor &) override;

      QIcon getIcon() const override;

      void comment() override;

      void format() override;

      ~LicosMajorMode() override;

     protected slots:

      virtual void dryrunLicos();

      virtual void showMaterialWizard();

      virtual void showThermalBehaviourWizard();

      virtual void showMechanicalBehaviourWizard();

      virtual void runLicos();

      virtual void startLicos();

      virtual void studyFinished(bool, QString);

      virtual void actionTriggered(QAction *);

      virtual void openDocumentation(QAction *);

      virtual void addMaterialProperties();

     protected:
      struct LicosInsertBlock;

      static QStringList buildLicosExtensionsSuffix();

      /*!
       * check if the given file exists and is readable
       * \param[in] r : the file absolute path
       * \param[in] f : the file to be checked
       * \return true if the file is exists and is readable
       */
      virtual bool checkFile(QString &, const QString &);

      /*!
       * search a file using the LICOS_USER_FILES environment variable
       * \return an empty string if nothing was found
       */
      virtual QString getLicosFile(const QString &);

      /*!
       * search a file using the given path
       * \return an empty string if nothing was found
       */
      QString getFileInPath(const QString &, const QString &);

      int getLineIndentation(QVector<QSharedPointer<LicosBlock>> &,
                             LicosTokenizer &,
                             const QTextCursor &) const;

      int getIndentationIncrement(QVector<QSharedPointer<LicosBlock>> &,
                                  LicosTokenizer &,
                                  int &,
                                  const QString &,
                                  const unsigned short) const;

      /*!
       * \return the file name under the given cursor. An empty string
       * if no suitable file name is found.
       * \param[in] tc : text cursor
       * \param[in] e  : extension tested
       */
      QString getFileNameUnderCursor(const QTextCursor &, const QString &);

      /*!
       * \return the file name under the given cursor. An empty string
       * if no suitable file name is found.
       * \param[in] tc : text cursor
       * \param[in] e  : valid extensions
       */
      QString getFileNameUnderCursor(const QTextCursor &, const QStringList &);

      /*!
       * look if the QTextCursor points to a licos extension file.
       * If so, import the file in a new buffer
       */
      bool import(const QTextCursor &);

      /*!
       * look if the qtextcursor points to a library
       * \return the library name
       */
      QString libraryNameUnderCursor(const QTextCursor &);

      virtual void createActions();

      virtual void createSearchActions();

      virtual void createAnalyseUsingMFMAction(const QString &);

      virtual void search(const QString &, const QString &, const QString &);

      virtual QString getLicosPath() const;

      virtual QString getMFrontMaterialsPath() const;

      virtual QString getLicosStudiesPath() const;

      virtual void analyseUsingMFM(const QString &);

      virtual void openMFrontSource(const QString &);

      QCompleter *c;

      QRegExp rlib;

      //     //! run licos
      //     QAction *ra;
      //     //! run licos in dry-run mode
      //     QAction *dra;
      //
      //     //! add material property
      //     QAction *ampa;
      //
      //     //! material wizard action
      //     QAction *mwa;
      //     //! thermal behaviour wizard action
      //     QAction *tbwa;
      //     //! mechanical behaviour wizard action
      //     QAction *mbwa;
      //
      //     //! open data file in tplot
      //     QAction *ta1;
      //     //! open licos curve in tplot
      //     QAction *ta2;
      //
      //     //! analyse a library using mfm
      //     QAction *mfma;
      //
      //     //! open mfront source
      //     QAction *msrca;
      //
      //     //! search in licos input files in Licos
      //     QAction *lsil;
      //     //! search in licos input files in MFrontMaterials
      //     QAction *lsim;
      //     //! search in licos input files in LicosStudies
      //     QAction *lsis;
      //     //! search in licos input files in
      //     //  Licos+MFrontMaterials+LicosStudies
      //     QAction *lsia;
      //
      //     //! search in mfront input files in Licos
      //     QAction *msil;
      //     //! search in mfront input files in MFrontMaterials
      //     QAction *msim;
      //     //! search in mfront input files in LicosStudies
      //     QAction *msis;
      //     //! search in mfront input files in
      //     //  Licos+MFrontMaterials+LicosStudies
      //     QAction *msia;
      //
      //     QAction *ddoca;
      //     QAction *udoca;
      //     QAction *tdoca;
      //     QAction *l10doca;
      //     QAction *l11doca;
      //     QAction *idoca;
      //     QAction *mdoca;
      //     QVector<QAction *> tudoca;

     private:
      Q_OBJECT

    };  // end of struct LicosMajorMode

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_LICOSMAJORMODE_H */
