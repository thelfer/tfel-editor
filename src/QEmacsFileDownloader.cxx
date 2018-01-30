#include "QEmacs/QEmacsFileDownloader.hxx"

namespace qemacs
{

  QEmacsFileDownloader::QEmacsFileDownloader(QUrl url, QObject *p)
    : QObject(p)
  {
    connect(&m_WebCtrl,&QNetworkAccessManager::finished,
	    this,&QEmacsFileDownloader::fileDownloaded);
    QNetworkRequest request(url);
    m_WebCtrl.get(request);
  }

  void QEmacsFileDownloader::fileDownloaded(QNetworkReply* pReply)
  {
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    emit downloaded(this);
  }

  QByteArray QEmacsFileDownloader::downloadedData() const
  {
    return m_DownloadedData;
  }

  QEmacsFileDownloader::~QEmacsFileDownloader() = default;

} // end of namespace qemacs
