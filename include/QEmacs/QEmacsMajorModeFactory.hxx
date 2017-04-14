/*! 
 * \file  QEmacsMajorModeFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef _LIB_QEMACS_QEMACSMAJORMODEFACTORY_H_
#define _LIB_QEMACS_QEMACSMAJORMODEFACTORY_H_ 

#include<memory>

#include<QtCore/QVector>
#include<QtCore/QRegExp>
#include<QtCore/QString>

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsMajorMode.hxx"

namespace qemacs
{

  class QEmacsWidget;

  class QEmacsBuffer;

  class QEmacsTextEditBase;

  struct QEMACS_VISIBILITY_EXPORT QEmacsMajorModeProxy
  {
    virtual QString
    getName(void) const = 0;
    virtual QEmacsMajorMode*
    getQEmacsMajorMode(QEmacsWidget&,
		       QEmacsBuffer&,
		       QEmacsTextEditBase&) const = 0;
    /*!
     * destructor
     */
    virtual ~QEmacsMajorModeProxy();
  }; // end of struct QEmacsMajorModeProxy

  /*!
   * a standard implementation of a proxy
   * \param T : major mode
   */
  template<typename T>
  struct StandardQEmacsMajorModeProxy
    : public QEmacsMajorModeProxy
  {
    /*!
     * \param[in] n : name of the major mode
     * \param[in] e : supported file extensions
     */
    StandardQEmacsMajorModeProxy(const QString&,
				 const QVector<QRegExp>& = QVector<QRegExp>(),
				 const bool = true);
    /*!
     * \param[in] n : return the major mode name
     */
    virtual QString
    getName(void) const override;
    /*!
     * \return a new instance of the major mode
     */
    virtual QEmacsMajorMode *
    getQEmacsMajorMode(QEmacsWidget&,
		       QEmacsBuffer&,
		       QEmacsTextEditBase&) const override;
    /*!
     * destructor
     */
    ~StandardQEmacsMajorModeProxy();
  private:
    //! major mode name
    const QString name;
    //! file extensions supported by the major mode
    const QVector<QRegExp> rexp;
  }; // end of struct StandardQEmacsMajorModeProxy
  
  /*!
   * Major mode factory
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsMajorModeFactory
  {
    typedef std::shared_ptr<QEmacsMajorMode>      QEmacsMajorModePtr;    
    typedef std::shared_ptr<QEmacsMajorModeProxy> QEmacsMajorModeProxyPtr;    

    static QEmacsMajorModeFactory&
    getQEmacsMajorModeFactory();

    void
    loadLibrary(const QString&);
    
    /*!
     * \return a major assiocated with the given file extension
     * \param[in] n : major mode name
     */
    QEmacsMajorMode *
    getQEmacsMajorModeByName(const QString&,
			     QEmacsWidget&,
			     QEmacsBuffer&,
			     QEmacsTextEditBase&) const;

    /*!
     * \return a major assiocated with the given file
     * \param[in] f : file name
     */
    QEmacsMajorMode *
    getQEmacsMajorModeForFile(const QString&,
			      QEmacsWidget&,
			      QEmacsBuffer&,
			      QEmacsTextEditBase&) const;
    
    void
    addQEmacsMajorMode(const QEmacsMajorModeProxyPtr,
		       const QVector<QRegExp>&,
		       const bool = true);
    
    QStringList
    getAvailableQEmacsMajorModesNames(void) const;

    bool
    hasQEmacsMajorMode(const QString&) const;

  private:

    QEmacsMajorModeFactory();

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

#endif /* _LIB_QEMACS_QEMACSMAJORMODEFACTORY_H */
