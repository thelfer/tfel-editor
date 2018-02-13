/*! 
 * \file  CMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   05/09/2012
 */

#ifndef LIB_QEMACS_QEMACSFILEDOWNLOADER_HXX
#define LIB_QEMACS_QEMACSFILEDOWNLOADER_HXX

#include<QtCore/QObject>
#include<QtCore/QByteArray>
#include<QtNetwork/QNetworkAccessManager>
#include<QtNetwork/QNetworkRequest>
#include<QtNetwork/QNetworkReply>
#include<QEmacs/Config.hxx>

namespace qemacs {

  struct QEMACS_VISIBILITY_EXPORT QEmacsFileDownloader
    : public QObject
  {
    explicit QEmacsFileDownloader(QUrl, QObject* = nullptr);

    virtual ~QEmacsFileDownloader();

    QByteArray downloadedData() const;

  signals:

    void downloaded(QEmacsFileDownloader *);

  private slots:

    void fileDownloaded(QNetworkReply* pReply);

  private:

    QNetworkAccessManager m_WebCtrl;

    QByteArray m_DownloadedData;

    Q_OBJECT
  };

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSFILEDOWNLOADER_*/
