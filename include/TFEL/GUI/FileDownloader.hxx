/*!
 * \file  CMajorModeBase.hxx
 * \brief
 * \author Helfer Thomas
 * \date   05/09/2012
 */

#ifndef LIB_TFEL_GUI_FILEDOWNLOADER_HXX
#define LIB_TFEL_GUI_FILEDOWNLOADER_HXX

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <TFEL/GUI/Config.hxx>

namespace tfel {

  namespace gui {

    struct TFEL_GUI_VISIBILITY_EXPORT FileDownloader : public QObject {
      explicit FileDownloader(QUrl, QObject* = nullptr);

      virtual ~FileDownloader();

      QByteArray downloadedData() const;

     signals:

      void downloaded(FileDownloader*);

     private slots:

      void fileDownloaded(QNetworkReply* pReply);

     private:
      QNetworkAccessManager m_WebCtrl;

      QByteArray m_DownloadedData;

      Q_OBJECT
    };

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_FILEDOWNLOADER_*/
