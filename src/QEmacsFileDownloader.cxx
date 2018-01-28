#include "QEmacs/QEmacsFileDownloader.hxx"

namespace qemacs
{

  QEmacsFileDownloader::QEmacsFileDownloader(QUrl url, QObject *p)
    : QObject(p)
  {
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
	    SLOT(fileDownloaded(QNetworkReply*)));
    QNetworkRequest request(url);
    m_WebCtrl.get(request);
  }

  QEmacsFileDownloader::~QEmacsFileDownloader() = default;

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

} // end of namespace qemacs
