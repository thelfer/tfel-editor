/*! 
 * \file  QEmacsMajorModeFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_QEMACS_QEMACSMAJORMODEFACTORY_HXX
#define LIB_QEMACS_QEMACSMAJORMODEFACTORY_HXX 

#include<memory>

#include<QtCore/QVector>
#include<QtCore/QRegExp>
#include<QtCore/QString>

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsMajorMode.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct QEmacsBuffer;
  // forward declaration
  struct QEmacsTextEditBase;

  //! \brief abstract class for all major mode proxies
  struct QEMACS_VISIBILITY_EXPORT QEmacsMajorModeProxy
  {
    //! \return the name of the major mode
    virtual QString getName() const = 0;
    /*!
     * \return the major mode
     * \param[in] w: qemacs widget
     * \param[in] b: qemacs buffer
     * \param[in] t: qemacs text editor widget
     */
    virtual QEmacsMajorMode* getQEmacsMajorMode(
        QEmacsWidget&, QEmacsBuffer&, QEmacsTextEditBase&) const = 0;
    //! destructor
    virtual ~QEmacsMajorModeProxy();
  }; // end of struct QEmacsMajorModeProxy

  /*!
   * \brief a standard implementation of a proxy
   * \tparam T: major mode
   */
  template <typename T>
  struct StandardQEmacsMajorModeProxy : public QEmacsMajorModeProxy {
    /*!
     * \param[in] n : name of the major mode
     * \param[in] e : supported file extensions
     */
    StandardQEmacsMajorModeProxy(
        const QString&,
        const QVector<QRegExp>& = QVector<QRegExp>(),
        const bool = true);
    //! \return return the major mode name
    QString getName() const override;
    /*!
     * \return a new instance of the major mode
     * \param[in] w: qemacs widget
     * \param[in] b: qemacs buffer
     * \param[in] t: qemacs text editor widget
     */
    QEmacsMajorMode * getQEmacsMajorMode(QEmacsWidget&,
					 QEmacsBuffer&,
					 QEmacsTextEditBase&) const override;
    //! destructor
    ~StandardQEmacsMajorModeProxy() override;
  private:
    //! major mode name
    const QString name;
    //! file extensions supported by the major mode
    const QVector<QRegExp> rexp;
  }; // end of struct StandardQEmacsMajorModeProxy

  //! \brief major mode factory
  struct QEMACS_VISIBILITY_EXPORT QEmacsMajorModeFactory
  {
    typedef std::shared_ptr<QEmacsMajorMode>      QEmacsMajorModePtr;    
    typedef std::shared_ptr<QEmacsMajorModeProxy> QEmacsMajorModeProxyPtr;    

    static QEmacsMajorModeFactory&
    getQEmacsMajorModeFactory();

    void loadLibrary(const QString&);

    /*!
     * \return a major assiocated with the given file extension
     * \param[in] n : major mode name
     * \param[in] w: qemacs widget
     * \param[in] b: qemacs buffer
     * \param[in] t: qemacs text editor widget
     */
    QEmacsMajorMode* getQEmacsMajorModeByName(
        const QString&,
        QEmacsWidget&,
        QEmacsBuffer&,
        QEmacsTextEditBase&) const;

    /*!
     * \return a major associated with the given file
     * \param[in] f: file name
     * \param[in] w: qemacs widget
     * \param[in] b: qemacs buffer
     * \param[in] t: qemacs text editor widget
     */
    QEmacsMajorMode* getQEmacsMajorModeForFile(
        const QString&,
        QEmacsWidget&,
        QEmacsBuffer&,
        QEmacsTextEditBase&) const;

    void addQEmacsMajorMode(const QEmacsMajorModeProxyPtr,
                            const QVector<QRegExp>&,
                            const bool = true);

    QStringList getAvailableQEmacsMajorModesNames() const;

    bool hasQEmacsMajorMode(const QString&) const;

   private:

    QEmacsMajorModeFactory();
    //! destructor
    ~QEmacsMajorModeFactory();
    
    struct Proxy
    {
      QEmacsMajorModeProxyPtr proxy;
      QVector<QRegExp> rexp;
    };

    QVector<Proxy> proxies;

  }; // end of struct QEmacsMajorModeFactory

} // end of namespace qemacs

#include"QEmacs/QEmacsMajorModeFactory.ixx"

#endif /* LIB_QEMACS_QEMACSMAJORMODEFACTORY_HXX */
