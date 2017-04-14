#ifndef _LIB_QEMACS_QEMACSFILEDOWNLOADER_H
#define _LIB_QEMACS_QEMACSFILEDOWNLOADER_H

#include<QtCore/QObject>
#include<QtCore/QByteArray>
#include<QtNetwork/QNetworkAccessManager>
#include<QtNetwork/QNetworkRequest>
#include<QtNetwork/QNetworkReply>

#include<QEmacs/Config.hxx>

namespace qemacs
{

  class QEMACS_VISIBILITY_EXPORT QEmacsFileDownloader
    : public QObject
  {

    Q_OBJECT

    public:

    explicit QEmacsFileDownloader(QUrl,
				  QObject * = nullptr);

    virtual ~QEmacsFileDownloader();

    QByteArray downloadedData() const;

  signals:

    void downloaded(QEmacsFileDownloader *);

  private slots:

    void fileDownloaded(QNetworkReply* pReply);

  private:

    QNetworkAccessManager m_WebCtrl;

    QByteArray m_DownloadedData;

  };

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSFILEDOWNLOADER_*/
