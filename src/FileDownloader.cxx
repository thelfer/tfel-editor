

#include "TFEL/GUI/FileDownloader.hxx"

namespace tfel {

  namespace gui {

    FileDownloader::FileDownloader(QUrl url, QObject* p) : QObject(p) {
      connect(&m_WebCtrl, &QNetworkAccessManager::finished, this,
              &FileDownloader::fileDownloaded);
      QNetworkRequest request(url);
      m_WebCtrl.get(request);
    }

    void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
      m_DownloadedData = pReply->readAll();
      // emit a signal
      pReply->deleteLater();
      emit downloaded(this);
    }

    QByteArray FileDownloader::downloadedData() const {
      return m_DownloadedData;
    }

    FileDownloader::~FileDownloader() = default;

  }  // end of namespace gui
}  // end of namespace tfel
