/*!
 * \file  MajorModeFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODEFACTORY_HXX
#define LIB_TFEL_GUI_MAJORMODEFACTORY_HXX

#include <memory>
#include <QtCore/QVector>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/MajorMode.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  // forward declaration
  struct TextEditBase;

  //! \brief abstract class for all major mode proxies
  struct TFEL_GUI_VISIBILITY_EXPORT MajorModeProxy {
    //! \return the name of the major mode
    virtual QString getName() const = 0;
    //! \return the icon associated with the major mode
    virtual QIcon getIcon() const = 0;
    /*!
     * \return the major mode
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text editor widget
     */
    virtual MajorMode* getMajorMode(EditorWidget&,
                                    Buffer&,
                                    TextEditBase&) const = 0;
    //! destructor
    virtual ~MajorModeProxy();
  };  // end of struct MajorModeProxy

  /*!
   * \brief a standard implementation of a proxy
   * \tparam T: major mode
   */
  template <typename T>
  struct StandardMajorModeProxy : public MajorModeProxy {
    /*!
     * \param[in] n: name of the major mode
     * \param[in] e: supported file extensions
     * \param[in] i: major mode icon
     */
    StandardMajorModeProxy(const QString&,
                           const QVector<QRegExp>& = QVector<QRegExp>(),
                           const QString& = QString(),
                           const bool = true);
    //! \return the major mode name
    QString getName() const override;
    //! \return the major mode icon
    QIcon getIcon() const override;
    /*!
     * \return a new instance of the major mode
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text editor widget
     */
    MajorMode* getMajorMode(EditorWidget&,
                            Buffer&,
                            TextEditBase&) const override;
    //! destructor
    ~StandardMajorModeProxy() override;

   private:
    //! major mode name
    const QString name;
    //! file extensions supported by the major mode
    const QVector<QRegExp> rexp;
    //! major mode name
    const QString icon;
  };  // end of struct StandardMajorModeProxy

  //! \brief major mode factory
  struct TFEL_GUI_VISIBILITY_EXPORT MajorModeFactory {
    typedef std::shared_ptr<MajorMode> MajorModePtr;
    typedef std::shared_ptr<MajorModeProxy> MajorModeProxyPtr;

    static MajorModeFactory& getMajorModeFactory();

    void loadLibrary(const QString&);

    /*!
     * \return a major assiocated with the given file extension
     * \param[in] n : major mode name
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text editor widget
     */
    MajorMode* getMajorModeByName(const QString&,
                                  EditorWidget&,
                                  Buffer&,
                                  TextEditBase&) const;
    /*!
     * \return a major associated with the given file
     * \param[in] f: file name
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text editor widget
     */
    MajorMode* getMajorModeForFile(const QString&,
                                   EditorWidget&,
                                   Buffer&,
                                   TextEditBase&) const;

    void addMajorMode(const MajorModeProxyPtr,
                      const QVector<QRegExp>&,
                      const bool = true);

    QStringList getAvailableMajorModesNames() const;

    /*!
     * \return a major associated with the given file
     * \param[in] f: file name
     */
    QString getMajorModeNameForFile(const QString&);
    /*!
     * \return the icon associated with a major mode
     * \param[in] n: major mode name
     */
    QIcon getMajorModeIcon(const QString&) const;

    bool hasMajorMode(const QString&) const;

   private:
    MajorModeFactory();
    //! destructor
    ~MajorModeFactory();

    struct Proxy {
      MajorModeProxyPtr proxy;
      QVector<QRegExp> rexp;
    };

    QVector<Proxy> proxies;

  };  // end of struct MajorModeFactory

}  // end of namespace tfel::gui

#include "TFEL/GUI/MajorModeFactory.ixx"

#endif /* LIB_TFEL_GUI_MAJORMODEFACTORY_HXX */
