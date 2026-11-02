/*!
 * \file  MajorModeFactory.hxx
 * \brief factory and proxies which build the major modes.
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODEFACTORY_HXX
#define LIB_TFEL_GUI_MAJORMODEFACTORY_HXX

#include <memory>
#include <QtCore/QVector>
#include <QtCore/QRegularExpression>
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
     *
     * \note the caller takes the ownership of the mode, which becomes the mode
     * of a text edit and is destroyed when the text edit replaces it
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
     * \param[in] b: if true, register a copy of this proxy in the major mode
     * factory. The constructor of a proxy which is itself a registration copy,
     * as the one the factory holds, must pass false not to register itself
     * again
     *
     * \note a mode is usually registered by a static instance of this template,
     * defined at the bottom of the source file which defines the mode, so that
     * loading the plugin which holds it is enough to make the mode known to
     * the editor. Each regular expression of \c e is matched against the name
     * of the files the editor opens, and the factory registers the commands
     * `<name>-mode` and `<name in lower case>-mode` along with the mode
     */
    StandardMajorModeProxy(
        const QString&,
        const QVector<QRegularExpression>& = QVector<QRegularExpression>(),
        const QString& = QString(),
        const bool = true);
    //! \return the major mode name
    QString getName() const override;
    /*!
     * \return the major mode icon, built from the resource path the proxy was
     * registered with. It is a default constructed icon when that path is empty
     */
    QIcon getIcon() const override;
    /*!
     * \return a new instance of the major mode
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text editor widget
     *
     * \note the caller takes the ownership of the mode
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
    const QVector<QRegularExpression> rexp;
    //! path of the icon of the major mode in the editor resources
    const QString icon;
  };  // end of struct StandardMajorModeProxy

  //! \brief major mode factory
  struct TFEL_GUI_VISIBILITY_EXPORT MajorModeFactory {
    //! a simple alias
    typedef std::shared_ptr<MajorMode> MajorModePtr;
    //! a simple alias
    typedef std::shared_ptr<MajorModeProxy> MajorModeProxyPtr;

    /*!
     * \return the unique instance of the factory
     *
     * \note the instance is created on the first call and destroyed when the
     * program ends
     */
    static MajorModeFactory& getMajorModeFactory();

    /*!
     * \brief load the given library, whose static initialisers register the
     * major modes it defines
     * \param[in] lib : name of the library
     *
     * \note the search of the library is left to the TFEL external library
     * manager
     */
    void loadLibrary(const QString&);

    /*!
     * \return a major assiocated with the given file extension
     * \param[in] n : major mode name
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text editor widget
     *
     * \note a null pointer, and a message in the minibuffer of \c w, when no
     * proxy registers a mode of that name. The caller takes the ownership of
     * the returned mode
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
     *
     * The first mode whose regular expressions match \c f wins, the most
     * recently registered ones being looked at first.
     *
     * \note a null pointer, and a message in the minibuffer of \c w, when no
     * mode matches. The caller takes the ownership of the returned mode
     */
    MajorMode* getMajorModeForFile(const QString&,
                                   EditorWidget&,
                                   Buffer&,
                                   TextEditBase&) const;

    /*!
     * \brief register a major mode proxy
     * \param[in] proxy: proxy which builds the mode
     * \param[in] e: regular expressions matching the files the mode handles
     * \param[in] b: if true, also register the commands `<name>-mode` and
     * `<name in lower case>-mode`, which switch the current buffer to that
     * mode. The constructor of a proxy passes false when it registers the
     * copy it made for the factory
     *
     * \note the factory keeps the proxy alive for as long as it lives. Modes
     * registered later take precedence over the previously registered ones
     * when they match the same file
     */
    void addMajorMode(const MajorModeProxyPtr,
                      const QVector<QRegularExpression>&,
                      const bool = true);

    /*!
     * \return the names of the registered modes, the most recently registered
     * one first
     */
    QStringList getAvailableMajorModesNames() const;

    /*!
     * \return a major associated with the given file
     * \param[in] f: file name
     *
     * \note an empty string when no mode matches the file name
     */
    QString getMajorModeNameForFile(const QString&);
    /*!
     * \return the icon associated with a major mode
     * \param[in] n: major mode name
     *
     * \note a default constructed icon when no mode of that name is registered
     */
    QIcon getMajorModeIcon(const QString&) const;

    /*!
     * \return true if a mode of the given name is registered
     * \param[in] n: major mode name
     */
    bool hasMajorMode(const QString&) const;

   private:
    /*!
     * \brief builds an empty factory
     *
     * \note only `getMajorModeFactory` may build the single factory of the
     * application: the modes are registered by the static initialisers of the
     * `StandardMajorModeProxy` objects the modes and the packages declare.
     */
    MajorModeFactory();
    //! destructor
    ~MajorModeFactory();

    struct Proxy {
      //! proxy which builds the mode
      MajorModeProxyPtr proxy;
      //! regular expressions of the file names the mode is chosen for
      QVector<QRegularExpression> rexp;
    };

    //! registered modes, the most recently registered one first
    QVector<Proxy> proxies;

  };  // end of struct MajorModeFactory

}  // end of namespace tfel::gui

#include "TFEL/GUI/MajorModeFactory.ixx"

#endif /* LIB_TFEL_GUI_MAJORMODEFACTORY_HXX */
